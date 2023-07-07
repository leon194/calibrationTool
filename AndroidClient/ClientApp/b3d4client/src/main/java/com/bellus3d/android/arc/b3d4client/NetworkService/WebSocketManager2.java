package com.bellus3d.android.arc.b3d4client.NetworkService;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import com.bellus3d.android.arc.b3d4client.AppStatusManager;
import com.bellus3d.android.arc.b3d4client.DiskService;
import com.bellus3d.android.arc.b3d4client.JsonModel.EventMessage;
import com.bellus3d.android.arc.b3d4client.LogService.LogService;

import org.greenrobot.eventbus.EventBus;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.sql.Timestamp;

import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;
import okhttp3.WebSocket;
import okhttp3.WebSocketListener;
import okio.ByteString;

import static com.bellus3d.android.arc.b3d4client.AppStatusManager.AppState.WEBSOCKET_CONNECTED;
import static com.bellus3d.android.arc.b3d4client.AppStatusManager.AppState.WEBSOCKET_DISCONNECTED;
import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.TAG;

public class WebSocketManager2 extends WebSocketListener {

    private static WebSocketManager2 instance = new WebSocketManager2();

    private WebSocket webSocket;
    private OkHttpClient okHttpClient;
    private NetworkService networkService;
    @SuppressLint("HandlerLeak")
    private Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case 1:
                    if (webSocket != null) {
                        String message = msg.getData().getString("message");
                        LogService.d(TAG, "message: " + message);
                        byte[] data = msg.getData().getByteArray("byteArray");
                        byte[] frame = DiskService.glTF(message, data);

                        try {
                            while (webSocket.queueSize() + frame.length >= 15000000) {
                                LogService.d(TAG, "queueSize: " + webSocket.queueSize());
                                Thread.sleep(10);
                            }
                            boolean isEnqueued = webSocket.send(ByteString.of(frame));
                            LogService.d(TAG, "isEnqueued: " + (isEnqueued? "true":"false")
                                    + " queueSize: " + webSocket.queueSize());
                            //measure transport speed
//                            long measureStart = System.currentTimeMillis();
//                            try {
//                                while (webSocket.queueSize() > 0) {
//                                    LogService.d(TAG, " transmitting... queueSize: " + webSocket.queueSize());
//                                    Thread.sleep(10);
//                                }
//                            } catch (InterruptedException e) {
//                                LogService.d(TAG, "InterruptedException: " + e.getCause());
//                            }
//                            long measureTotal = System.currentTimeMillis() - measureStart;
//                            LogService.d(TAG, "transmit done queueSize: " + webSocket.queueSize() + " timeConsuming: " + measureTotal + " ms" );
                        } catch (InterruptedException e) {
                            LogService.d(TAG, "InterruptedException: " + e.getCause());
                            e.printStackTrace();
                            LogService.logStackTrace(TAG, e.getStackTrace());
                        }
//                        if(true) {
//
//                        } else {
//                            byte[] data = msg.getData().getByteArray("byteArray");
//                            webSocket.send(ByteString.of(data));
//                        }
                    }
                    break;
            }
        }
    };

    private WebSocketManager2() {
        okHttpClient = new OkHttpClient();
    }

    public static WebSocketManager2 getInstance() {
        return instance;
    }

    public WebSocket getWebSocket() {
        return webSocket;
    }

    public void init(String uri) {
        Request request = new Request.Builder()
                .url(uri)
                .build();
        webSocket = okHttpClient.newWebSocket(request, this);
    }

    protected void registerNetworkService(NetworkService networkService) {
        this.networkService = networkService;
    }

    protected void sendBinary(String message, byte[] data, boolean glTF) {

        String msg = "Websocket sendBinary: "+message+", "+data.length;
        LogService.d(TAG, msg);
        Message handlerMessage = new Message();
        handlerMessage.what = 1;
        Bundle bundle = new Bundle();
        bundle.putString("message", message);
        bundle.putByteArray("byteArray", data);
        handlerMessage.setData(bundle);
        handler.sendMessage(handlerMessage);
    }

    @Override
    public void onOpen(@NotNull WebSocket webSocket, @NotNull Response response) {
        super.onOpen(webSocket, response);
        LogService.d(TAG, "");
        AppStatusManager.getInstance().setState(WEBSOCKET_CONNECTED);
    }

    @Override
    public void onMessage(@NotNull WebSocket webSocket, @NotNull String text) {
        super.onMessage(webSocket, text);
        LogService.d(TAG, text);
        EventBus.getDefault().post(new EventMessage(text));
    }

    @Override
    public void onMessage(@NotNull WebSocket webSocket, @NotNull ByteString bytes) {
        super.onMessage(webSocket, bytes);
        LogService.d(TAG, "");
    }

    @Override
    public void onClosing(@NotNull WebSocket webSocket, int code, @NotNull String reason) {
        super.onClosing(webSocket, code, reason);
        String message = String.format("code: %d, reason: %s", code, reason);
        LogService.d(TAG, message);
    }

    @Override
    public void onClosed(@NotNull WebSocket webSocket, int code, @NotNull String reason) {
        super.onClosed(webSocket, code, reason);
        String message = String.format("code: %d, reason: %s", code, reason);
        LogService.d(TAG, message);
        AppStatusManager.getInstance().setState(WEBSOCKET_DISCONNECTED);
    }

    @Override
    public void onFailure(@NotNull WebSocket webSocket, @NotNull Throwable t, @Nullable Response response) {
        super.onFailure(webSocket, t, response);
        String msg = t.getMessage();
        String localizedMsg =t.getLocalizedMessage();
        t.printStackTrace();
        LogService.e(TAG, "throwable msg: " + msg );
        LogService.e(TAG, "throwable localizedMsg: " + localizedMsg);
        LogService.e(TAG, "response code: " + (response != null ? response.code() : 0));
        LogService.e(TAG, "response message: " + (response != null ? response.message() : null));
        networkService.wsDisconnected(msg);
    }
}
