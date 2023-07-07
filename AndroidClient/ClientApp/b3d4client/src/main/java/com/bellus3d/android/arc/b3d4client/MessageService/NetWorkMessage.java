package com.bellus3d.android.arc.b3d4client.MessageService;

import com.bellus3d.android.arc.b3d4client.AppStatusManager;
import com.bellus3d.android.arc.b3d4client.JsonModel.*;
import com.bellus3d.android.arc.b3d4client.JsonModel.innerjsonmodel.Position;
import com.bellus3d.android.arc.b3d4client.LogService.LogService;
import com.google.gson.Gson;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.List;
import java.util.Vector;

import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.TAG;

public class NetWorkMessage {

    public enum HostCommands {
        CMD_LAYOUT_POSITION,
        CMD_CAPTURE_COMPRESSION,
        CMD_CAMERA_CONFIGURATION,
        CMD_BUFFER_COLOR,
        CMD_STREAM_DEPTH,
        CMD_BUFFER_CAPTURE,
        CMD_BUFFER_FETCH,
        CMD_BUFFER_SNAPSHOT,
        CMD_BUFFER_SNAPSHOT_RECEIVE,
        CMD_BUFFER_MERGE,
        CMD_BUFFER_RELEASE,
        CMD_BUFFER_CANCEL,
        CMD_STREAM_CAPTURE,
        CMD_START_STREAM,
        CMD_STOP_STREAM,
        CMD_LAMP_ON,
        CMD_LAMP_BLINK,
        CMD_LAMP_OFF,
        CMD_DEVICE_INFO,
        CMD_CAMERA_SOURCES,
        CMD_SOURCE_FORMATS,
        CMD_FORMAT_DIMENSIONS,
        CMD_BUFEER_MAX_FRAMES,
        CMD_BUFFERS_MAX,
        CMD_FRAME_RATES,
        CMD_RECALIBRATION,
        CMD_CANCEL_PROCESS,
        CMD_END
    }

    public enum ErrorRequest {
        REQUEST_STOP_STREAM,
        REQUEST_RESTART_CLIENT,
        REQUEST_REBOOT
    }

    public enum RequestType{
        BUFFER_CAPTURE("buffer_capture"),
        BUFFER_COLOR("buffer_color"),
        BUFFER_FETCH("buffer_fetch"),
        BUFFER_MERGE("buffer_merge"),
        BUFFER_RELEASE("buffer_release"),
        BUFFER_CANCEL("buffer_cancel"),
        BUFFER_SNAPSHOT("buffer_snapshot"),
        BUFFER_SNAPSHOT_RECEIVE("buffer_snapshot_receive"),
        STREAM_CAPTURE("stream_capture"),
        STREAM_DEPTH("stream_depth"),
        CAPTURE_COMPRESSION("capture_compression"),
        CAMERA_CONFIGURATION("camera_configuration"),
        CANCEL_PROCESS("process_cancel"),
        CONNECT_INVITE("connect_invite"),
        DEVICE_INFO("device_info"),
        DISCONNECT("disconnect"),
        GET_RESOURCE_INFO("GetSourceInfo"),
        HELLO("hello"),
        KEEP_ALIVE("keep_alive"),
        LAYOUT_POSITION("layout_position"),
        MAX_BUFFER_FRAMES("max_buffer_frames"),
        MERGE("merge"),
        PING("ping"),
        PONG("pong"),
        SOURCE_FRAME_RATES("source_framerates"),
        START_BENCH("start_bench"),
        STATUS_BLINK("status_blink"),
        STATUS_OFF("status_off"),
        STATUS_ON("status_on"),
        STREAM("stream"),
        STREAM_START("stream_start"),
        STREAM_STOP("stream_stop"),
        RECALIBRATION("reCalibration");

        private String name;

        private RequestType(String name) {
            this.name = name;
        }

        public String getName() {
            return this.name;
        }
    }

    public enum Status{
        OK("OK"),
        ERROR("ERROR");
        private String name;

        private Status(String name) {
            this.name = name;
        }

        public String getName() {
            return this.name;
        }
    }

    public enum ResponseTo{
        CONNECT("connect"),
        STATUS_BLINK("status_blink"),
        STATUS_ON("status_on"),
        STATUS_OFF("status_off"),
        STITCH("stitch"),
        STREAM_STOP("stream_stop"),
        UPLOAD("upload");

        private String name;

        private ResponseTo(String name) {
            this.name = name;
        }

        public String getName() {
            return this.name;
        }

    }

    enum Type{
        KEEP_ALIVE
    }

    public NetWorkMessage() {
        camera_sources = new Vector();
        sensor_formats = new Vector();
        sensor_dimensions = new Vector();
        tracking_types = new Vector();
        stat_types = new Vector();

        /* init tacking and stat type */
        tracking_types.add("FACE");
        stat_types.add("COLOR");
    }

    public HostCommands cmds;

    /* common */
    public String request;
    public String request_id;
    public String source;
    public String dimension;
    public String format;

    public boolean debug;
    public String status;
    public String error;

    public String interval;
    public String duration;
    public String color;
    public Vector<String> camera_sources;
    public Vector<String> sensor_formats;
    public Vector<String> sensor_dimensions;

    /* supported  track type and AE/AWB stats */
    public Vector<String> tracking_types;
    public Vector<String> stat_types;

    /* Stream Start */
    public String frames;
    public double request_fps;
    public boolean is_diagnostic;

    /* Stream Stop*/
    public String stream_mode;

    /* indicate current stats type */
    public List<String> capture_stats;
    /* M cam ae gain, awb r/g/b gain */
    public String adjust_color_gain_luminocity;
    public String adjust_color_gain_red;
    public String adjust_color_gain_green;
    public String adjust_color_gain_blue;
    public String adjust_color_gain_expline;

    /* indicate current tracking type */
    public List<String> capture_tracking;

    /* Buffer Capture */
    public boolean ir_projector;
    public boolean ir_flood;
    public boolean return_first_frame;
    public String capture_buffer_id;
    public String irexp;
    public Position faceRect;
    public Position distance;
    public List<String> xf;

    /* Buffer Raw */
    public String depth_frames;
    public String stream_capture_id;

    /* Buffer Color */
    public List<Integer> selectedFrameIndices;
    public String color_buffer_id;
    public String request_type;

    /* Buffer Cancel */
    public String cancel_buffer_id;

    /* Buffer Merge */
    public String merge_buffer_id;
    public boolean do_ManualRecalibration;

    /* Buffer fetch */
    public String fetch_buffer_id;
    public int fetch_frame_type;

    /* Buffer Release */
    public String release_buffer_id;

    /* Stream Depth */
    public String streamDepth_buffer_id;
    public int streamDepth_frame_fps;
    public int streamDepth_frame_number;
    public double streamDepth_color_frame_ratio;
    public String streamDepth_start_index;

    /* Buffer Snapshot */
    public String bufferSnapshot_IRFrameNumber;
    public String bufferSnapshot_frameRate;
    public String bufferSnapshot_buffer_id;

    /* Buffer Receive */
    public Boolean isStreamPause;
    public String bufferReceive_buffer_id;

    public String compression;
    public String layout;
    public String layout_devices;
    public String layout_position;
    public String ping_type;

    public static JSONObject buildNetMessage(NetWorkMessage netMsg, String message) {
        Gson gson = new Gson();
        Common common = gson.fromJson(message, Common.class);
        JSONObject json = null;
        try {
            json = new JSONObject(message);
        } catch (JSONException e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }

        netMsg.debug = common.isDebug();

        netMsg.status = common.getStatus();

        if (netMsg.status != null && !netMsg.status.equals(Status.OK.getName())) {
            netMsg.error = common.getError();
            AppStatusManager.getInstance().setState(AppStatusManager.AppState.WEBSOCKET_CONNECTED);
            LogService.e(TAG, " error status "+ netMsg.status+": "+netMsg.error);
            return null;
        } else {
            netMsg.status = Status.OK.getName();
            //if Arc1 build mesh success
            AppStatusManager.getInstance().setState(AppStatusManager.AppState.WEBSOCKET_CONNECTED);
            netMsg.error = "";
        }

        /* this is for all message will parse */
        netMsg.request_id = common.getRequestId();
        if (netMsg.request_id.isEmpty()) {
            LogService.e(TAG, " missing request_id : " + message);
            return null;
        }

        netMsg.request = common.getRequest();
        LogService.d(TAG, " netMsg.request:" + netMsg.request);
        if(netMsg.request != null) {
            if(netMsg.request.equals(RequestType.STREAM_START.getName())) {
                StreamStartRequest streamStartRequest = gson.fromJson(message, StreamStartRequest.class);
                if (streamStartRequest != null) {
                    netMsg.source = streamStartRequest.getSource();
                    netMsg.frames = String.valueOf(streamStartRequest.getFrames());
                    netMsg.dimension = streamStartRequest.getDimension();
                    netMsg.format = streamStartRequest.getFormat();
                    netMsg.request_fps = streamStartRequest.getRequestFps();
                    netMsg.capture_tracking = streamStartRequest.getTracking();
                    netMsg.capture_stats = streamStartRequest.getStats();
                    netMsg.stream_mode = streamStartRequest.get_stream_mode();
                    netMsg.irexp = streamStartRequest.getIrExp();
                    netMsg.is_diagnostic = streamStartRequest.getDiagnostic();
                } else {
                    LogService.w(TAG, "no streamStartRequest data, should be set red light");
                }
            } else if(netMsg.request.equals(RequestType.STREAM_STOP.getName())){
                StreamStopRequest streamStopRequest = gson.fromJson(message, StreamStopRequest.class);
                netMsg.stream_mode = streamStopRequest.get_stream_mode();
            }else if(netMsg.request.equals(RequestType.BUFFER_CAPTURE.getName())) {
                CaptureRequest captureRequest = gson.fromJson(message, CaptureRequest.class);
                if (captureRequest != null) {
                    netMsg.source = captureRequest.getSource();
                    netMsg.frames = String.valueOf(captureRequest.getFrames());
                    netMsg.dimension = captureRequest.getDimension();
                    netMsg.format = captureRequest.getFormat();
                    netMsg.ir_projector = captureRequest.isIrProjector();
                    netMsg.ir_flood = captureRequest.isIrFlood();
                    netMsg.capture_buffer_id = captureRequest.getBufferId();
                    netMsg.return_first_frame = captureRequest.isReturnFirstFrame();
                    netMsg.irexp = captureRequest.getIrExp();
                    if (captureRequest.getAdjust() != null) {
                        if (captureRequest.getAdjust().getColor() != null) {
                            netMsg.adjust_color_gain_luminocity = captureRequest.getAdjust().getColor().getY();
                            netMsg.adjust_color_gain_red = captureRequest.getAdjust().getColor().getR();
                            netMsg.adjust_color_gain_green = captureRequest.getAdjust().getColor().getG();
                            netMsg.adjust_color_gain_blue = captureRequest.getAdjust().getColor().getB();
                            netMsg.adjust_color_gain_expline = captureRequest.getAdjust().getColor().getExposureLine();
                        } else {
                            LogService.w(TAG, "no color value, should be set red light");
                        }
                    } else {
                        LogService.w(TAG, "no adjust value, should be set red light");
                    }
                } else {
                    LogService.w(TAG, "no captureRequest data, should be set red light");
                }
            } else if(netMsg.request.equals(RequestType.PING.getName())) {
                netMsg.ping_type = common.getType();
            } else if(netMsg.request.equals(RequestType.BUFFER_MERGE.getName())) {
                BufferIdRequest bufferIdRequest = gson.fromJson(message, BufferIdRequest.class);
                if (bufferIdRequest != null) {
                    netMsg.merge_buffer_id = bufferIdRequest.getBufferId();
                    netMsg.do_ManualRecalibration = bufferIdRequest.isManual_recalibration();
                    netMsg.faceRect = bufferIdRequest.getTracking().getFaceTrackingInfo().getFaceRect();
                    netMsg.distance = bufferIdRequest.getTracking().getFaceTrackingInfo().getDistance();
                    netMsg.xf = bufferIdRequest.getXf();
                } else {
                    LogService.w(TAG, "no merge bufferIdRequest data, should be red light");
                }
            } else if(netMsg.request.equals(RequestType.BUFFER_FETCH.getName())) {
                BufferFetchRequest bufferFetchRequest = gson.fromJson(message, BufferFetchRequest.class);
                if (bufferFetchRequest != null) {
                    netMsg.fetch_buffer_id = bufferFetchRequest.getBufferId();
                    netMsg.fetch_frame_type = bufferFetchRequest.getBufferType();
                } else {
                    LogService.w(TAG, "no release bufferIdRequest data, should be red light");
                }
            } else if(netMsg.request.equals(RequestType.BUFFER_RELEASE.getName())) {
                BufferIdRequest bufferIdRequest = gson.fromJson(message, BufferIdRequest.class);
                if (bufferIdRequest != null) {
                    netMsg.release_buffer_id = bufferIdRequest.getBufferId();
                } else {
                    LogService.w(TAG, "no release bufferIdRequest data, should be red light");
                }
            } else if(netMsg.request.equals(RequestType.STREAM_CAPTURE.getName())) {
                CaptureRequest captureRequest = gson.fromJson(message, CaptureRequest.class);
                netMsg.depth_frames = String.valueOf(captureRequest.getFrames());
                netMsg.stream_capture_id = captureRequest.getBufferId();
                if (captureRequest.getAdjust() != null) {
                    if (captureRequest.getAdjust().getColor() != null) {
                        netMsg.adjust_color_gain_luminocity = captureRequest.getAdjust().getColor().getY();
                        netMsg.adjust_color_gain_red = captureRequest.getAdjust().getColor().getR();
                        netMsg.adjust_color_gain_green = captureRequest.getAdjust().getColor().getG();
                        netMsg.adjust_color_gain_blue = captureRequest.getAdjust().getColor().getB();
                        netMsg.adjust_color_gain_expline = captureRequest.getAdjust().getColor().getExposureLine();
                    } else {
                        LogService.w(TAG, "no color value, should be set red light");
                    }
                } else {
                    LogService.w(TAG, "no adjust value, should be set red light");
                }
                LogService.d(TAG, "STREAM_CAPTURE frame index: " + netMsg.depth_frames);
            } else if(netMsg.request.equals(RequestType.BUFFER_COLOR.getName())) {
                BufferColorRequest bufferColorRequest = gson.fromJson(message, BufferColorRequest.class);
                netMsg.selectedFrameIndices = bufferColorRequest.getIndices();
                netMsg.color_buffer_id = bufferColorRequest.getBufferId();
                netMsg.request_type = bufferColorRequest.getRequestType();
                LogService.d(TAG, "BUFFER_COLOR:" + bufferColorRequest.getIndices().size());
            } else if(netMsg.request.equals(RequestType.BUFFER_CANCEL.getName())) {
                BufferIdRequest bufferCancelRequest =  gson.fromJson(message, BufferIdRequest.class);
                netMsg.cancel_buffer_id = bufferCancelRequest.getBufferId();
            } else if(netMsg.request.equals(RequestType.STREAM_DEPTH.getName())) {
                StreamDepthRequest streamDepthRequest = gson.fromJson(message, StreamDepthRequest.class);
                netMsg.streamDepth_buffer_id = streamDepthRequest.getBufferId();
                netMsg.streamDepth_frame_fps = streamDepthRequest.getFrameFPS();
                netMsg.streamDepth_frame_number = streamDepthRequest.getFrameNumber();
                netMsg.streamDepth_color_frame_ratio = streamDepthRequest.getColorFrameRatio();
                netMsg.irexp = streamDepthRequest.getIrExp();
                netMsg.streamDepth_start_index = streamDepthRequest.getStartIndex();
                netMsg.do_ManualRecalibration = streamDepthRequest.isManual_recalibration();

                if (streamDepthRequest.getAdjust() != null) {
                    if (streamDepthRequest.getAdjust().getColor() != null) {
                        netMsg.adjust_color_gain_luminocity = streamDepthRequest.getAdjust().getColor().getY();
                        netMsg.adjust_color_gain_red = streamDepthRequest.getAdjust().getColor().getR();
                        netMsg.adjust_color_gain_green = streamDepthRequest.getAdjust().getColor().getG();
                        netMsg.adjust_color_gain_blue = streamDepthRequest.getAdjust().getColor().getB();
                        netMsg.adjust_color_gain_expline = streamDepthRequest.getAdjust().getColor().getExposureLine();
                    } else {
                        LogService.w(TAG, "no color value, should be set red light");
                    }
                } else {
                    LogService.w(TAG, "no adjust value, should be set red light");
                }
            } else if(netMsg.request.equals(RequestType.BUFFER_SNAPSHOT.getName())) {
                SnapShotRequest snapShotRequest = gson.fromJson(message, SnapShotRequest.class);
                netMsg.bufferSnapshot_buffer_id = snapShotRequest.getBufferId();
                netMsg.bufferSnapshot_IRFrameNumber = snapShotRequest.getIRFrameNumber();
                netMsg.bufferSnapshot_frameRate = snapShotRequest.getFrameRate();
            } else if(netMsg.request.equals(RequestType.BUFFER_SNAPSHOT_RECEIVE.getName())) {
                SnapShotReceiveRequest receiveRequest = gson.fromJson(message, SnapShotReceiveRequest.class);
                //netMsg.bufferSnapshot_buffer_id = receiveRequest.getBufferId();
                netMsg.isStreamPause = true; // To pause stream in preview
                netMsg.bufferReceive_buffer_id = receiveRequest.getBufferId();
            } else if(netMsg.request.equals(RequestType.CANCEL_PROCESS.getName())) {
                CancelProcessRequest cancelProcessRequest = gson.fromJson(message, CancelProcessRequest.class);
            }
        }
        return json;
    }
}