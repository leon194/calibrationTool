package com.bellus3d.android.arc.b3d4client.camera;

import android.os.Build;
import android.support.annotation.RequiresApi;
import android.util.Size;

import com.bellus3d.android.arc.b3d4client.ConfigService.LocalConfigService;
import com.bellus3d.android.arc.b3d4client.LogService.LogService;
import com.bellus3d.android.arc.b3d4client.MessageService.NetWorkMessage;

import java.util.ArrayList;
import java.util.List;

import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.*;

/**
 * Camera capture settings
 */

public class B3DCaptureSettings {

    private static final String TAG = "B3DCaptureSettings";

    public enum CameraSource{
        COLOR("COLOR"),
        DEPTH("DEPTH"),
        DEVICE("DEVICE"),
        IR_LEFT("IR-LEFT"),
        IR_RIGHT("IR-RIGHT");

        private String name;

        private CameraSource(String name) {
            this.name = name;
        }

        public String getName() {
            return this.name;
        }

    }

    public enum Layout{
        NAME("layoutName"),
        DEVICES("layoutDevices"),
        POSITION("layoutPosition");

        private String name;

        private Layout(String name) {
            this.name = name;
        }

        public String getName() {
            return this.name;
        }

    }

    public enum Format {
        RAW("RAW"),
        JPEG("JPEG"),
        PNG("PNG");
        private String name;

        private Format(String format) {
            this.name = format;
        }

        public String getName() {
            return this.name;
        }
    }

    public ArrayList<String> sourceNames = new ArrayList<String>();

    public ArrayList<String> formatNames = new ArrayList<String>();

    private ArrayList<CameraSource> mSupportedSources = null;
    private ArrayList<Format> mSupportedFormats = null;
    private ArrayList<String>  mSupportedDimensions = null;

    private CameraSource mBufferSource = CameraSource.DEVICE;
    private Format mBufferFormat = Format.RAW;
    private CameraSource mSource = CameraSource.DEVICE;
    private Format mFormat = Format.RAW;

    private int mBufferWidth = 0;
    private int mBufferHeight = 0;
    private int mWidth = 0;
    private int mHeight = 0;
    private double mFPS = 0.0;
    private double mColorRatio = 0.0;

    private boolean mDebug = false;
    private String mLayoutName = null;
    private int mLayoutDevices = 0;
    private String mLayoutPosition = null;

    private double mDefaultCompression = 0.0;
    private double mCompression = 0.0;
    private int mMaxBufferFrames = 0;
    private int mBufferFrames = 0;
    private int mStreamFrames = 0;
    private int mDepthFrames = 0;
    private int mStreamDepthBufferFrames = 0;
    private int mBufferSnapFrames = 0;
    private double mBufferSnapFramesFPS = 0.0;
    private int mBufferFetchSize = -1;

    private String mCaptureBufferBufferID = null;
    private String mFetchBufferBufferID = null;
    private String mMergeBufferBufferD = null;
    private String mReleaseBufferBufferID = null;
    private String mStreamCaptureBufferBufferID = null;
    private String mBufferColorBufferBufferID = null;
    private String mBufferCancelBufferID = null;
    private String mStreamDepthBufferBufferID = null;
    private String mBufferSnapFramesBufferID = null;
    private String mBufferSnapCamPose = null;
    private String mBufferReceiveBufferID = null;

    private int mStreamDepthStartIndex = 0;

    private boolean mTrackingFace = false;
    private double mTrackingFaceX = 0.0;
    private double mTrackingFaceY = 0.0;
    private double mTrackingFaceZ = 0.0;
    private double mTrackingFaceRotationX = 0.0;
    private double mTrackingFaceRotationY = 0.0;
    private double mTrackingFaceRotationZ = 0.0;
    private double mTrackingFaceFaceRectX = 0.0;
    private double mTrackingFaceFaceRectY = 0.0;
    private double mTrackingFaceFaceRectWidth = 0.0;
    private double mTrackingFaceFaceRectHeight = 0.0;
    private double mFaceDistance[] = null;
    private double mFaceRect[] = null;
    private double mxform[] = null;

    private boolean mCapturingStatsColor = false;
    private int mStatsColorExpLine = 0;
    private int mStatsColorGainLuminocity = 0;
    private int mStatsColorGainRed = 0;
    private int mStatsColorGainGreen = 0;
    private int mStatsColorGainBlue = 0;

    private double mInterval = 0.0;
    private double mDuration = 0.0;

    private boolean mIrFloodOn = false;
    private boolean mIrProjectorOn = false;
    private boolean mReturnFirstFrame = false;
    private float mIrExp = 3.5f;

    private boolean mIsStreamPause = false;
    private boolean mIsDiagnostic = false;

    private String mRequest = "UNKNOWN";
    private String mRequestId = "UNKNOWN";
    private String mSubRequest = "UNKNOWN";
    private String mSubRequestId = "UNKNOWN";
    private String mBufferColorRequestType = "UNKNOWN";

    private boolean mdoManualReCalibration = false;

    private List<Integer> mSelectedFrameIndices;

    private String streamMode;
    private String mbufferCaptureMode = BUFFER_CAPTURE_STILL;
    private int mFetchBufferType = 0;

    public B3DCaptureSettings(B3DDepthCamera.DeviceType _DepthCamType) {
        sourceNames.add(B3DCaptureSettings.CameraSource.COLOR.getName());
        sourceNames.add(B3DCaptureSettings.CameraSource.DEPTH.getName());
        sourceNames.add(B3DCaptureSettings.CameraSource.DEVICE.getName());
        sourceNames.add(B3DCaptureSettings.CameraSource.IR_LEFT.getName());
        sourceNames.add(B3DCaptureSettings.CameraSource.IR_RIGHT.getName());

        formatNames.add(Format.RAW.getName());
        formatNames.add(Format.JPEG.getName());
        formatNames.add(Format.PNG.getName());

        switch(_DepthCamType) {
            case DEVICE_B3D4: {
                ArrayList<B3DCaptureSettings.CameraSource> sources = new ArrayList<B3DCaptureSettings.CameraSource>();
                sources.add(B3DCaptureSettings.CameraSource.DEVICE);
                sources.add(B3DCaptureSettings.CameraSource.COLOR);
                sources.add(B3DCaptureSettings.CameraSource.IR_LEFT);
                sources.add(B3DCaptureSettings.CameraSource.IR_RIGHT);
                sources.add(B3DCaptureSettings.CameraSource.DEPTH);
                setSupportedSources(sources);

                ArrayList<B3DCaptureSettings.Format> formats = new ArrayList<B3DCaptureSettings.Format>();
                formats.add(B3DCaptureSettings.Format.JPEG);
                formats.add(B3DCaptureSettings.Format.PNG);
                setSupportedFormats(formats);
                break;
            }
            case DEVICE_B3D4_SINGLE:
            case DEVICE_FILES: {
                ArrayList<B3DCaptureSettings.CameraSource> sources = new ArrayList<B3DCaptureSettings.CameraSource>();
                sources.add(B3DCaptureSettings.CameraSource.COLOR);
                setSupportedSources(sources);

                ArrayList<B3DCaptureSettings.Format> formats = new ArrayList<B3DCaptureSettings.Format>();
                formats.add(B3DCaptureSettings.Format.JPEG);
                setSupportedFormats(formats);

                // Default settings
                set(B3DCaptureSettings.CameraSource.COLOR, B3DCaptureSettings.Format.JPEG, 320, 240, 20.0);
                break;
            }
        }
    }



    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    private void setFormatDimension(NetWorkMessage msg) {

        Size size = LocalConfigService.parseDimension(msg.dimension);

        if (msg.request.equals(NetWorkMessage.RequestType.BUFFER_CAPTURE.getName())) {
            if (msg.format != null && !msg.format.isEmpty()) {
                mBufferFormat = getFormatByName(msg.format);
            } else {
                mBufferFormat = Format.RAW;
            }

            mBufferWidth = size.getWidth();
            mBufferHeight = size.getHeight();
            if( mBufferWidth == ARC_8M_COLOR_WIDTH && mBufferHeight == ARC_8M_COLOR_HEIGHT)
                mbufferCaptureMode = BUFFER_CAPTURE_STILL;
            else if( mBufferWidth == ARC_2M_COLOR_WIDTH && mBufferHeight == ARC_2M_COLOR_HEIGHT)
                mbufferCaptureMode = BUFFER_CAPTURE_4D;
        } else {
            if (msg.format != null && !msg.format.isEmpty()) {
                mFormat = getFormatByName(msg.format);
            } else {
                mFormat = mBufferFormat;
            }

            mWidth = size.getWidth();
            mHeight = size.getHeight();
            if (mWidth == 0 || mHeight == 0) {
                mWidth = mBufferWidth;
                mHeight = mBufferHeight;
            }
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    public void set(NetWorkMessage msg) {
        if (msg == null) return;

        if (msg.request == null || msg.request.isEmpty()) return;
        mRequest = msg.request;
        if (msg.request_id == null || msg.request_id.isEmpty()) return;
        mRequestId = msg.request_id;

        mInterval = (msg.interval != null && !msg.interval.isEmpty()) ? Double.parseDouble(msg.interval) : 0.0;
        mDuration = (msg.duration != null && !msg.duration.isEmpty()) ? Double.parseDouble(msg.duration) : 0.0;

        LogService.v(TAG, "debug: " + msg.debug);
        mDebug = msg.debug;

        if (mRequest.equals(NetWorkMessage.RequestType.LAYOUT_POSITION.getName())) {
            if (msg.layout != null && !msg.layout.isEmpty()) {
                mLayoutName = msg.layout;
            }
            if (msg.layout_devices != null && !msg.layout_devices.isEmpty()) {
                mLayoutDevices = Integer.parseInt(msg.layout_devices);
            }
            if (msg.layout_position != null && !msg.layout_position.isEmpty()) {
                mLayoutPosition = msg.layout_position;
            }
            LogService.d(TAG, "layout_position - LayoutName: " + mLayoutName
                    +", LayoutDevices: "+mLayoutDevices+", LayoutPosition: "+ mLayoutPosition);
        } else if (mRequest.equals(NetWorkMessage.RequestType.CAPTURE_COMPRESSION.getName())) {
            if (msg.compression != null && !msg.compression.isEmpty()) {
                mDefaultCompression = Double.parseDouble(msg.compression);
            } else {
                mDefaultCompression = 0.0;
            }
            LogService.d(TAG, "capture-compression - DefaultCompression: " + mDefaultCompression);
        } else if (mRequest.equals(NetWorkMessage.RequestType.BUFFER_CAPTURE.getName())) {
            setFormatDimension(msg);

            if (msg.source != null && !msg.source.isEmpty()) {
                mBufferSource = getSourceByName(msg.source);
            } else {
                mBufferSource = CameraSource.DEVICE;
            }

            if (msg.frames != null && !msg.frames.isEmpty()) {
                mBufferFrames = Integer.parseInt(msg.frames);
            } else {
                // getBufferFrames() == 0 will cause send raw to host
                mBufferFrames = -1;
            }

            if (msg.capture_buffer_id != null && !msg.capture_buffer_id.isEmpty()) {
                mCaptureBufferBufferID = msg.capture_buffer_id;
            } else {
                mCaptureBufferBufferID = null;
            }

            mIrFloodOn = msg.ir_flood;
            mIrProjectorOn = msg.ir_projector;
            mReturnFirstFrame = msg.return_first_frame;

            if (msg.adjust_color_gain_luminocity != null && !msg.adjust_color_gain_luminocity.isEmpty()) {
                mStatsColorGainLuminocity = Integer.parseInt(msg.adjust_color_gain_luminocity);;
            } else {
                mStatsColorGainLuminocity = 0;
            }

            if (msg.adjust_color_gain_red != null && !msg.adjust_color_gain_red.isEmpty()) {
                mStatsColorGainRed = Integer.parseInt(msg.adjust_color_gain_red);
            } else {
                mStatsColorGainRed = 0;
            }

            if (msg.adjust_color_gain_green != null && !msg.adjust_color_gain_green.isEmpty()) {
                mStatsColorGainGreen = Integer.parseInt(msg.adjust_color_gain_green);
            } else {
                mStatsColorGainGreen = 0;
            }

            if (msg.adjust_color_gain_blue != null && !msg.adjust_color_gain_blue.isEmpty()) {
                mStatsColorGainBlue = Integer.parseInt(msg.adjust_color_gain_blue);
            } else {
                mStatsColorGainBlue = 0;
            }

            if (msg.adjust_color_gain_expline != null && !msg.adjust_color_gain_expline.isEmpty()) {
                mStatsColorExpLine = Integer.parseInt(msg.adjust_color_gain_expline);
            } else {
                mStatsColorExpLine = 0;
            }

            if (msg.irexp != null && !msg.irexp.isEmpty()) {
                mIrExp = Float.parseFloat(msg.irexp);
            } else {
                mIrExp = 3.5f;
            }

            mTrackingFace = (msg.capture_tracking != null && msg.capture_tracking.contains("FACE"));
            mCapturingStatsColor = (msg.capture_stats != null && msg.capture_stats.contains("COLOR"));

            LogService.d(TAG, NetWorkMessage.RequestType.BUFFER_CAPTURE + ", BufferSource: "+mBufferSource
                    +", BufferFormat: "+mBufferFormat+", BufferWidth: "+mBufferWidth
                    +", BufferHeight: "+mBufferHeight+", IrProjectorOn: "+mIrProjectorOn
                    +", IrFloodOn: "+mIrFloodOn+", BufferFrames: "+mBufferFrames +", IrExp : "+mIrExp
                    +", white-balance: "+mStatsColorExpLine+","+mStatsColorGainLuminocity+","+mStatsColorGainRed+","+mStatsColorGainGreen+","+mStatsColorGainBlue);

        } else if (mRequest.equals(NetWorkMessage.RequestType.STREAM_CAPTURE.getName())) {
            if (msg.depth_frames != null && !msg.depth_frames.isEmpty()) {
                mDepthFrames = Integer.parseInt(msg.depth_frames);
            } else {
                mDepthFrames = 0;
            }

            if (msg.stream_capture_id != null && !msg.stream_capture_id.isEmpty()) {
                mStreamCaptureBufferBufferID = msg.stream_capture_id;
            } else {
                mStreamCaptureBufferBufferID = null;
            }

            if (msg.adjust_color_gain_luminocity != null && !msg.adjust_color_gain_luminocity.isEmpty()) {
                mStatsColorGainLuminocity = Integer.parseInt(msg.adjust_color_gain_luminocity);;
            } else {
                mStatsColorGainLuminocity = 0;
            }

            if (msg.adjust_color_gain_red != null && !msg.adjust_color_gain_red.isEmpty()) {
                mStatsColorGainRed = Integer.parseInt(msg.adjust_color_gain_red);;
            } else {
                mStatsColorGainRed = 0;
            }

            if (msg.adjust_color_gain_green != null && !msg.adjust_color_gain_green.isEmpty()) {
                mStatsColorGainGreen = Integer.parseInt(msg.adjust_color_gain_green);;
            } else {
                mStatsColorGainGreen = 0;
            }

            if (msg.adjust_color_gain_blue != null && !msg.adjust_color_gain_blue.isEmpty()) {
                mStatsColorGainBlue = Integer.parseInt(msg.adjust_color_gain_blue);;
            } else {
                mStatsColorGainBlue = 0;
            }

            mTrackingFace = (msg.capture_tracking != null && msg.capture_tracking.contains("FACE"));
            mCapturingStatsColor = (msg.capture_stats != null && msg.capture_stats.contains("COLOR"));

            if (msg.adjust_color_gain_expline != null && !msg.adjust_color_gain_expline.isEmpty()) {
                mStatsColorExpLine = Integer.parseInt(msg.adjust_color_gain_expline);
            } else {
                mStatsColorExpLine = 0;
            }

            mIrProjectorOn = msg.ir_projector;

            LogService.d(TAG, NetWorkMessage.RequestType.STREAM_CAPTURE + ", DepthFrames: "+mDepthFrames
                    +", white-balance: "+mStatsColorExpLine+","+mStatsColorGainLuminocity+","+mStatsColorGainRed+","+mStatsColorGainGreen+","+mStatsColorGainBlue);

        } else if (mRequest.equals(NetWorkMessage.RequestType.STREAM_DEPTH.getName())) {
            mStreamDepthBufferFrames = msg.streamDepth_frame_number;
            mFPS = msg.streamDepth_frame_fps;
            mColorRatio = msg.streamDepth_color_frame_ratio;
            mdoManualReCalibration = msg.do_ManualRecalibration;
            /* mStreamDepthStartIndex > 1 will use start index */
            mStreamDepthStartIndex = ((msg.streamDepth_start_index == null  || Integer.valueOf(msg.streamDepth_start_index) < 0) ? 1
                    : Integer.valueOf(msg.streamDepth_start_index));

            if (msg.streamDepth_buffer_id != null && !msg.streamDepth_buffer_id.isEmpty()) {
                mStreamDepthBufferBufferID = msg.streamDepth_buffer_id;
            } else {
                mStreamDepthBufferBufferID = null;
            }

            if (msg.adjust_color_gain_luminocity != null && !msg.adjust_color_gain_luminocity.isEmpty()) {
                mStatsColorGainLuminocity = Integer.parseInt(msg.adjust_color_gain_luminocity);;
            } else {
                mStatsColorGainLuminocity = 0;
            }

            if (msg.adjust_color_gain_red != null && !msg.adjust_color_gain_red.isEmpty()) {
                mStatsColorGainRed = Integer.parseInt(msg.adjust_color_gain_red);;
            } else {
                mStatsColorGainRed = 0;
            }

            if (msg.adjust_color_gain_green != null && !msg.adjust_color_gain_green.isEmpty()) {
                mStatsColorGainGreen = Integer.parseInt(msg.adjust_color_gain_green);;
            } else {
                mStatsColorGainGreen = 0;
            }

            if (msg.adjust_color_gain_blue != null && !msg.adjust_color_gain_blue.isEmpty()) {
                mStatsColorGainBlue = Integer.parseInt(msg.adjust_color_gain_blue);;
            } else {
                mStatsColorGainBlue = 0;
            }

            mCapturingStatsColor = (msg.capture_stats != null && msg.capture_stats.contains("COLOR"));

            if (msg.adjust_color_gain_expline != null && !msg.adjust_color_gain_expline.isEmpty()) {
                mStatsColorExpLine = Integer.parseInt(msg.adjust_color_gain_expline);
            } else {
                mStatsColorExpLine = 0;
            }

            if (msg.irexp != null && !msg.irexp.isEmpty()) {
                mIrExp = Float.parseFloat(msg.irexp);
            } else {
                mIrExp = 3.5f;
            }

            LogService.d(TAG, NetWorkMessage.RequestType.STREAM_DEPTH + ", BufferFrames : "+ mStreamDepthBufferFrames +", IrExp : "+mIrExp
                    +", white-balance: "+mStatsColorExpLine+","+mStatsColorGainLuminocity+","+mStatsColorGainRed+","+mStatsColorGainGreen+","+mStatsColorGainBlue);
        } else if (mRequest.equals(NetWorkMessage.RequestType.BUFFER_COLOR.getName())) {
            if (msg.selectedFrameIndices != null && !msg.selectedFrameIndices.isEmpty()) {
                mSelectedFrameIndices = msg.selectedFrameIndices;
            } else {
                mSelectedFrameIndices = new ArrayList<>();
            }

            if (msg.color_buffer_id != null && !msg.color_buffer_id.isEmpty()) {
                mBufferColorBufferBufferID = msg.color_buffer_id;
            } else {
                mBufferColorBufferBufferID = null;
            }

            if (msg.request_type != null && !msg.request_type.isEmpty()) {
                mBufferColorRequestType = msg.request_type;
            } else {
                mBufferColorRequestType = null;
            }
        } else if (mRequest.equals(NetWorkMessage.RequestType.BUFFER_CANCEL.getName()))  {
            if (msg.cancel_buffer_id != null && !msg.cancel_buffer_id.isEmpty()) {
                mBufferCancelBufferID = msg.cancel_buffer_id;
            } else {
                mBufferCancelBufferID = null;
            }
        } else if (mRequest.equals(NetWorkMessage.RequestType.STREAM_START.getName())) {
            setFormatDimension(msg);
            if (msg.source != null && !msg.source.isEmpty()) {
                mSource = getSourceByName(msg.source);
            } else {
                mSource = CameraSource.DEVICE;
            }

            mFPS = msg.request_fps;

            if (msg.frames != null && !msg.frames.isEmpty()) {
                mStreamFrames = Integer.parseInt(msg.frames);
            } else {
                mStreamFrames = 0;
            }

            mTrackingFace = (msg.capture_tracking != null && msg.capture_tracking.contains("FACE"));
            mCapturingStatsColor = (msg.capture_stats != null && msg.capture_stats.contains("COLOR"));
            streamMode = msg.stream_mode;

            if (msg.irexp != null && !msg.irexp.isEmpty()) {
                mIrExp = Float.parseFloat(msg.irexp);
            } else {
                mIrExp = 3.5f;
            }

            mIsDiagnostic = msg.is_diagnostic;

            LogService.d(TAG, NetWorkMessage.RequestType.STREAM_START + ", Source: "+mSource+", FormattingService.Format: "+mFormat+", Width: "+mWidth+", Height: "+mHeight+", Compression: "+mCompression+", FPS: "+mFPS+" ,StreamFrames : " + mStreamFrames + " , rExp : " + mIrExp
                    + " , streamMode : " + streamMode + " , mIsDiagnostic : " + mIsDiagnostic);
        } else if (mRequest.equals(NetWorkMessage.RequestType.STREAM_STOP.getName())) {
            streamMode = msg.stream_mode;
        } else if (mRequest.equals(NetWorkMessage.RequestType.BUFFER_FETCH.getName())) {
            if (msg.fetch_buffer_id != null && !msg.fetch_buffer_id.isEmpty()) {
                mFetchBufferBufferID = msg.fetch_buffer_id;
            } else {
                mFetchBufferBufferID = null;
            }

            mFetchBufferType = msg.fetch_frame_type;

            LogService.d(TAG, NetWorkMessage.RequestType.BUFFER_FETCH +", BufferId: " + mFetchBufferBufferID +", BufferType: " + mFetchBufferType);
        } else if (mRequest.equals(NetWorkMessage.RequestType.BUFFER_MERGE.getName())) {
            if (msg.merge_buffer_id != null && !msg.merge_buffer_id.isEmpty()) {
                mMergeBufferBufferD = msg.merge_buffer_id;
            } else {
                mMergeBufferBufferD = null;
            }

            mdoManualReCalibration = msg.do_ManualRecalibration;

            mFaceDistance = new double[3];
            mFaceDistance[0] = Double.parseDouble(msg.distance.getX());
            mFaceDistance[1] = Double.parseDouble(msg.distance.getY());
            mFaceDistance[2] = Double.parseDouble(msg.distance.getZ());

            mFaceRect = new double[4];
            mFaceRect[0] = Double.parseDouble(msg.faceRect.getX());
            mFaceRect[1] = Double.parseDouble(msg.faceRect.getY());
            mFaceRect[2] = Double.parseDouble(msg.faceRect.getWidth());
            mFaceRect[3] = Double.parseDouble(msg.faceRect.getHeight());

            mxform = new double[msg.xf.size()];
            for(int x = 0 ; x < msg.xf.size(); x++) {
                mxform[x] = Double.parseDouble(msg.xf.get(x));
            }

            LogService.d(TAG, NetWorkMessage.RequestType.BUFFER_MERGE +", BufferId: "+mMergeBufferBufferD +", DoManualRecalibration: "+ mdoManualReCalibration +", "
                    +", TrackingFace: "+mFaceDistance[0]+","+mFaceDistance[1]+","+mFaceDistance[2]
                    +", TrackingFaceRect : "+mFaceRect[0]+","+mFaceRect[1]+","+mFaceRect[2]+","+mFaceRect[3]
                    +", xform length : "+mxform.length);

        } else if (mRequest.equals(NetWorkMessage.RequestType.BUFFER_RELEASE.getName())) {
            if (msg.release_buffer_id != null && !msg.release_buffer_id.isEmpty()) {
                mReleaseBufferBufferID = msg.release_buffer_id;
            } else {
                mReleaseBufferBufferID = null;
            }
        }
    }

    public void update(NetWorkMessage msg) {
        if (msg == null) return;

        if (mRequest.equals(NetWorkMessage.RequestType.STREAM_START.getName())) {
            if (msg.request == null || msg.request.isEmpty()) return;
            mSubRequest = msg.request;
            if (msg.request_id == null || msg.request_id.isEmpty()) return;
            mSubRequestId = msg.request_id;

            if(msg.request.equals(NetWorkMessage.RequestType.BUFFER_SNAPSHOT.getName())) {
                if (msg.bufferSnapshot_IRFrameNumber != null && !msg.bufferSnapshot_IRFrameNumber.isEmpty()) {
                    mBufferSnapFrames = Integer.parseInt(msg.bufferSnapshot_IRFrameNumber);
                } else {
                    mBufferSnapFrames = 0;
                }

                if (msg.bufferSnapshot_frameRate != null && !msg.bufferSnapshot_frameRate.isEmpty()) {
                    mBufferSnapFramesFPS = Double.parseDouble(msg.bufferSnapshot_frameRate);
                } else {
                    mBufferSnapFramesFPS = 0.0;
                }

                if (msg.bufferSnapshot_buffer_id != null && !msg.bufferSnapshot_buffer_id.isEmpty()) {
                    mBufferSnapFramesBufferID = msg.bufferSnapshot_buffer_id;
                    String[] separated = mBufferSnapFramesBufferID.split("-");
                    mBufferSnapCamPose = separated[separated.length-1];
                } else {
                    mBufferSnapFramesBufferID = null;
                    mBufferSnapCamPose = null;
                }
                LogService.d(TAG, NetWorkMessage.RequestType.BUFFER_SNAPSHOT + ", BufferId: "+mBufferSnapFramesBufferID +", IRFrameNumber: "+mBufferSnapFrames+", "
                        +", Frame Rate : "+mBufferSnapFramesFPS);
            } else if (msg.request.equals(NetWorkMessage.RequestType.BUFFER_SNAPSHOT_RECEIVE.getName())) {
                if (msg.bufferReceive_buffer_id != null && !msg.bufferReceive_buffer_id.isEmpty()) {
                    mBufferReceiveBufferID = msg.bufferReceive_buffer_id;
                } else {
                    mBufferReceiveBufferID = null;
                }

                mIsStreamPause = msg.isStreamPause;

                LogService.d(TAG, NetWorkMessage.RequestType.BUFFER_SNAPSHOT_RECEIVE + ", BufferId: "+mBufferReceiveBufferID +", mIsStreamPause : "+ mIsStreamPause);
            }
        }
    }

    public String set(CameraSource source, Format format, int width, int height, double fps) {
        if (mSupportedSources != null) {
            if (!mSupportedSources.contains(source)) return "ERRROR: Unsupported source";
        }
        mSource = source;

        if (mSupportedFormats != null) {
            if (!mSupportedFormats.contains(format)) return "ERROR: Unsupported format";
        }
        mFormat = format;

        if (mSupportedDimensions != null) {
            if (!mSupportedDimensions.contains(width+"x"+height)) return "ERROR: Unsupported format";
        }
        mWidth = width;
        mHeight = height;
        mFPS = fps;

        return null;
    }

    public void setSupportedSources(ArrayList<CameraSource> sources) {
        mSupportedSources = sources;
    }

    public void setSupportedFormats(ArrayList<Format> formats) {
        mSupportedFormats = formats;
    }

    public void setSupportedDimensions(ArrayList<String> dimensions) {
        mSupportedDimensions = dimensions;
    }

    public ArrayList<CameraSource> getSupportedSources() {
        return mSupportedSources;
    }

    public ArrayList<Format> getSupportedFormats() {
        return mSupportedFormats;
    }

    public ArrayList<String> getSupportedFormatsDimensions() {
        return mSupportedDimensions;
    }

    public CameraSource getSource() {return mSource;}

    public CameraSource getBufferSource() {return mBufferSource;}

    public String getSourceName(CameraSource source) {
        return sourceNames.get(source.ordinal());
    }

    public CameraSource getSourceByName(String source) {
        if (mSupportedSources == null || mSupportedSources.size() == 0) return CameraSource.DEVICE;
        if (source == null || source.isEmpty()) return CameraSource.DEVICE;

        int index = sourceNames.indexOf(source.toUpperCase());
        return CameraSource.values()[index];
    }

    public Format getFormat() {return mFormat;}

    public String getFormatName(Format format) {
        return formatNames.get(format.ordinal());
    }

    public Format getFormatByName(String format) {
        int index = formatNames.indexOf(format.toUpperCase());
        return Format.values()[index];
    }

    public void setSize(int width, int height) {
        mWidth = width;
        mHeight = height;
    }

    public String getDimension() {return mWidth+"x"+mHeight;}

    public int getWidth() {return mWidth;}

    public int getHeight() {return mHeight;}

    public double getFPS() {return mFPS;}

    public int getStreamFrames() { return mStreamFrames; }


    public String getLayoutPosition() {
        if (mLayoutPosition != null && !mLayoutPosition.isEmpty()) return mLayoutPosition;
        return null;
    }

    public String getLayoutName() {
        if (mLayoutName != null && !mLayoutName.isEmpty()) return mLayoutName;
        return null;
    }

    public int getLayoutDevices() {
        return mLayoutDevices;
    }

    public double getCompression() {return mCompression;}

    public void setMaxBufferFrames(int count) {mMaxBufferFrames = count;}
    public int getMaxBufferFrames() {return mMaxBufferFrames;}

    public int getBufferFrames() {return mBufferFrames;}
    public int getDepthFrames() {return mDepthFrames;}
    public List<Integer> getSelectedFrameIndices() { return mSelectedFrameIndices;}

    public int getStreamDepthFrames() {return mStreamDepthBufferFrames;}
    public void setStreamDepthFrames(int frames) {mStreamDepthBufferFrames = frames;}
    public double getStreamDepthFramesFps() {return mFPS;}
    public double getStreamDepthColorRatio() {return mColorRatio;}
    public int getStreamDepthStartIndex() {return mStreamDepthStartIndex;}

    public int getBufferWidth() {return mBufferWidth;}

    public int getBufferHeight() {return mBufferHeight;}

    public Format getBufferFormat() {return mBufferFormat;}

    public double getInterval() {return mInterval;};

    public double getDuration() {return mDuration;};

    public boolean isDebug() {return mDebug;}

    public boolean isIrFloodOn() {return mIrFloodOn;}

    public boolean isIrProjectorOn() {return mIrProjectorOn;}

    public boolean isReturnFirstFrame() { return mReturnFirstFrame;}

    public String getRequestID() {return mRequestId;}

    public String getRequest() {return mRequest;}

    public String getBufferColorRequestType() {return mBufferColorRequestType;}

    public String getCaptureBufferID () { return mCaptureBufferBufferID; }

    public String getMergeBufferID () { return mMergeBufferBufferD; }

    public String getFetchBufferID () { return mFetchBufferBufferID; }

    public String getReleaseBufferID () { return mReleaseBufferBufferID; }

    public String getStreamCaptureBufferID() { return mStreamCaptureBufferBufferID; }

    public String getBufferColorBufferID () { return mBufferColorBufferBufferID; }

    public String getBufferCancelBufferID() { return mBufferCancelBufferID; }

    public String getStreamDepthBufferID() { return mStreamDepthBufferBufferID; }

    public boolean isTrackingFace () { return mTrackingFace; }

    public void setIsTrackingFace (boolean isTrackingFace) { mTrackingFace = isTrackingFace; }

    public boolean hasTrackingFace () {
        if (mTrackingFaceX != 0.0) return true;
        if (mTrackingFaceY != 0.0) return true;
        if (mTrackingFaceZ != 0.0) return true;
        if (mTrackingFaceRotationX != 0.0) return true;
        if (mTrackingFaceRotationY != 0.0) return true;
        if (mTrackingFaceRotationY != 0.0) return true;
        return false;
    }

    public void setTrackingFace (float[] headPoseInfo) {
        if(headPoseInfo == null)
            mTrackingFaceX = mTrackingFaceY = mTrackingFaceZ =
                    mTrackingFaceRotationX = mTrackingFaceRotationY = mTrackingFaceRotationZ =
                            mTrackingFaceFaceRectX = mTrackingFaceFaceRectY = mTrackingFaceFaceRectWidth = mTrackingFaceFaceRectHeight = 0;
        else {
            mTrackingFaceX = headPoseInfo[0];
            mTrackingFaceY = headPoseInfo[1];
            mTrackingFaceZ = headPoseInfo[2];
            mTrackingFaceRotationX = headPoseInfo[3];
            mTrackingFaceRotationY = headPoseInfo[4];
            mTrackingFaceRotationZ = headPoseInfo[5];
            mTrackingFaceFaceRectX = headPoseInfo[6];
            mTrackingFaceFaceRectY = headPoseInfo[7];
            mTrackingFaceFaceRectWidth = headPoseInfo[8];
            mTrackingFaceFaceRectHeight = headPoseInfo[9];
        }
    }

    public double getTrackingFaceX () { return mTrackingFaceX; }
    public double getTrackingFaceY () { return mTrackingFaceY; }
    public double getTrackingFaceZ () { return mTrackingFaceZ; }
    public double getTrackingFaceRotationX () { return mTrackingFaceRotationX; }
    public double getTrackingFaceRotationY () { return mTrackingFaceRotationY; }
    public double getTrackingFaceRotationZ () { return mTrackingFaceRotationZ; }
    public double getTrackingFaceFaceRectX () { return mTrackingFaceFaceRectX; }
    public double getTrackingFaceFaceRectY () { return mTrackingFaceFaceRectY; }
    public double getTrackingFaceFaceRectWidth () { return mTrackingFaceFaceRectWidth; }
    public double getTrackingFaceFaceRectHeight () { return mTrackingFaceFaceRectHeight; }
    public double[] getFaceRectA () { return mFaceRect; }
    public double[] getFaceDistanceA () { return mFaceDistance; }
    public double[] getxFormA () { return mxform; }


    public boolean isCapturingStatsColor () { return mCapturingStatsColor; }

    public boolean hasStatsColor () {
        if (mStatsColorGainLuminocity == 0) return false;
        if (mStatsColorExpLine == 0) return false;
        if (mStatsColorGainRed == 0) return false;
        if (mStatsColorGainGreen == 0) return false;
        if (mStatsColorGainBlue == 0) return false;
        return true;
    }

    public boolean isDiagnostic() {return mIsDiagnostic;}

    public void setStatsColorGain (int expline, int gainLuminocity, int gainRed, int gainGreen, int gainBlue) {
        mStatsColorExpLine        = expline;
        mStatsColorGainLuminocity = gainLuminocity;
        mStatsColorGainLuminocity = gainRed;
        mStatsColorGainLuminocity = gainGreen;
        mStatsColorGainLuminocity = gainBlue;
    }

    public int getStatsColorExpLine () { return mStatsColorExpLine; }
    public int getStatsColorGainLuminocity () { return mStatsColorGainLuminocity; }
    public int getStatsColorGainRed () { return mStatsColorGainRed; }
    public int getStatsColorGainGreen () { return mStatsColorGainGreen; }
    public int getStatsColorGainBlue () { return mStatsColorGainBlue; }

    public float getIrExp () { return mIrExp; }

    public boolean getDoManualReCalibration () { return mdoManualReCalibration; }

    public String getStreamMode() { return streamMode;}

    public String getBufferCaptureMode() { return mbufferCaptureMode;}

    public void setBufferFetchSize(int bufferFetchSize) { this.mBufferFetchSize = bufferFetchSize; }
    public int getBufferFetchSize() { return mBufferFetchSize;}

    public int getBufferFetchType() { return mFetchBufferType;}

    /* sub command */
    public int getBufferSnapFrames() {return mBufferSnapFrames;}
    public double getBufferSnapFramesFPS() {return mBufferSnapFramesFPS;}
    public String getBufferSnapBufferID () { return mBufferSnapFramesBufferID;}
    public String getSubRequestID() {return mSubRequestId;}
    public String getSubRequest() {return mSubRequest;}
    public String getBufferSnapCamPose() {return mBufferSnapCamPose;}

    public String getBufferSnapReceiveBufferID () { return mBufferReceiveBufferID;}
}
