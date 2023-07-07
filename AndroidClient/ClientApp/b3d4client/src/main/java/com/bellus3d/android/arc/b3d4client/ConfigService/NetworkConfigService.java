package com.bellus3d.android.arc.b3d4client.ConfigService;

import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;

import com.bellus3d.android.arc.b3d4client.GlobalResourceService;
import com.bellus3d.android.arc.b3d4client.LogService.LogService;

import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.Collections;
import java.util.List;
import java.util.regex.Matcher;

import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.TAG;

public class NetworkConfigService {

    public static WifiConfiguration createAPConfiguration(String networkSSID, String networkPasskey, String securityMode) {
        WifiConfiguration wifiConfiguration = new WifiConfiguration();
        wifiConfiguration.SSID = "\"" + networkSSID + "\"";
        if (securityMode.equalsIgnoreCase("OPEN")) {
            wifiConfiguration.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
        } else if (securityMode.equalsIgnoreCase("WEP")) {
            wifiConfiguration.wepKeys[0] = "\"" + networkPasskey + "\"";
            wifiConfiguration.wepTxKeyIndex = 0;
            wifiConfiguration.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
            wifiConfiguration.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP40);
        } else if (securityMode.equalsIgnoreCase("PSK")) {
            wifiConfiguration.preSharedKey = "\"" + networkPasskey + "\"";
            wifiConfiguration.hiddenSSID = true;
            wifiConfiguration.status = WifiConfiguration.Status.ENABLED;
            wifiConfiguration.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
            wifiConfiguration.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
            wifiConfiguration.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_PSK);
            wifiConfiguration.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);
            wifiConfiguration.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
            wifiConfiguration.allowedProtocols.set(WifiConfiguration.Protocol.RSN);
            wifiConfiguration.allowedProtocols.set(WifiConfiguration.Protocol.WPA);
        } else {
            LogService.e(TAG, "Unsupported security mode: " + securityMode);
            return null;
        }
        return wifiConfiguration;
    }

    public static String getIPAddress(boolean useIPv4) {
        if (!useIPv4) {
            LogService.w(TAG, "getIPAddress IPv6 not currently supported");
            return "";
        }

        try {
            List<NetworkInterface> interfaces = Collections.list(NetworkInterface.getNetworkInterfaces());
            for (NetworkInterface intf : interfaces) {
                List<InetAddress> addrs = Collections.list(intf.getInetAddresses());
                for (InetAddress addr : addrs) {
                    if (!addr.isLoopbackAddress()) {
                        String sAddr = addr.getHostAddress();
                        LogService.d(TAG, "getIPAddress: "+sAddr);
                        //boolean isIPv4 = InetAddressUtils.isIPv4Address(sAddr);
                        boolean isIPv4 = sAddr.indexOf(':')<0;

                        if (isIPv4) {
                            //LogService.i(TAG, "getIPAddress IPv4: "+sAddr);
                            Matcher ipMatch = GlobalResourceService.ipRegex.matcher(sAddr);
                            if (ipMatch.find()) {
                                return sAddr;
                            }
                        }
                    }
                }
            }
        } catch (Exception ignored) { } // for now eat exceptions
        return "";
    }

    public static String getScanResultSecurity(ScanResult scanResult) {
        final String cap = scanResult.capabilities;
        final String[] securityModes = {"WEP", "PSK", "EAP"};
        for (int i = securityModes.length - 1; i >= 0; i--) {
            if (cap.contains(securityModes[i])) {
                return securityModes[i];
            }
        }
        return "OPEN";
    }
}
