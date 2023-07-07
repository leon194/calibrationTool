package com.bellus3d.android.arc.b3d4client;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.os.Environment;

import com.bellus3d.android.arc.b3d4client.LogService.LogService;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.io.RandomAccessFile;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;
import ir.mahdi.mzip.zip.ZipArchive;

import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.*;

public class DiskService {

    public static File sdcard = Environment.getExternalStorageDirectory();
    public static String sdcardLoadFile(String path) {
        BufferedReader br;
        String data = "";
        String line;
        try {
            File file = new File(sdcard, path);
            if (!file.exists()) return null;

            br = new BufferedReader(new FileReader(file));
            while ((line = br.readLine()) != null) {
                data += line;
            }
            br.close();
        } catch (Exception e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
            return null;
        }
        LogService.d(TAG, data);
        return data;
    }

    public static void sdcardSaveFile(String path, String data) {

        File file = new File(sdcard, path);
        String parent = file.getParent();
        File folder = new File(parent);
        if(!folder.exists()) folder.mkdirs();

        try {
            file.createNewFile();
            FileOutputStream out = new FileOutputStream(file);
            OutputStreamWriter writer = new OutputStreamWriter(out);
            writer.append(data);
            writer.close();
            out.flush();
            out.close();
            LogService.d(TAG, data);
        } catch (Exception e) {
            LogService.e(TAG, "Failed : " + e.toString());
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }
    }

    public static byte[] glTF(String msg, byte[] data) {
        LogService.d(TAG, msg);
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
        try {
            int msgLength = msg.length();
            int msgPad = msgLength % 4;
            if( msgPad > 0){
                msgLength += (4 - msgPad);
            }

            int payloadLength = data.length;

            LogService.v(TAG, "msg length: " + msgLength);
            LogService.v(TAG, "data length: " + payloadLength);
            int payloadPad = payloadLength % 4;
            if( payloadPad > 0 ){
                payloadLength += (4 - payloadPad);
            }
            LogService.v(TAG, "msgPad: " + msgPad);
            LogService.v(TAG, "payloadPad: " + payloadPad);

            // glTF header
            outputStream.write("glTF".getBytes());
            outputStream.write(intToByteBufferLE(2));
            outputStream.write(intToByteBufferLE(224+msgLength+payloadLength));

            // JSON chunk
            outputStream.write(intToByteBufferLE(msgLength));
            outputStream.write("JSON".getBytes());
            outputStream.write(msg.getBytes());
            if (msgPad > 0) {
                outputStream.write("   ".substring(0,4-msgPad).getBytes());
            }

            // Payload chunk
            outputStream.write(intToByteBufferLE(payloadLength));
            outputStream.write("BIN\0".getBytes());
            outputStream.write(data);
            if (payloadPad > 0) {
                outputStream.write("\0\0\0".substring(0, 4 - payloadPad).getBytes());
            }

            LogService.v(TAG, "outputStream.size() : " + outputStream.size());
            return outputStream.toByteArray();

        } catch (Exception e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
            return null;
        }
    }

    public static void zipToPath(String path, String dest, String password) {
        File file = new File(dest);
        String parent = file.getParent();
        File folder = new File(parent);
        if(!folder.exists()) folder.mkdirs();

        ZipArchive archive = new ZipArchive();
        archive.zip(path, dest, password);
    }

    public static byte[] zipFiles(String path, File[] srcfiles) {
        String[] paths = {path};
        return zipFiles(paths, srcfiles);
    }

    public static byte[] zipFiles(String[] paths, File[] srcfiles){
        int pathCount = paths.length;
        if (pathCount < 1) {
            LogService.e(TAG, "missing paths");
            return new byte[0];
        }

        if (pathCount > 1 && pathCount != srcfiles.length) {
            LogService.e(TAG, "path count not equal to srcFiles count");
            return new byte[0];
        }

        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        try {
            ZipOutputStream out = new ZipOutputStream(baos);
            if (srcfiles == null) {
                zipAddFolder(paths[0], out);
            } else {
                for(int i = 0; i < srcfiles.length; i++){
                    if (srcfiles[i] == null) continue;
                    String path = paths[(pathCount == 1) ? 0 : i];
                    if (path == null) continue;
                    zipAddFile(path, srcfiles[i].getPath(), out);
                }
            }
            out.close();
        } catch (Exception e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
            return new byte[0];
        }

        return baos.toByteArray();
    }

    private static void zipAddFile(String path, String file, ZipOutputStream out) throws Exception {
        String filePath = path+file;
        File folder = new File(filePath);
        if (folder.isDirectory()) {
            zipAddFolder(path, file, out);
        } else {
            LogService.d(TAG, filePath + ", " + file);
            byte[] buf = new byte[1024];
            FileInputStream in = new FileInputStream(filePath);
            out.putNextEntry(new ZipEntry(file));
            int len;
            while((len=in.read(buf))>0){
                out.write(buf,0,len);
            }
            out.closeEntry();
            in.close();
        }
    }

    private static void zipAddFolder(String path, ZipOutputStream out) throws Exception {
        String entryPath = path.replace(ARC_CLIENT_PATH, "");
        File folder = new File(path);
        for (String fileName : folder.list()) {
            String filePath = entryPath.isEmpty() ? "" : entryPath+"/";
            filePath += fileName;
            LogService.d(TAG, ARC_CLIENT_PATH + ", " + filePath);
            zipAddFile(ARC_CLIENT_PATH, filePath, out);
        }
    }

    private static void zipAddFolder(String path, String file, ZipOutputStream out) throws Exception {
        File folder = new File(path+file);
        for (String fileName : folder.list()) {
            String filePath = file+"/"+fileName;
            LogService.d(TAG, path + ", " + filePath);
            zipAddFile(path, filePath, out);
        }
    }

    public static void deleteFile(String filepath) {
        File file = new File(filepath);
        if (file.exists()) file.delete();
    }

    /* this function will not remove the directory that dir point to*/
    public static void removeDirectory(File dir) {
        if(dir == null || dir.list() == null) return;
        for (File file: dir.listFiles()) {
            if (file.isDirectory())
                removeDirectory(file);
            file.delete();
        }
    }

    public static void removeDirectory(String path) {
        File dir = new File(path);
        removeDirectory(dir);
    }

    public static String swap(String str, int i, int j)
    {
        char ch[] = str.toCharArray();
        char temp = ch[i];
        ch[i] = ch[j];
        ch[j] = temp;
        return new String(ch);
    }

    /**
     * Create a directory if it doesn't exist
     * @param dir The directory needs to be created.
     */
    public static void createDirectory(File dir){
        if(!dir.exists()) {
            if(!dir.mkdirs()){
                LogService.e(TAG, "failed making directory: "+dir.getAbsolutePath());
            }
        }
    }

    public static boolean fileExists(String path){
        File file = new File(path);
        return file == null? false : file.exists();
    }

    /**
     * Create a new file if it doesn't exist
     * @param file The file needs to be created.
     */
    public static boolean createFile(File file){
        createDirectory(file.getParentFile());
        if(!file.exists()) {
            try {
                return file.createNewFile();
            } catch (IOException e) {
                LogService.e(TAG, "Failed to create file: "+file.getAbsolutePath());
                e.printStackTrace();
                LogService.logStackTrace(TAG, e.getStackTrace());
            }
            return false;
        }
        return true;
    }

    public static boolean renameFolder(String oldFolderPath, String newFolderPath) {
        return renameFolder(null, oldFolderPath, newFolderPath);
    }

    /**
     * Rename a folder with new name
     * @param path the path to the folder needs to be renamed
     * @param oldFolderName name of the old folder needs to be renamed
     * @param newFolderName name of the new folder needs to be renamed to
     */
    public static boolean renameFolder(File path, String oldFolderName, String newFolderName) {
        File oldFolder;
        File newFolder;
        if(path == null){
            oldFolder = new File(oldFolderName);
            newFolder = new File(newFolderName);
        } else {
            oldFolder = new File(path, oldFolderName);
            newFolder = new File(path, newFolderName);
        }
        boolean success = oldFolder.renameTo(newFolder);
        if(!success){
            LogService.e(TAG, "Failed to rename folder " + oldFolder.getAbsolutePath() + " to " + newFolder.getAbsolutePath());
        }
        return success;
    }

    /**
     * Copy a file to a destination file
     * @param sourceFile source file needs to be copied
     * @param destFile destination file
     */
    public static void copyFile(File sourceFile, File destFile){
        createFile(destFile);
        FileChannel source = null;
        FileChannel destination = null;
        try {
            source = new FileInputStream(sourceFile).getChannel();
            destination = new FileOutputStream(destFile).getChannel();
            destination.transferFrom(source, 0, source.size());
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        } catch (IOException e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        } finally {
            try {
                if (source != null)
                    source.close();
                if (destination != null)
                    destination.close();
            } catch (IOException e) {
                LogService.logStackTrace(TAG, e.getStackTrace());
            }
        }
    }

    /**
     * Copy a file to a destination file
     * @param sourceFile source file needs to be copied
     * @param destFile destination file
     */
    public static void copyFile(String sourceFile, String destFile){
        FileChannel source = null;
        FileChannel destination = null;
        try {
            source = new FileInputStream(sourceFile).getChannel();
            destination = new FileOutputStream(destFile).getChannel();
            destination.transferFrom(source, 0, source.size());
        } catch (FileNotFoundException e) {
            LogService.logStackTrace(TAG, e.getStackTrace());
        } catch (IOException e) {
            LogService.logStackTrace(TAG, e.getStackTrace());
        } finally {
            try {
                if (source != null)
                    source.close();
                if (destination != null)
                    destination.close();
            } catch (IOException e) {
                LogService.logStackTrace(TAG, e.getStackTrace());
            }
        }
    }

    public void copyAssetFileToFileStorage(final String assetFilepath, final File destFile,
                                           final AssetManager assetManager) {
        createFile(destFile);

        try {
            InputStream in   = assetManager.open(assetFilepath);
            OutputStream out = new FileOutputStream(destFile);
            byte[] buffer = new byte[1024];
            int read = in.read(buffer);
            while (read != -1) {
                out.write(buffer, 0, read);
                read = in.read(buffer);
            }
            out.close();
            in.close();
        } catch (IOException e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }
    }

    /**
     * Copy a folder and all the content inside this folder to another folder recursively
     * @param srcDir Source folder needs to be copied
     * @param dstDir Destination folder
     */
    public static void copyDirectory(String srcDir, String dstDir) {
        try {
            File src = new File(srcDir);
            File dst = new File(dstDir, src.getName());
            if (src.isDirectory()) {
                String files[] = src.list();
                int filesLength = files.length;
                for (int i = 0; i < filesLength; i++) {
                    String src1 = (new File(src, files[i]).getPath());
                    String dst1 = dst.getPath();
                    copyDirectory(src1, dst1);
                }
            } else {
                copyFile(src, dst);
            }
        } catch (Exception e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }
    }

    /**
     * Delete a folder or file and all the sub folders and files recursively
     * @param fileOrDirectory
     */
    public void deleteFileOrDirectory(File fileOrDirectory) {
        if(fileOrDirectory.isDirectory()){
            for(File child: fileOrDirectory.listFiles()){
                deleteFileOrDirectory(child);
            }
        }
        fileOrDirectory.delete();
    }

    /**
     * Writes data to a file based on specified path, file, and data to write
     *
     * @param filePath
     * @param fileName
     * @param data
     */
    public void writeToFile(String filePath, String fileName, String data) {
        File file = new File(filePath, fileName);
        createFile(file);
        try {
            FileOutputStream f = new FileOutputStream(file);
            f.write(data.getBytes());
            f.close();
        } catch (IOException e) {
            LogService.e(TAG, "Failed to write to file: "+file.getAbsolutePath());
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }

    }

    public void writeBitMap(String filePath, String fileName, Bitmap bmp){
        if(filePath == "" || fileName == "" || bmp == null){
            LogService.e(TAG, "File path is: "+filePath + "\nFile name is: "+fileName + "\nBMP is null? "+(bmp == null));
            return;
        }
        FileOutputStream out = null;
        File file = new File(filePath, fileName);
        try {
            out = new FileOutputStream(file);
            bmp.compress(Bitmap.CompressFormat.PNG, 100, out); // bmp is your Bitmap instance
            // PNG is a lossless format, the compression factor (100) is ignored
        } catch (Exception e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        } finally {
            try {
                if (out != null) {
                    out.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
                LogService.logStackTrace(TAG, e.getStackTrace());
            }
        }
    }

    /**
     * Appends notes to an existing file.
     *
     * @param filePath
     * @param fileName
     * @param data
     */
    public static void appendToFile(String filePath, String fileName, String data) {
        File file = new File(filePath, fileName);
        if(!file.exists()){
            createFile(file);
        }
        try {
            FileOutputStream f = new FileOutputStream(file, true);
            f.write(data.getBytes());
            f.close();
        } catch (IOException e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }
    }

    public static void overwriteContentToFile(String filePath, String fileName, String data) {
        File file = new File(filePath, fileName);
        if(!file.exists()){
            createFile(file);
        }
        try {
            FileOutputStream f = new FileOutputStream(file, false);
            f.write(data.getBytes());
            f.close();
        } catch (IOException e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }
    }
    /**
     * Remove all lines(including the line that the key word is in) of a file before certain key word
     * For modifying OpenCV output .yml file to one that Snakeyaml library can read
     * @param file the file needs to be modified
     * @param lineStarterToRemove the key word
     */
    public void formatOpenCVYmlFile(String file, String lineStarterToRemove) {
        boolean foundTheLine = false;
        try {
            File inFile = new File(file);
            if (!inFile.isFile()) {
                LogService.e(TAG, "File doen't exist.");
                return;
            }
            //Construct the new file that will later be renamed to the original filename.
            File tempFile = new File(inFile.getAbsolutePath() + ".tmp");
            BufferedReader br = new BufferedReader(new FileReader(file));
            PrintWriter pw = new PrintWriter(new FileWriter(tempFile));
            String line ;
            //Read from the original file and write to the new
            //unless content matches data to be removed.
            while ((line = br.readLine()) != null) {
                if (!line.trim().startsWith(lineStarterToRemove)) {
                    if(foundTheLine) {
                        pw.println(line);
                        pw.flush();
                    }
                } else {
                    foundTheLine = true;
                }
            }
            pw.close();
            br.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        } catch (IOException e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }
    }

    /**
     * read a file line by line and put String results in arraylist
     *
     * @param infoFile the file need to be read
     * @return an ArrayList of String of the file
     */
    public ArrayList<String> readFile(File infoFile) {
        ArrayList<String> result = new ArrayList<String>();

        if (infoFile.exists()) {
            BufferedReader brTest = null;
            try {
                brTest = new BufferedReader(new FileReader(infoFile));
                String tempString = "";
                while (null != (tempString = brTest.readLine())) {
                    result.add(tempString);
                }
            } catch (FileNotFoundException e) {
                e.printStackTrace();
                LogService.logStackTrace(TAG, e.getStackTrace());
            } catch (IOException e) {
                e.printStackTrace();
                LogService.logStackTrace(TAG, e.getStackTrace());
            }
        }
        return result;
    }

    /**
     * read a Directory and put each file in an ArrayList by the order the name.
     *
     * @param path the directory need to be read
     * @param isFolderNeeded true means folder false means file
     * @param isAscendingOrder true means ascending false means descending
     * @return an ArrayList of Files
     */
    public ArrayList<File> readDirectory(String path, boolean isFolderNeeded, final boolean isAscendingOrder) {
        ArrayList<File> directories = new ArrayList<File>();
        File dir = new File(path);
        File[] files = dir.listFiles();
        if (files != null) {
            if (files.length > 1) {
                Arrays.sort(files, new Comparator<File>() {
                    @Override
                    public int compare(File f1, File f2) {
//                        return Long.valueOf(f2.lastModified()).compareTo(f1.lastModified());
                        if(isAscendingOrder){
                            // sort file by last modified date in ascending order
                            return f1.getName().compareTo(f2.getName());
                        } else {
                            // sort file by last modified date in descending order
                            return -f1.getName().compareTo(f2.getName());
                        }
                    }
                });
            }

            for (int i = 0; i < files.length; i++) {
                if(isFolderNeeded) {
                    if (files[i].isFile()) continue;
                    directories.add(files[i]);
                } else {
                    if (files[i].isFile()) {
                        directories.add(files[i]);
                    }
                }
            }
        }
        return directories;
    }

    public boolean pathExists(String path){
        File dir = new File(path);
        return dir.exists();
    }

    public boolean keywordExistsInFile(String keyword, File file){
        if(!file.exists()) {
            LogService.e(TAG, file.getAbsolutePath()+" doesn't exist.");
            return false;
        }
        try {
            FileInputStream in = new FileInputStream(file);
            int len = 0;
            byte[] data1 = new byte[1024];
            while ( -1 != (len = in.read(data1)) ){
                if(new String(data1, 0, len).contains(keyword)){
                    return true;
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }
        return false;
    }

    public String extractTextureFileName(File materialFile) {
        String[] results;
        if (materialFile.exists()) {
            BufferedReader brTest = null;
            try {
                brTest = new BufferedReader(new FileReader(materialFile));
                String tempString = "";
                while (null != (tempString = brTest.readLine())) {
                    results = tempString.split("");
                    if(results.length == 2 && results[0] == "map_Kd"){
                        return results[1];
                    }
                }
            } catch (FileNotFoundException e) {
                e.printStackTrace();
                LogService.logStackTrace(TAG, e.getStackTrace());
            } catch (IOException e) {
                e.printStackTrace();
                LogService.logStackTrace(TAG, e.getStackTrace());
            }
        }
        return "";
    }

    static byte[] intToByteBufferLE(int i) {
        return new byte[]{
                (byte)i,
                (byte)(i >>> 8),
                (byte)(i >>> 16),
                (byte)(i >>> 24)
        };
    }

    public static boolean copyAssetFolder(Context context, String srcName, String dstName) {
        try {
            boolean result = true;
            String fileList[] = context.getAssets().list(srcName);
            if (fileList == null) return false;

            if (fileList.length == 0) {
                result = copyAssetFile(context, srcName, dstName);
            } else {
                File file = new File(dstName);
                result = file.mkdirs();
                for (String filename : fileList) {
                    result &= copyAssetFolder(context, srcName + File.separator + filename, dstName + File.separator + filename);
                }
            }
            return result;
        } catch (IOException e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
            return false;
        }
    }

    public static boolean copyAssetFile(Context context, String srcName, String dstName) {
        try {
            InputStream in = context.getAssets().open(srcName);
            File outFile = new File(dstName);
            OutputStream out = new FileOutputStream(outFile);
            byte[] buffer = new byte[1024];
            int read;
            while ((read = in.read(buffer)) != -1) {
                out.write(buffer, 0, read);
            }
            in.close();
            out.close();
            return true;
        } catch (IOException e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
            return false;
        }
    }

    public static byte[] concat(byte[]... arrays) {
        int length = 0;
        for (byte[] array : arrays) {
            length += array.length;
        }
        byte[] result = new byte[length];
        int pos = 0;
        for (byte[] array : arrays) {
            System.arraycopy(array, 0, result, pos, array.length);
            pos += array.length;
        }
        return result;
    }

    public static byte[] readFileToByte(String path) {
        File file = new File(path);
        int size = (int) file.length();
        byte[] bytes = new byte[size];
        try {
            BufferedInputStream buf = new BufferedInputStream(new FileInputStream(file));
            buf.read(bytes, 0, bytes.length);
            buf.close();
        } catch (FileNotFoundException e) {
            LogService.e(TAG,e.toString());
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        } catch (IOException e) {
            LogService.e(TAG,e.toString());
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }

        return bytes;
    }

    public static boolean mmapSave(byte[] data, String filename) {
        try {
            LogService.d(TAG," start save file to "+ filename);
            long startMs = System.currentTimeMillis();
            RandomAccessFile randomAccessFile = new RandomAccessFile(filename, "rw");
            MappedByteBuffer mappedByteBuffer = randomAccessFile.getChannel().map(FileChannel.MapMode.READ_WRITE, 0, data.length);
            mappedByteBuffer.put(data);
            LogService.d(TAG,"mappedByteBuffer size : " + mappedByteBuffer.capacity() + " byte length : " +data.length + " cost : " + ( System.currentTimeMillis() -startMs));
            mappedByteBuffer.clear();
            randomAccessFile.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
            return false;
        } catch (IOException e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
            return false;
        }
        return true;
    }

    public static byte[] mmapGet(String filename) {
        byte[] data = null;
        try {
            LogService.d(TAG,"start load file from "+ filename);
            long startMs = System.currentTimeMillis();
            File fileData = new File(filename);
            if (!fileData.exists()) return data;
            RandomAccessFile randomAccessFile = new RandomAccessFile(fileData, "rw");
            MappedByteBuffer mappedByteBuffer = randomAccessFile.getChannel().map(FileChannel.MapMode.READ_ONLY, 0, randomAccessFile.length());
            data = new byte[mappedByteBuffer.remaining()];
            mappedByteBuffer.get(data);
            LogService.d(TAG,"mappedByteBuffer size : " + mappedByteBuffer.capacity() + " bufferToSend length : " +data.length + " cost : " + ( System.currentTimeMillis() -startMs));
            mappedByteBuffer.clear();
            randomAccessFile.close();
        } catch (FileNotFoundException e) {
            LogService.e(TAG,"file doesn't exist: " + filename );
            LogService.e(TAG,e.toString() );
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
            return data;
        } catch (IOException e) {
            LogService.e(TAG,"IOException !!");
            LogService.e(TAG,e.toString() );
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
            return data;
        }
        return data;
    }
}
