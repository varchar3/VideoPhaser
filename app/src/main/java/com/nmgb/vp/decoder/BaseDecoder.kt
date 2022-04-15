package com.nmgb.vp.decoder

import android.media.MediaCodec
import android.media.MediaFormat
import android.util.Log
import com.nmgb.vp.decoder.extractor.IExtractor
import java.io.File
import java.nio.ByteBuffer

/**
- @author:  LZC
- @time:  1/5/2022
- @desc:基礎解碼框架
 */
abstract class BaseDecoder(path:String) :IDecoder{
    private var isRunning = true
    /**
     * 线程等待锁
     */
    private val mLock = Object()
    /**
     * 是否可以进入解码
     */
    private var mReadyForDecode  = false

    var mFilePath:String = path

    private var mEndPos:Long = 0L

    //同步相关-----------------------------------------------------------------------
    private var mStartTimeForSync = -1L

    //解码器相关-----------------------------------------------------------------------
    /**
     * 解码器
     * */
    protected var mCodec: MediaCodec? = null

    /**
     * 音视频数据读取器,Android自带了一个MediaExtractor，这里为了方便拓展使用自制的
     */
    protected var mExtractor: IExtractor? = null

    /**
     * 码输入缓冲区
     * */
    protected var mInputBuffers:Array<ByteBuffer>?=null

    /**
     * 码输出缓冲区
     * */
    protected var mOutputBuffers:Array<ByteBuffer>?=null

    /**
     * 定义解码后装码的容器
     * */
    private var mBufferInfo = MediaCodec.BufferInfo()

    var mState = DecodeState.STOP

    var mStateListener: IDecoderStateListener? = null

    var mDuration:Long = 0L

    /**
     * 流数据是否结束
     */
    private var mIsEOS = false

    protected var mVideoWidth = 0

    protected var mVideoHeight = 0

    private fun init():Boolean {
        if(mFilePath.isEmpty() || !File(mFilePath).exists()){
            Log.e("decoding","文件或路径为空")
            mStateListener?.decodeError(this,"EMPTY FILE OR PATH!")
            return false
        }

        if (!check()) return false

        mExtractor = initExtractor(mFilePath)

        if (mExtractor == null ||
            mExtractor!!.getFormats() == null) return false

        //初始化參數
        if (!initParams()) return false
        //初始化渲染器：视频不需要，音频为AudioTracker
        if (!initRender()) return false
        //初始化解碼器
        if (!initCodec()) return false

        return true
    }

    /**
     * 初始化参数
     * */
    private fun initParams():Boolean{
        try {
            val format = mExtractor!!.getFormats()!!
            mDuration = format.getLong(MediaFormat.KEY_DURATION)/1000
            if (mEndPos == 0L) mEndPos = mDuration
            initSpecParams(format)
        }catch(e:Exception){
            return false
        }
        return true
    }

    /**
     * 初始化render，交給子類
     * */
    abstract fun initRender():Boolean

    /**
     * 渲染,交給子類
     */
    abstract fun render(outputBuffers: ByteBuffer,
                        bufferInfo: MediaCodec.BufferInfo)

    /**
     * 初始化解码器
     * */
    private fun initCodec():Boolean{
        try {
            //1.根据音视频编码格式初始化解码器
            val type = mExtractor!!.getFormats()!!.getString(MediaFormat.KEY_MIME)
            mCodec = MediaCodec.createDecoderByType(type!!)
            //2.配置解码器
            if (!configCodec(mCodec!!, mExtractor!!.getFormats()!!)) {
                waitDecode()
            }
            //3.启动解码器
            mCodec!!.start()
            //4.賦值解码器缓冲区
            mInputBuffers = mCodec?.inputBuffers
            mOutputBuffers = mCodec?.outputBuffers
        }catch(e:Exception){
            return false
        }
        return true
    }

    override fun run() {
        if (mState == DecodeState.STOP) {
            mState = DecodeState.START
        }
        mStateListener?.decodePrepare(this)

        if (!init()) return

        try{
            while (isRunning) {
                if (
                    mState != DecodeState.START &&
                    mState != DecodeState.DECODING &&
                    mState != DecodeState.SEEKING) {
                    waitDecode()
                    // ---------【同步时间矫正】-------------
                    //恢复同步的起始时间，即去除等待流失的时间
                    mStartTimeForSync = System.currentTimeMillis() - getCurrentTimeStamp()
                }

                if (!isRunning || mState == DecodeState.STOP) {
                    isRunning = false
                    break
                }

                if (mStartTimeForSync == -1L) {
                    mStartTimeForSync = System.currentTimeMillis()
                }

                //如果数据没有解码完毕，将数据推入解码器解码
                if (!mIsEOS) {
                    //【解码步骤：2. 将数据压入解码器输入缓冲】
                    mIsEOS = pushBufferToDecoder()
                }

                //【解码步骤：3. 将解码好的数据从缓冲区拉取出来】
                val index = pullBufferFromDecoder()

                if (index >= 0) {
                    // ---------【音视频同步】-------------
                    if (mState == DecodeState.DECODING){
                        sleepRender()
                    }
                    //【解码步骤：4. 渲染】
                    render(mOutputBuffers!![index], mBufferInfo)
                    //【解码步骤：5. 释放输出缓冲】參數2 render為是否將這一針數據顯示
                    mCodec!!.releaseOutputBuffer(index, true)
                    if (mState == DecodeState.START) {
                        mState = DecodeState.PAUSE
                    }
                }

                //【解码步骤：6. 判断解码是否完成】標注下完成的回調
                if (mBufferInfo.flags == MediaCodec.BUFFER_FLAG_END_OF_STREAM) {
                    mState = DecodeState.FINISH
                    mStateListener?.decodeFinish(this)
                    doneDecode()
                    release()
                }
            }
        } catch (e:Exception){
            print(e)
        }
    }

    /**
     * 【解码步骤：2. 将数据压入解码器输入缓冲】
     * */
    private fun pushBufferToDecoder(): Boolean{
        //返回要填充有效数据的输入缓冲区的索引,單位：微秒
        val inputBufferIndex = mCodec!!.dequeueInputBuffer(2000)
        var isEndOfStream = false
        //跟源碼res >= 0對應，證明有數據輸入
        if(inputBufferIndex >= 0) {
            //獲取到當前索引的緩衝數據
            val inputBuffer = mInputBuffers!![inputBufferIndex]
            //樣本大小
            val sampleSize = mExtractor!!.readBuffer(inputBuffer)
            //如果SampleSize返回-1，说明没有更多的数据了。
            if (sampleSize < 0) {
                //如果数据已经取完，压入数据结束标志：BUFFER_FLAG_END_OF_STREAM
                mCodec!!.queueInputBuffer(inputBufferIndex, 0, 0,
                    0,MediaCodec.BUFFER_FLAG_END_OF_STREAM)
                isEndOfStream = true
            }else{
                mCodec!!.queueInputBuffer(inputBufferIndex, 0,
                    sampleSize, mExtractor!!.getCurrentTimeStamp(), 0)
            }
        }
        return isEndOfStream
    }

    private fun pullBufferFromDecoder(): Int{
        // 查询是否有解码完成的数据，index >=0 时，表示数据有效，并且index为缓冲区索引,同上-1是无限等待
        when(val index = mCodec!!.dequeueOutputBuffer(mBufferInfo, 1000)){
            MediaCodec.INFO_TRY_AGAIN_LATER->{}//没有可用数据，等会再来
            MediaCodec.INFO_OUTPUT_FORMAT_CHANGED->{}//输出格式改变了
            MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED->{//输入缓冲改变了
                mOutputBuffers = mCodec!!.outputBuffers
            }else->{
                return index
            }
        }
        return -1
    }

    private fun sleepRender() {
        //系统currentTimeMillis以毫秒为准
        //演示的时间戳 微秒为单位/1000 ->毫秒为单位
        val passTime = System.currentTimeMillis() - mStartTimeForSync
        val curTime = getCurrentTimeStamp()
        if (curTime > passTime) {
            Thread.sleep(curTime - passTime)
        }
    }

    /**
     * 解码线程进入等待
     */
    private fun waitDecode() {
        try {
            if (mState == DecodeState.PAUSE) {
                mStateListener?.decodePause(this)
            }
            synchronized(mLock) {
                mLock.wait()
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    /**
     * 通知解码线程继续运行
     */
    fun notifyDecode() {
        synchronized(mLock) {
            mLock.notifyAll()
        }
        if (mState == DecodeState.DECODING) {
            mStateListener?.decoderRunning(this)
        }
    }

    /**
     * 释放解码器等資源
     * */
    fun release(){
        try {
            mState = DecodeState.STOP
            mIsEOS = false
            mExtractor?.stop()
            mCodec?.stop()
            mCodec?.release()
            Log.i("released","执行了清除缓存")
            mStateListener?.decoderDestroy(this)
        }catch (e:Exception){
            println("release error by msg:$e")
        }
    }

    override fun go() {
        mState = DecodeState.DECODING
        notifyDecode()
    }

    override fun stop() {
        mState = DecodeState.STOP
        isRunning = false
        notifyDecode()
    }

    override fun pause() {
        mState = DecodeState.PAUSE
    }

    override fun seekTo(pos: Long): Long {
        return 0L
    }

    override fun seekAndPlay(pos: Long): Long {
        return 0L
    }

    override fun isDecoding(): Boolean {
        return mState == DecodeState.DECODING
    }

    override fun isSeeking(): Boolean {
        return mState == DecodeState.SEEKING
    }

    override fun isStop(): Boolean {
        return mState == DecodeState.STOP
    }

    override fun getFilePath(): String {
        return mFilePath
    }

    override fun getWidth(): Int {
        return mVideoWidth
    }

    override fun getHeight(): Int {
        return mVideoHeight
    }

    override fun getDuration(): Long {
        return mDuration
    }

    override fun getRotationAngle(): Int {
        return 0
    }

    override fun getTrack(): Int {
        return 0
    }

    override fun getMediaFormat(): MediaFormat? {
        return mExtractor?.getFormats()
    }

    override fun getCurrentTimeStamp(): Long {
        return mBufferInfo.presentationTimeUs / 1000
    }

    /**
     * 结束解码
     */
    abstract fun doneDecode()

    /**
     * 检查子类参数
     */
    abstract fun check(): Boolean

    /**
     * 初始化数据提取器
     */
    abstract fun initExtractor(path: String): IExtractor

    /**
     * 初始化子类自己特有的参数
     */
    abstract fun initSpecParams(format: MediaFormat)

    /**
     * 配置解码器
     */
    abstract fun configCodec(codec: MediaCodec, format: MediaFormat): Boolean

}