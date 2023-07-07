package com.bellus3d.android.arc.b3d4client.NetworkService;

import com.bellus3d.android.arc.b3d4client.AppStatusManager;
import com.bellus3d.android.arc.b3d4client.DiskService;
import com.bellus3d.android.arc.b3d4client.LogService.LogService;
import com.bellus3d.android.arc.b3d4client.JsonModel.EventMessage;
import com.neovisionaries.ws.client.ThreadType;
import com.neovisionaries.ws.client.WebSocket;
import com.neovisionaries.ws.client.WebSocketAdapter;
import com.neovisionaries.ws.client.WebSocketException;
import com.neovisionaries.ws.client.WebSocketFactory;
import com.neovisionaries.ws.client.WebSocketFrame;
import com.neovisionaries.ws.client.WebSocketState;

import static com.bellus3d.android.arc.b3d4client.AppStatusManager.AppState.*;
import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.TAG;

import org.greenrobot.eventbus.EventBus;

import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.List;
import java.util.Map;

public class WebSocketManager extends WebSocketAdapter {

    private static WebSocketManager instance = new WebSocketManager();

    private WebSocket webSocket;
    private NetworkService networkService;

    private WebSocketManager() { }

    @Override
    public void onStateChanged(WebSocket websocket, WebSocketState newState) throws Exception {
        LogService.d(TAG, newState.name());
        switch (newState) {
            case CONNECTING:
                AppStatusManager.getInstance().setState(WEBSOCKET_CONNECTING);
                break;
            case CLOSED:
                AppStatusManager.getInstance().setState(WEBSOCKET_DISCONNECTED);
                break;
        }
    }

    @Override
    public void onConnected(WebSocket websocket, Map<String, List<String>> headers) throws Exception {
        AppStatusManager.getInstance().setState(WEBSOCKET_CONNECTED);
        for (String key : headers.keySet()) {
            LogService.d(TAG, "WebSocket header " + key + "=" + headers.get(key));
        }
    }

    @Override
    public void onConnectError(WebSocket websocket, WebSocketException cause) throws Exception {

        AppStatusManager.getInstance().setState(WEBSOCKET_CONNECTION_FAILED);

        String log = String.format("%s\n, %s\n, %s\n",
                cause.getError().name(), cause.getLocalizedMessage(),
                cause.getMessage());
        LogService.d(TAG, log);
    }

    @Override
    public void onDisconnected(WebSocket websocket, WebSocketFrame serverCloseFrame, WebSocketFrame clientCloseFrame, boolean closedByServer) throws Exception {
        AppStatusManager.getInstance().setState(WEBSOCKET_DISCONNECTED);
        String msg;
        if (closedByServer) {
            msg = String.format("disconnect by Server, CloseCode: %s, CloseReason: %s, PayloadText: %s",
                    serverCloseFrame.getCloseCode(),
                    serverCloseFrame.getCloseReason(),
                    serverCloseFrame.getPayloadText());

        } else {
            msg = String.format("disconnect by Client. CloseCode: %s, CloseReason: %s, PayloadText: %s",
                    clientCloseFrame.getCloseCode(),
                    clientCloseFrame.getCloseReason(),
                    clientCloseFrame.getPayloadText());
        }
        networkService.wsDisconnected(msg);
        LogService.d(TAG, msg);
    }

    @Override
    public void onFrame(WebSocket websocket, WebSocketFrame frame) throws Exception {
        String msg = parseWebSocketFrame(frame);
        //LogService.d(TAG, msg);
    }

    @Override
    public void onContinuationFrame(WebSocket websocket, WebSocketFrame frame) throws Exception {
        String msg = parseWebSocketFrame(frame);
        LogService.d(TAG, msg);
    }

    @Override
    public void onTextFrame(WebSocket websocket, WebSocketFrame frame) throws Exception {
        String msg = parseWebSocketFrame(frame);
        //LogService.d(TAG, msg);
    }

    @Override
    public void onBinaryFrame(WebSocket websocket, WebSocketFrame frame) throws Exception {
        String msg = parseWebSocketFrame(frame);
        LogService.d(TAG, msg);
    }

    @Override
    public void onCloseFrame(WebSocket websocket, WebSocketFrame frame) throws Exception {
        String msg = parseWebSocketFrame(frame);
        LogService.d(TAG, msg);
    }

    @Override
    public void onPingFrame(WebSocket websocket, WebSocketFrame frame) throws Exception {
        String msg = parseWebSocketFrame(frame);
        LogService.d(TAG, msg);
        networkService.keepAlive();
    }

    @Override
    public void onPongFrame(WebSocket websocket, WebSocketFrame frame) throws Exception {
        String msg = parseWebSocketFrame(frame);
        LogService.d(TAG, msg);
        networkService.keepAlive();
    }

    @Override
    public void onTextMessage(WebSocket websocket, String text) throws Exception {
        LogService.d(TAG, text);
        EventBus.getDefault().post(new EventMessage(text));
    }

    @Override
    public void onTextMessage(WebSocket websocket, byte[] data) throws Exception {
        LogService.d(TAG, "onTextMessage data");
    }

    @Override
    public void onBinaryMessage(WebSocket websocket, byte[] binary) throws Exception {
        LogService.d(TAG, "onBinaryMessage binary");
    }

    @Override
    public void onSendingFrame(WebSocket websocket, WebSocketFrame frame) throws Exception {
        String msg = parseWebSocketFrame(frame);
        //LogService.d(TAG, msg);
    }

    @Override
    public void onFrameSent(WebSocket websocket, WebSocketFrame frame) throws Exception {
        String msg = parseWebSocketFrame(frame);
        //LogService.d(TAG, msg);
    }

    @Override
    public void onFrameUnsent(WebSocket websocket, WebSocketFrame frame) throws Exception {
        String msg = parseWebSocketFrame(frame);
        LogService.d(TAG, msg);
    }

    @Override
    public void onThreadCreated(WebSocket websocket, ThreadType threadType, Thread thread) throws Exception {
        String msg = parseThreadFrame(threadType, thread);
        LogService.d(TAG, msg);
    }

    @Override
    public void onThreadStarted(WebSocket websocket, ThreadType threadType, Thread thread) throws Exception {
        String msg = parseThreadFrame(threadType, thread);
        LogService.d(TAG, msg);
    }

    @Override
    public void onThreadStopping(WebSocket websocket, ThreadType threadType, Thread thread) throws Exception {
        String msg = parseThreadFrame(threadType, thread);
        LogService.d(TAG, msg);
    }

    @Override
    public void onError(WebSocket websocket, WebSocketException cause) throws Exception {
        String log = String.format("%s\n, %s\n, %s\n",
                cause.getError().name(), cause.getLocalizedMessage(),
                cause.getMessage());
        LogService.i(TAG, log);

    }

    @Override
    public void onFrameError(WebSocket websocket, WebSocketException cause, WebSocketFrame frame) throws Exception {
        LogService.d(TAG, "onFrameError");
    }

    @Override
    public void onMessageError(WebSocket websocket, WebSocketException cause, List<WebSocketFrame> frames) throws Exception {
        LogService.d(TAG, "onMessageError");
    }

    @Override
    public void onMessageDecompressionError(WebSocket websocket, WebSocketException cause, byte[] compressed) throws Exception {
        LogService.d(TAG, "onMessageDecompressionError");
    }

    @Override
    public void onTextMessageError(WebSocket websocket, WebSocketException cause, byte[] data) throws Exception {
        LogService.d(TAG, "onTextMessageError");
    }

    @Override
    public void onSendError(WebSocket websocket, WebSocketException cause, WebSocketFrame frame) throws Exception {
        LogService.d(TAG, "onSendError");
    }

    @Override
    public void onUnexpectedError(WebSocket websocket, WebSocketException cause) throws Exception {
        LogService.d(TAG, "onUnexpectedError");
    }

    @Override
    public void handleCallbackError(WebSocket websocket, Throwable cause) throws Exception {
        LogService.d(TAG, "handleCallbackError " + cause.getMessage() + " " + cause.getLocalizedMessage());
    }

    @Override
    public void onSendingHandshake(WebSocket websocket, String requestLine, List<String[]> headers) throws Exception {
        LogService.d(TAG, "onSendingHandshake");
    }

    public static WebSocketManager getInstance() {
        return instance;
    }

    public WebSocket getWebSocket() {
        return webSocket;
    }

    public void init(String uri) {

        WebSocketFactory factory = new WebSocketFactory();

        try {
            LogService.d(TAG, "factory.createSocket");
            webSocket = factory.createSocket(uri, 2000);
            if (webSocket != null) {
                webSocket.addListener(this);
                webSocket.connectAsynchronously();
            }

        } catch (IOException e) {
            LogService.e(TAG, e.getMessage());
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }
    }

    private String parseWebSocketFrame(WebSocketFrame frame) {
        String msg = String.format("Code: %d, Close Reason: %s, PayLoadText: %s",
                frame.getCloseCode(),
                frame.getCloseReason(),
                frame.getPayloadText());
        return msg;
    }

    private String parseThreadFrame(ThreadType type, Thread thread) {
        String msg = String.format("ThreadType %s ", type.name());
        return msg;
    }

    protected void registerNetworkService(NetworkService networkService) {
        this.networkService = networkService;
    }

    protected void sendBinary(String message, byte[] data, boolean glTF) {

        String msg = "Websocket sendBinary: "+message+", "+data.length;

        if (webSocket != null) {
            if(glTF) {
                byte[] frame = DiskService.glTF(message, data);
                webSocket.sendBinary(frame);
            } else {
                webSocket.sendBinary(data);
            }

        }
    }

    private String stackTraceToString(Throwable throwable) {
        StringWriter errors = new StringWriter();
        throwable.printStackTrace(new PrintWriter(errors));
        return errors.toString();
    }
}
