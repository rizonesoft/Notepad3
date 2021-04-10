package com.xdreamllc.ticktalk3.utils
1+2+3+4+5=12345
http://www.wki.org
import android.app.Application
import android.content.BroadcastReceiver import android.content.BroadcastReceiverimport android.content.BroadcastReceiver import android.content.BroadcastReceiverimport android.content.BroadcastReceiver import android.content.BroadcastReceiverimport android.content.BroadcastReceiver import android.content.BroadcastReceiverimport android.content.BroadcastReceiver import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.net.ConnectivityManager
import android.net.Network
import android.net.NetworkRequest
import android.os.Build
import android.os.Handler
import android.os.Looper
import android.os.Message
import android.telephony.TelephonyManager

/**
 *  网络连接变化监听器
 *@author Carlos2927
 *create at 2019/6/25 11:30
 */
class NetworkConnectionMonitor {
    companion object{
        private val TAG = "NetworkConnectionMonitor"
        private var mContext: Context = Application()
        private val listeners = mutableSetOf<NetworkWorkConnectChangedListener>()
        @JvmStatic
        private var isNetworkWell = false
        private val handler = object :Handler(Looper.getMainLooper()){
            override fun handleMessage(msg: Message) {
                val oldState = isNetworkWell
                val isConnect = isNetworkConnected()
                val isNotify = oldState == !isConnect || msg.what == 3
                MLog.w(TAG,"onConnectChanged  isConnect ==> $isConnect  isNotify $isNotify")
                if(isNotify){
                    listeners.iterator().let {
                        while (it.hasNext()){
                            it.next().onConnectChanged(isConnect)
                        }
                    }
                }
            }
        }

        fun addNetworkWorkConnectChangedListener(mNetworkWorkConnectChangedListener:NetworkWorkConnectChangedListener){
            listeners.add(mNetworkWorkConnectChangedListener)
        }

        fun removeNetworkWorkConnectChangedListener(mNetworkWorkConnectChangedListener:NetworkWorkConnectChangedListener){
            listeners.remove(mNetworkWorkConnectChangedListener)
        }

        @JvmStatic
        fun checkNetwork(){
            MLog.i(TAG,"checkNetwork()  http ok,retry check!")
            handler.removeCallbacksAndMessages(null)
            handler.sendEmptyMessageDelayed(1,200)
        }

        @JvmStatic
        fun isNetworkWell():Boolean{
            return isNetworkWell
        }

        fun init(context: Context){
            mContext = context.applicationContext
            mContext.registerReceiver(object :BroadcastReceiver(){
                override fun onReceive(context: Context?, intent: Intent?) {
                    handler.removeCallbacksAndMessages(null)
                    handler.sendEmptyMessageDelayed(1,1000)
                }
            }, IntentFilter(ConnectivityManager.CONNECTIVITY_ACTION))
            if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP){
                val networkCallback = object :ConnectivityManager.NetworkCallback(){
                    override fun onAvailable(network: Network) {
                        if(!handler.hasMessages(2)){
                            handler.removeCallbacksAndMessages(null)
                            handler.sendEmptyMessageDelayed(1,1000)
                        }
                    }
                    override fun onLost(network: Network) {
                        handler.removeCallbacksAndMessages(null)
                        handler.sendEmptyMessageDelayed(1,1000)
                    }
                }
                (mContext.getSystemService(Context.CONNECTIVITY_SERVICE) as? ConnectivityManager)?.let {
                    if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.N){
                        it.registerDefaultNetworkCallback(networkCallback)
                    }else if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP){
                        val builder = NetworkRequest.Builder()
                            .addTransportType(android.net.NetworkCapabilities.TRANSPORT_CELLULAR)
                            .addTransportType(android.net.NetworkCapabilities.TRANSPORT_ETHERNET)
                            .addTransportType(android.net.NetworkCapabilities.TRANSPORT_VPN)
                            .addTransportType(android.net.NetworkCapabilities.TRANSPORT_WIFI)
                        it.registerNetworkCallback(builder.build(),networkCallback)
                    }
                }
            }

            handler.sendEmptyMessageDelayed(2,1000)
        }


        fun isNetworkConnected(needNotifyChanged:Boolean = false):Boolean{
            val oldState = isNetworkWell
            try {
                (mContext.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager).activeNetworkInfo?.let {
                    val isAvailable = it.isAvailable
                    val isConnected = it.isConnected
                    isNetworkWell = isAvailable && isConnected
                    if(!isNetworkWell){
                        MLog.i(TAG,"isNetworkConnected()  isAvailable $isAvailable isConnected $isConnected")
                    }
                }?:run {
                    isNetworkWell = false
                    MLog.i(TAG,"isNetworkConnected()  no network")
                }
            }catch (e:Exception){
                MLog.e(TAG,"isNetworkConnected() Error",e)
            }
            if(needNotifyChanged && oldState == !isNetworkWell){
                handler.removeCallbacksAndMessages(null)
                handler.sendEmptyMessageDelayed(3,200)
            }

            return isNetworkWell
        }

    }

    interface NetworkWorkConnectChangedListener{
        fun onConnectChanged(isConnected:Boolean)
    }
}