package com.nmgb.vp;

import org.junit.Test;

/**
 * - @author:  LZC
 * - @time:  1/5/2022
 * - @desc:
 */
public class Testss {
    @Test
    public static void main(String[] args) {
        new MThreadTest().run();
    }
    static class MThreadTest implements Runnable{
        @Override
        public void run() {
            for (int i = 0; i <5; i++){
                System.out.println("this is how it works"+i);
            }
        }
    }
}
