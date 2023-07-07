package com.bellus3d.android.arc.b3d4client;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Rect;
import android.graphics.YuvImage;

import com.bellus3d.android.arc.b3d4client.LogService.LogService;
import com.bellus3d.android.arc.b3d4client.camera.B3DCaptureSettings;

import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;

import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.TAG;

public class FormattingService {

    public static byte[] compressYuvImg(byte[] bytedata, int format, int width, int height, int[] strides, String compressformat) {
        double start, end;
        YuvImage image = new YuvImage(bytedata, format,width,height,strides);
        ByteArrayOutputStream os = new ByteArrayOutputStream();
        start = System.currentTimeMillis();
        image.compressToJpeg(new Rect(0,0,width,height),90,os);
        if(compressformat.equals(B3DCaptureSettings.Format.JPEG.getName())) {
            end = System.currentTimeMillis();
            LogService.d(TAG, "cost " + (end - start) + " ms");
            return os.toByteArray();
        } else {
            Bitmap bmp = BitmapFactory.decodeByteArray(os.toByteArray(), 0, os.size());
            bmp.compress(Bitmap.CompressFormat.valueOf(compressformat),90, os);
            end = System.currentTimeMillis();
            LogService.d(TAG, "cost " + (end - start) + " ms");
            return os.toByteArray();
        }
    }

    public static byte[] compressByteImg(byte[] bytedata, int width, int height, String format) {
        double start, end;
        start = System.currentTimeMillis();
        byte [] bits = new byte [ bytedata.length*4 ];
        for( int i = 0; i < bytedata.length; i++)
        {
            bits[i*4] = bits[i*4+1] = bits[i*4+2] = bytedata[i];
            bits[i*4+3] = (byte) 0xff; // the alpha.
        }
        Bitmap image = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        image.copyPixelsFromBuffer(ByteBuffer.wrap(bits));
        bits = null;
        ByteArrayOutputStream os = new ByteArrayOutputStream();
        image.compress(Bitmap.CompressFormat.valueOf(format),90, os);
        end = System.currentTimeMillis();
        LogService.d(TAG,"cost " + (end-start) + " ms");
        return os.toByteArray();
    }

    public static byte[] rotateNV21(final byte[] yuv,
                                    final int width,
                                    final int height,
                                    final int rotation) {
        if (rotation == 0) return yuv;
        if (rotation % 90 != 0 || rotation < 0 || rotation > 270) {
            throw new IllegalArgumentException("0 <= rotation < 360, rotation % 90 == 0");
        }

        final byte[]  output    = new byte[yuv.length];
        final int     frameSize = width * height;
        final boolean swap      = rotation % 180 != 0;
        final boolean xflip     = rotation % 270 != 0;
        final boolean yflip     = rotation >= 180;

        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
                final int yIn = j * width + i;
                final int uIn = frameSize + (j >> 1) * width + (i & ~1);
                final int vIn = uIn       + 1;

                final int wOut     = swap  ? height              : width;
                final int hOut     = swap  ? width               : height;
                final int iSwapped = swap  ? j                   : i;
                final int jSwapped = swap  ? i                   : j;
                final int iOut     = xflip ? wOut - iSwapped - 1 : iSwapped;
                final int jOut     = yflip ? hOut - jSwapped - 1 : jSwapped;

                final int yOut = jOut * wOut + iOut;
                final int uOut = frameSize + (jOut >> 1) * wOut + (iOut & ~1);
                final int vOut = uOut + 1;

                output[yOut] = (byte)(0xff & yuv[yIn]);
                output[uOut] = (byte)(0xff & yuv[uIn]);
                output[vOut] = (byte)(0xff & yuv[vIn]);
            }
        }
        return output;
    }

    public static byte[] scaleYUV420(byte[] data, int imageWidth, int imageHeight, int ratio) {
        double start, end;
        start = System.currentTimeMillis();
        byte[] yuv = new byte[imageWidth/ratio * imageHeight/ratio * 3 / 2];
        // Y
        int i = 0;
        for (int y = 0; y < imageHeight; y+=ratio) {
            for (int x = 0; x < imageWidth; x+=ratio) {
                yuv[i] = data[y * imageWidth + x];
                i++;
            }
        }
        // U and V color components
        for (int y = 0; y < imageHeight / 2; y+=ratio) {
            for (int x = 0; x < imageWidth; x += ratio*2) {
                yuv[i] = data[(imageWidth * imageHeight) + (y * imageWidth) + x];
                i++;
                yuv[i] = data[(imageWidth * imageHeight) + (y * imageWidth) + (x + 1)];
                i++;
            }
        }
        end = System.currentTimeMillis();
        LogService.d(TAG,"cost " + (end-start) + " ms");
        return yuv;
    }

    public static byte[] byteArrayReverse( byte[] original ) {
        byte[] out = new byte[original.length];
        for(int x = 0 ; x < original.length ; x++)
            out[original.length-1-x] = original[x];

        return out;
    }

    public static void savePNG8UC1(byte[] buffer, int width, int height, String path) {
        savePNG8UC1JNI(buffer,width,height,path);
    }

    private static native void savePNG8UC1JNI(byte[] buffer, int width, int height, String path);
}
