package com.bellus3d.android.arc.b3d4client.NetworkService;

import com.bellus3d.android.arc.b3d4client.MessageService.NetWorkMessage;

/**
 * For notice B3DCameraService commands
 */

public interface B3DNetworkingListener {

    // notify registrants
    void onMessage(NetWorkMessage msg);

    void onErrorRequest(NetWorkMessage.ErrorRequest request);

    void onUpdateUI(String view, String message);

    void onUIEvent(String event, int value);
}
