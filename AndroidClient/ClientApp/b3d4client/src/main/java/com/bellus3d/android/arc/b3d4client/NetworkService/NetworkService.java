package com.bellus3d.android.arc.b3d4client.NetworkService;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.support.annotation.RequiresApi;

import com.bellus3d.android.arc.b3d4client.AppStatusManager;
import com.bellus3d.android.arc.b3d4client.ConfigService.NetworkConfigService;
import com.bellus3d.android.arc.b3d4client.GlobalResourceService;
import com.bellus3d.android.arc.b3d4client.JsonModel.*;
import com.bellus3d.android.arc.b3d4client.JsonModel.innerjsonmodel.*;
import com.bellus3d.android.arc.b3d4client.LogService.LogService;
import com.bellus3d.android.arc.b3d4client.TimeStampService;
import com.bellus3d.android.arc.b3d4client.MessageService.NetWorkMessage;
import com.google.gson.Gson;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;
import org.json.JSONObject;

import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.TAG;
import static com.bellus3d.android.arc.b3d4client.MessageService.NetWorkMessage.ErrorRequest.*;
import static com.bellus3d.android.arc.b3d4client.MessageService.NetWorkMessage.HostCommands.*;
import static com.bellus3d.android.arc.b3d4client.AppStatusManager.AppState.*;

import java.util.List;
import java.util.Timer;
import java.util.TimerTask;
import java.util.regex.Pattern;

public class NetworkService {

    private boolean useWIFISolution = false;

    // Version info
    public String osVersion;
    public int sdkVersion;
    public int buildNumber;
    public String versionName;

    // Get unique deviceID
    public String deviceMfg;
    public String deviceModel;
    public String deviceID;
    public String adbID;


    private static Context _context;

    public B3DNetworkingListener _listener;

    private WifiManager _wifi;
    private ScanResult _hs = null;
    private String _deviceIP;
    private String _rssi = "";
    private String _hostaddr = null;
    private int _webSocketPort=-1;
    private String _webSocketURL;

    private long lastEvent = 0;

    private Timer _webSocktimer;
    private TimerTask _webSocktask;

    private Timer wifiScanTimer = new Timer();
    private TimerTask wifiScanTimerTask = new TimerTask() {
        @Override
        public void run() {
            if (_wifi != null) {
                LogService.d(TAG, "start wifi scan");
                _wifi.startScan();
            }
        }
    };

    private Gson gson;
    private DeviceConfig _deviceConfig;

    private NetWorkMessage.HostCommands currentCommand;

    public NetworkService(final Context context){
        _context = context;
        keepAlive();

        // Version info
        osVersion = GlobalResourceService.getOSVersion();
        sdkVersion = GlobalResourceService.getSDKVersion();
        buildNumber = GlobalResourceService.getAppBuildNumber();
        versionName = GlobalResourceService.getAppVersionName();

        // Get unique deviceID
        deviceMfg = GlobalResourceService.getDeviceManufacturer();
        deviceModel = GlobalResourceService.getDeviceModel();
        deviceID = GlobalResourceService.getDeviceId();
        adbID = GlobalResourceService.getAdbId();

        EventBus.getDefault().register(this);
        gson = new Gson();
    }

    @Override
    protected void finalize() throws Throwable {
        EventBus.getDefault().unregister(this);
        super.finalize();
    }

    public void setDeviceConfig(DeviceConfig deviceConfig) {
        _deviceConfig = deviceConfig;
        _webSocketPort = deviceConfig.getWsPort();
        _hostaddr = deviceConfig.getHostAddress();
    }

    public String getWebSocketURL() {
        return _webSocketURL;
    }

    public boolean initializeNetwork() {
        LogService.d(TAG, "initializeNetwork");

        if(useWIFISolution)
            _wifi = (WifiManager) _context.getApplicationContext().getSystemService(Context.WIFI_SERVICE);

        _webSocketURL = GlobalResourceService.WEB_SOCKET_PROTO + "://"  + _hostaddr + ":" + _webSocketPort;
        WebSocketManager2.getInstance().registerNetworkService(this);

        if(!useWIFISolution) checkUSBorWIFI();

        return true;
    }

    private void checkUSBorWIFI() {
        if (useWIFISolution && !_deviceConfig.getLocalConnection()) {
            // The following code needs to be refactored/modularized
            // Search for B3D4 _wifi hotspots
            LogService.i(TAG, "Searching for B3D4 hosts");

            //According to Android Developer guide, each foreground app can scan four times
            // in a 2-minute period.
            wifiScanTimer.schedule(wifiScanTimerTask, 0, 30000);
            AppStatusManager.getInstance().setState(WIFI_SCANNING);

        } else {

            // Start Websocket if mWsPort defined in config file
            if (_webSocketPort > 0) {
                LogService.i(TAG, "Using designated websocket port: "+_webSocketPort);
                connectToWebSocket(_hostaddr, _webSocketPort);
                startWebSocketMonitor();
            }
        }
    }

    public void keepAlive() {
        lastEvent = System.currentTimeMillis();
    }

    public long sinceLastEvent() {
        return System.currentTimeMillis() - lastEvent;
    }

    private boolean idleNetwork(long threshold) {
        return threshold < sinceLastEvent();
    }

    private void startWebSocketMonitor() {
        _webSocktimer = new Timer();
        _webSocktask = new TimerTask() {
            @RequiresApi(api = Build.VERSION_CODES.N)
            @Override
            public void run() {
                if(!AppStatusManager.getInstance().passedState(WEBSOCKET_CONNECTED) && _webSocketPort > 0) {
                    LogService.d(TAG, "reconnecte to designated websocket port: "
                            + _webSocketPort + " current state : " + AppStatusManager.getInstance().getState());
                    if(useWIFISolution) {
                        if (!_wifi.isWifiEnabled()) {
                            //enable wifi, make sure wifi always enable
                            LogService.d(TAG, "Wifi turn on");
                            _wifi.setWifiEnabled(true);
                        }
                    }
                    connectToWebSocket(_hostaddr, _webSocketPort);
                }
            }
        };

        _webSocktimer.schedule(_webSocktask, 0, 2000);
    }

    public void connectToWebSocket(String ip, int port){
        if (AppStatusManager.getInstance().passedState(WEBSOCKET_READY_TO_CONNECT)) return;
        LogService.d(TAG, "current state: " + AppStatusManager.getInstance().getState());
        AppStatusManager.getInstance().setState(WEBSOCKET_READY_TO_CONNECT);

        LogService.i(TAG, "Attempting to connect to host via WebSocket: "+_webSocketURL);
        WebSocketManager2.getInstance().init(_webSocketURL);
    }

    public void registerNetWorkingListener (B3DNetworkingListener listener) {
        _listener = listener;
    }

    public void sendMessage(String message){
        try {
            LogService.d(TAG, message);
            WebSocketManager2.getInstance().getWebSocket().send(message);
        } catch (Exception e) {
            LogService.e(TAG,"webSocket sendMessage error " + e.toString());
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }
    }

    public void sendBinaryToHost(String msg, byte[] byteData, boolean glTF) {
        try {
            WebSocketManager2.getInstance().sendBinary(msg, byteData, glTF);
            LogService.d(TAG,"webSocket sendBinary done ");
        } catch (Exception e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG,e.getStackTrace());
        }
    }

    private int configureWifi(ScanResult hs) {
        LogService.d(TAG, AppStatusManager.getInstance().getState().name());
        if (AppStatusManager.getInstance().passedState(WEBSOCKET_CONNECTED)) return -1;

        String secMode = "PSK";
        String ssid = "";
        if (hs != null) {
            secMode = NetworkConfigService.getScanResultSecurity(hs);
            ssid = hs.SSID;
        } else {
            ssid = _deviceConfig.getHotspot();
        }
        if (ssid == null || ssid.isEmpty()) {
            LogService.e(TAG, "missing ssid");
            return -1;
        }

        // Look up cached wifi configuration
        int wifiId = -1;
        List<WifiConfiguration> networks = _wifi.getConfiguredNetworks();
        for(WifiConfiguration config : networks) {
            LogService.d(TAG, "configured network: "+config.SSID);
            if (config.SSID.equals("\""+ssid+"\"")) {
                wifiId = config.networkId;
                LogService.d(TAG, "picked configured network: "+config.SSID + "  id is: "+wifiId);
            } else {
                _wifi.removeNetwork(config.networkId);
            }
        }

        //add a new wifi configuration
        if (wifiId < 0) {
            String _password = _deviceConfig.getPassword();
            WifiConfiguration config = NetworkConfigService.createAPConfiguration(ssid, _password, secMode);
            wifiId = _wifi.addNetwork(config);
            if (wifiId < 0) {
                String msg = "Unable to add "+ssid+" to AP Configuration";
                LogService.e(TAG, msg);
                return -1;
            }
        }
        LogService.d(TAG, "Network configured: "+wifiId);
        AppStatusManager.getInstance().setState(WIFI_CONFIGURED);
        return wifiId;
    }

    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN)
    private boolean connectToHotspot(int wifiID) {

        if (!AppStatusManager.getInstance().passedState(WIFI_CONFIGURED)) {
            LogService.e(TAG, "wifi not configured yet");
            return false;
        }

        String msg;
        if (wifiID < 0) {
            if (configureWifi(_hs) < 0) {
                msg = "Unable to configure wifi for ";
                LogService.e(TAG, msg);
                _listener.onUpdateUI("WifiP2PStatus",msg);
                return false;
            }
        }

        if (!_wifi.enableNetwork(wifiID, true)) {
            msg = "Unable to connect to ";
            LogService.e(TAG, msg);
            _listener.onUpdateUI("WifiP2PStatus",msg);
            return false;
        }
        boolean ok = _wifi.reconnect();
        if (!ok) {
            LogService.e(TAG, "Hotspot connect failed");
            return false;
        }
        LogService.d(TAG, "Hotspot connected; resolving network");

        return true;
    }


    private String getIPAddress() {
        String testIP = NetworkConfigService.getIPAddress(true);
        return testIP;
    }

    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN)
    private void processWifiScanResults(){
        LogService.d(TAG, AppStatusManager.getInstance().getState().name());
        if (AppStatusManager.getInstance().passedState(WIFI_CONFIGURED)) return;

        String _hotspot = _deviceConfig.getHotspot();
        if(_hotspot == null){
            LogService.e(TAG, "Hotspot is not set");
            return;
        }
        LogService.d(TAG, "Hotspot : "+_hotspot);

        if(!AppStatusManager.getInstance().passedState(WIFI_CONFIGURED)) {

            List<ScanResult> scanResults = null;
            // use hot spot name to filter out all the other wifi hot spot
            String configSSID = _deviceConfig.getHotspot();
            Pattern hsRegexFromConfigFile = Pattern.compile(_deviceConfig.getHotspot());
            scanResults = _wifi.getScanResults();
            if (scanResults == null || scanResults.size() == 0) {
                LogService.e(TAG, "scan result empty");
                return;
            }

            for (ScanResult scanResult : scanResults) {
                LogService.d(TAG, "scan result: " + scanResult.SSID);
                if (scanResult.SSID.equals(configSSID)) {
                    LogService.d(TAG, "Found SSID: " + scanResult.SSID + ", " + scanResult.BSSID + ", " + scanResult.level);
                    if (scanResult.SSID.contains(_hotspot)) {
                        _hs = scanResult;
                        _rssi = Integer.valueOf(scanResult.level).toString();
                        break;
                    }
                }
            }
        }

        if(_hs == null){
            LogService.e(TAG, "Cannot find HotSpot");
            return;
        }

        AppStatusManager.getInstance().setState(WIFI_SCANNED);
        int wifiId = configureWifi(_hs);

        connectToHotspot(wifiId);
    }

    // We need this so that the camera knows to try to reconnect
    public void wsDisconnected(String message) {
        if (AppStatusManager.getInstance().getState().equals(CAMERA_RECORDING)) {
            _listener.onErrorRequest(REQUEST_STOP_STREAM);
        }
        AppStatusManager.getInstance().setState(WEBSOCKET_DISCONNECTED);

        LogService.i(TAG, "Websocket disconnected: " + message);
    }

    private BroadcastReceiver wifiScanReceiver = new BroadcastReceiver() {
        @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN)
        @Override
        public void onReceive(Context ctx, Intent intent) {
            boolean success = intent.getBooleanExtra(WifiManager.EXTRA_RESULTS_UPDATED, false);
            if (success) {
                LogService.d(TAG, "Receiving new wifi scan results. current state is: "
                        + AppStatusManager.getInstance().getState().name());
            } else {
                LogService.d(TAG, "did not received new wifi scan results. current state is: "
                        + AppStatusManager.getInstance().getState().name());
            }
            if (!_deviceConfig.getLocalConnection()) {
                processWifiScanResults();
            }
        }
    };

    private BroadcastReceiver wifiAPReceiver = new BroadcastReceiver() {

        private final String WIFI_AP_STATE_CHANGED_ACTION = "android.net.wifi.WIFI_AP_STATE_CHANGED";

        @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN)
        @Override
        public void onReceive(Context context, Intent intent) {
            boolean isHsConnected = false;
            ConnectivityManager conMan = (ConnectivityManager)context.getSystemService(Context.CONNECTIVITY_SERVICE);
            NetworkInfo netInfo = conMan.getActiveNetworkInfo();
            if (netInfo != null) {
                isHsConnected = netInfo.isConnectedOrConnecting();
            }

            String action = intent.getAction();
            switch(action) {
                case WifiManager.NETWORK_IDS_CHANGED_ACTION://"android.net.wifi.NETWORK_IDS_CHANGED":
                {
                    LogService.d(TAG, "NETWORK_IDS_CHANGED_ACTION");
                    break;
                }

                // Changed wifi state
                case WifiManager.WIFI_STATE_CHANGED_ACTION://"android.net.wifi.WIFI_STATE_CHANGED"
                {
                    LogService.d(TAG, "wifiAPReceiver WIFI_STATE_CHANGED");
                    int state = intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE, 0);
                    switch(state)
                    {
                        case WifiManager.WIFI_STATE_DISABLED:
                        {
                            LogService.d(TAG, "WiFi Off");
                            AppStatusManager.getInstance().setState(WIFI_OFF);
                            _wifi.setWifiEnabled(true);
                            break;
                        }
                        case WifiManager.WIFI_STATE_DISABLING:
                        {
                            LogService.d(TAG, "wifiAPReceiver WIFI_STATE_DISABLING");
                            break;
                        }
                        case WifiManager.WIFI_STATE_ENABLED:
                        {
                            LogService.d(TAG, "WiFi On");
                            if (!AppStatusManager.getInstance().passedState(WIFI_ON)) {
                                AppStatusManager.getInstance().setState(WIFI_ON);
                            }
                            checkUSBorWIFI();
                            break;
                        }
                        case WifiManager.WIFI_STATE_ENABLING: {
                            LogService.d(TAG, "WIFI_STATE_ENABLING");
                            break;
                        }
                        case WifiManager.WIFI_STATE_UNKNOWN:
                        {
                            LogService.d(TAG, "WIFI_STATE_UNKNOWN");
                            break;
                        }
                        default:
                        {
                            LogService.d(TAG, "WIFI_STATE unknown : "+state);
                            break;
                        }
                    }
                    break;
                }

                // Connectivity changed
                case WifiManager.NETWORK_STATE_CHANGED_ACTION://"android.net.wifi.STATE_CHANGE"
                {
                    NetworkInfo info = intent.getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);
                    NetworkInfo.State status = info.getState();
                    NetworkInfo.DetailedState state = info.getDetailedState();
                    LogService.d(TAG, "NETWORK_STATE_CHANGED_ACTION: "+status.toString()+":"+state.toString());
                    if (status == NetworkInfo.State.DISCONNECTED || status == NetworkInfo.State.DISCONNECTING) {
                        LogService.d(TAG, "wifiAPReceiver STATE_CHANGE: "+status.toString()+":"+state.toString());
                        if (!_deviceConfig.getLocalConnection()) {
                            if (_wifi.isWifiEnabled()) {
                                AppStatusManager.getInstance().setState(WIFI_DISCONNECTED);
                            } else {
                                AppStatusManager.getInstance().setState(WIFI_OFF);
                            }
                        }
                    } else if (status == NetworkInfo.State.CONNECTED) {
                        if (AppStatusManager.getInstance()
                                .passedState(WEBSOCKET_READY_TO_CONNECT)) break;

                        LogService.d(TAG, "stop continue scan");
                        _deviceIP = getIPAddress();
                        LogService.d(TAG, "My IP address:" + _deviceIP);
                        if (!_deviceConfig.getLocalConnection()) {
                            connectToWebSocket(null, 0);
                        }
                    }

                    break;
                }

                case  WIFI_AP_STATE_CHANGED_ACTION://"android.net.wifi.WIFI_AP_STATE_CHANGED"
                {
                    LogService.d(TAG, "WIFI_AP_STATE_CHANGED");
                    break;
                }
                case WifiManager.RSSI_CHANGED_ACTION: //"android.net.wifi.RSSI_CHANGED"
                {
                    LogService.d(TAG, "RSSI_CHANGED");
                    int rssi = intent.getIntExtra(WifiManager.EXTRA_NEW_RSSI, 0);
                    break;
                }
                default:
                {
                    LogService.e(TAG, "unknown action : "+action);
                    break;
                }
            }
        }
    };

    /***
     *  register some broadcastReceiver about wifi
     *  like Network state change, Wifi state and wifi scan result
     */
    public void registerWifiReceiver() {
        if(!useWIFISolution) return;
        //register wifi broadcast action
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
        intentFilter.addAction(WifiManager.NETWORK_IDS_CHANGED_ACTION);
        intentFilter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
        intentFilter.addAction(WifiManager.RSSI_CHANGED_ACTION);
        _context.registerReceiver(wifiAPReceiver, intentFilter);

        intentFilter.addAction(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION);
        _context.registerReceiver(wifiScanReceiver, intentFilter);
    }

    /***
     *  unregister some broadcastReceiver about wifi
     *  like Network state change, Wifi state and wifi scan result
     */
    public void unRegisterWifiReceiver() {
        if(!useWIFISolution) return;
        _context.unregisterReceiver(wifiScanReceiver);
        _context.unregisterReceiver(wifiAPReceiver);
    }


    @Subscribe(threadMode = ThreadMode.ASYNC)
    public void onMessageEvent(EventMessage eventMessage) {
        String message = eventMessage.getMsg();
        LogService.d(TAG, "received host message: " + message);
        keepAlive();

        NetWorkMessage netmsg = new NetWorkMessage();

        JSONObject json = NetWorkMessage.buildNetMessage(netmsg, message);
        if( json == null ) return;

        Common common = gson.fromJson(message, Common.class);

        String sendMessageStr;
        if (common.getRequest() != null) { // request
            if (common.getRequest().equals(NetWorkMessage.RequestType.HELLO.getName())) {
                String requestID = TimeStampService.getTimestamp(
                        null, GlobalResourceService.TIME_DIFFERENCE);

                HelloRequest helloRequest = new HelloRequest();
                helloRequest.setRequestId(requestID);
                helloRequest.setRequest(common.getRequest());
                helloRequest.setAdbId(GlobalResourceService.getAdbId());
                helloRequest.setDeviceId(GlobalResourceService.getDeviceId());
                helloRequest.setDeviceModel(GlobalResourceService.getDeviceModel());
                helloRequest.setManufacturer(GlobalResourceService.getDeviceManufacturer());
                helloRequest.setVersion(versionName);
                helloRequest.setBuildNo(String.valueOf(buildNumber));
                helloRequest.setClientIp(_deviceIP);
                helloRequest.setRssi(_rssi);

                sendMessageStr = gson.toJson(helloRequest);
                WebSocketManager2.getInstance().getWebSocket().send(sendMessageStr);
                AppStatusManager.getInstance().setState(WEBSOCKET_CONNECTED);
            } else if (common.getRequest().equals(
                    NetWorkMessage.RequestType.BUFFER_CAPTURE.getName())) {
                LogService.d(TAG, "" +
                        "Received request to capture burst to buffer request_id: " +
                        common.getRequestId());

                currentCommand = netmsg.cmds = CMD_BUFFER_CAPTURE;
                _listener.onMessage(netmsg);
            } else if (common.getRequest().equals(
                    NetWorkMessage.RequestType.BUFFER_FETCH.getName())) {
                LogService.d(TAG, "" +
                        "Received request to fetch buffer request_id: " +
                        common.getRequestId());

                currentCommand = netmsg.cmds = CMD_BUFFER_FETCH;
                _listener.onMessage(netmsg);
            } else if (common.getRequest().equals(
                    NetWorkMessage.RequestType.STREAM_CAPTURE.getName())) {
                LogService.d(TAG, "" +
                        "Received request to capture raw buffer request_id: " +
                        common.getRequestId());
                netmsg.cmds = CMD_STREAM_CAPTURE;
                _listener.onMessage(netmsg);

            } else if (common.getRequest().equals(
                    NetWorkMessage.RequestType.BUFFER_COLOR.getName())) {
                LogService.d(TAG, "Received request to fetch raw color buffer request_id : " +
                        common.getRequestId());

                currentCommand = netmsg.cmds = CMD_BUFFER_COLOR;
                _listener.onMessage(netmsg);

            } else if (common.getRequest().equals(
                    NetWorkMessage.RequestType.BUFFER_SNAPSHOT.getName())) {
                LogService.d(TAG, "Received request to fetch preview buffer request_id : " +
                        common.getRequestId());

                currentCommand = netmsg.cmds = CMD_BUFFER_SNAPSHOT;
                _listener.onMessage(netmsg);
            } else if (common.getRequest().equals(NetWorkMessage.RequestType.BUFFER_SNAPSHOT_RECEIVE.getName())) {
                LogService.d(TAG, "Received request to upload snapshot buffer request_id : " +
                        common.getRequestId());
                currentCommand = netmsg.cmds = CMD_BUFFER_SNAPSHOT_RECEIVE;
                _listener.onMessage(netmsg);
            } else if (common.getRequest().equals(NetWorkMessage.RequestType.STREAM_DEPTH.getName())) {
                LogService.d(TAG, "Received request to stream depth  request_id : " +
                        common.getRequestId());
                currentCommand = netmsg.cmds = CMD_STREAM_DEPTH;
                _listener.onMessage(netmsg);
            } else if (common.getRequest().equals(
                    NetWorkMessage.RequestType.BUFFER_CANCEL.getName())) {
                LogService.d(TAG, "Received request to  buffer cancel request_id : " +
                        common.getRequestId());

                currentCommand = netmsg.cmds = CMD_BUFFER_CANCEL;
                _listener.onMessage(netmsg);

            } else if (common.getRequest().equals(
                    NetWorkMessage.RequestType.STREAM_START.getName())) {
                LogService.d(TAG, "Received request to start stream: " +
                        common.getRequestId());

                currentCommand = netmsg.cmds = CMD_START_STREAM;
                _listener.onMessage(netmsg);

            } else if (common.getRequest().equals(
                    NetWorkMessage.RequestType.STREAM_STOP.getName())) {
                LogService.d(TAG, "Received request to end stream: " +
                        common.getRequestId());

                currentCommand = netmsg.cmds = CMD_STOP_STREAM;
                _listener.onMessage(netmsg);

            } else if (common.getRequest().equals(
                    NetWorkMessage.RequestType.BUFFER_MERGE.getName())) {
                LogService.d(TAG, "Received request to merge buffer request_id: "+
                        common.getRequestId());

                currentCommand = netmsg.cmds = CMD_BUFFER_MERGE;
                _listener.onMessage(netmsg);

            } else if (common.getRequest().equals(
                    NetWorkMessage.RequestType.RECALIBRATION.getName())) {
                LogService.d(TAG, "Received request to reCalibration request_id: "+
                        common.getRequestId());

                currentCommand = netmsg.cmds = CMD_RECALIBRATION;
                _listener.onMessage(netmsg);

            } else if (common.getRequest().equals(
                    NetWorkMessage.RequestType.BUFFER_RELEASE.getName())) {
                LogService.d(TAG, "Received request to release buffer");

                currentCommand = netmsg.cmds = CMD_BUFFER_RELEASE;
                _listener.onMessage(netmsg);

            } else if (common.getRequest().equals(
                    NetWorkMessage.RequestType.CANCEL_PROCESS.getName())) {
                LogService.d(TAG, "Received request to cancel process");

                currentCommand = netmsg.cmds = CMD_CANCEL_PROCESS;
                _listener.onMessage(netmsg);

            } else if (common.getRequest().equals(NetWorkMessage.RequestType.CAMERA_CONFIGURATION.getName())) { //deprecate?

                LogService.d(TAG, "Received request to camera configuration");
                currentCommand = netmsg.cmds = CMD_CAMERA_CONFIGURATION;
                _listener.onMessage(netmsg);

            } else if (common.getRequest().equals(
                    NetWorkMessage.RequestType.LAYOUT_POSITION.getName())) { //deprecate?

                LogService.d(TAG, "Received devices camera position");
                currentCommand = netmsg.cmds = CMD_LAYOUT_POSITION;
                _listener.onMessage(netmsg);

            } else if (common.getRequest().equals(
                    NetWorkMessage.RequestType.DISCONNECT.getName())) {

                LogService.i(TAG, "Received Host disconnect notification");
                wsDisconnected("Host sent disconnect notification");

            } else if (common.getRequest().equals(
                    NetWorkMessage.RequestType.PING.getName())) { //deprcate?
                LogService.d(TAG, "Received ping");

                PongRequest pongRequest = new PongRequest();
                pongRequest.setRequest(NetWorkMessage.RequestType.PONG.getName());
                pongRequest.setRequestId(netmsg.request_id);

                if (netmsg.ping_type != null && netmsg.ping_type.equals("keepalive")) {

                    LogService.d(TAG, "Received ping and is keepalive");
                    pongRequest.setType(NetWorkMessage.RequestType.KEEP_ALIVE.getName());

                } else {
                    LogService.d(TAG, "Received ping and not keepalive");
                    pongRequest.setRssi(_rssi);
                }
                sendMessageStr = gson.toJson(pongRequest);
                LogService.d(TAG, sendMessageStr);
                sendMessage(sendMessageStr);
            }
        } else if(common.getResponseTo() != null) { //response
            String msg = "onMessage response_to unsupported: "+ common.getResponseTo();

            if (common.getResponseTo().equals(NetWorkMessage.ResponseTo.CONNECT.getName())) {
                msg = "Host confirms WebSocket connected";

                String host = common.getHostname();
                String address = common.getHostAddress();

                LogService.d(TAG,
                        "ws received connect confirmation : "+ host + " " + address);

            } else if (common.getResponseTo().equals(
                    NetWorkMessage.ResponseTo.UPLOAD.getName())) { //upload
                msg = "Host confirmed upload";
            } else if (common.getResponseTo().equals(NetWorkMessage.RequestType.PING.getName())) {
                msg = "Host responded with pong";
            } else if (common.getResponseTo().equals(NetWorkMessage.RequestType.BUFFER_CAPTURE.getName())) {
                msg = "Host confirmed buffer capture";
            }
            LogService.d(TAG, msg);
        } else {
            LogService.d(TAG, "other message");
        }
    }
}
