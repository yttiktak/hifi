//
//  InterfaceActivity.java
//  android/app/src/main/java
//
//  Created by Stephen Birarda on 1/26/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

package io.highfidelity.hifiinterface;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.graphics.Point;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Vibrator;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.SlidingDrawer;

import org.qtproject.qt5.android.QtNative;
import org.qtproject.qt5.android.QtLayout;
import org.qtproject.qt5.android.QtSurface;
import org.qtproject.qt5.android.bindings.QtActivity;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import io.highfidelity.hifiinterface.fragment.WebViewFragment;
import io.highfidelity.hifiinterface.receiver.HeadsetStateReceiver;

/*import com.google.vr.cardboard.DisplaySynchronizer;
import com.google.vr.cardboard.DisplayUtils;
import com.google.vr.ndk.base.GvrApi;*/

public class InterfaceActivity extends QtActivity implements WebViewFragment.OnWebViewInteractionListener {

    public static final String DOMAIN_URL = "url";
    public static final String EXTRA_GOTO_USERNAME = "gotousername";
    private static final String TAG = "Interface";
    public static final String EXTRA_ARGS = "args";
    private static final int WEB_DRAWER_RIGHT_MARGIN = 262;
    private static final int WEB_DRAWER_BOTTOM_MARGIN = 150;
    private static final int NORMAL_DPI = 160;

    private Vibrator mVibrator;
    private HeadsetStateReceiver headsetStateReceiver;

    //public static native void handleHifiURL(String hifiURLString);
    private native void nativeOnCreate(AssetManager assetManager);
    private native void nativeOnDestroy();
    private native void nativeGotoUrl(String url);
    private native void nativeGoToUser(String username);
    private native void nativeBeforeEnterBackground();
    private native void nativeEnterBackground();
    private native void nativeEnterForeground();
    private native long nativeOnExitVr();
    private native void nativeInitAfterAppLoaded();

    private AssetManager assetManager;

    private static boolean inVrMode;

    private boolean nativeEnterBackgroundCallEnqueued = false;
    private SlidingDrawer mWebSlidingDrawer;
    private boolean mStartInDomain;
    private boolean isLoading;
//    private GvrApi gvrApi;
    // Opaque native pointer to the Application C++ object.
    // This object is owned by the InterfaceActivity instance and passed to the native methods.
    //private long nativeGvrApi;
    
    public void enterVr() {
        //Log.d("[VR]", "Entering Vr mode (java)");
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
        inVrMode = true;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        isLoading = true;
        Intent intent = getIntent();
        if (intent.hasExtra(DOMAIN_URL) && !TextUtils.isEmpty(intent.getStringExtra(DOMAIN_URL))) {
            intent.putExtra("applicationArguments", "--url " + intent.getStringExtra(DOMAIN_URL));
        } else if (intent.hasExtra(EXTRA_ARGS)) {
            String args = intent.getStringExtra(EXTRA_ARGS);
            if (!TextUtils.isEmpty(args)) {
                mStartInDomain = true;
                intent.putExtra("applicationArguments", args);
            }
        }
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        if (intent.getAction() == Intent.ACTION_VIEW) {
            Uri data = intent.getData();

//            if (data.getScheme().equals("hifi")) {
//                handleHifiURL(data.toString());
//            }
        }
        
/*        DisplaySynchronizer displaySynchronizer = new DisplaySynchronizer(this, DisplayUtils.getDefaultDisplay(this));
        gvrApi = new GvrApi(this, displaySynchronizer);
        */
//        Log.d("GVR", "gvrApi.toString(): " + gvrApi.toString());

        assetManager = getResources().getAssets();

        //nativeGvrApi =
            nativeOnCreate(assetManager /*, gvrApi.getNativeGvrContext()*/);

        final View rootView = getWindow().getDecorView().findViewById(android.R.id.content);

        // This is a workaround to hide the menu bar when the virtual keyboard is shown from Qt
        rootView.getViewTreeObserver().addOnGlobalLayoutListener(() -> {
            if (getActionBar() != null && getActionBar().isShowing()) {
                getActionBar().hide();
            }
        });
        Intent splashIntent = new Intent(this, SplashActivity.class);
        splashIntent.putExtra(SplashActivity.EXTRA_START_IN_DOMAIN, mStartInDomain);
        startActivity(splashIntent);
        
        mVibrator = (Vibrator) this.getSystemService(VIBRATOR_SERVICE);
        headsetStateReceiver = new HeadsetStateReceiver();
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (isLoading) {
            nativeEnterBackgroundCallEnqueued = true;
        } else {
            nativeEnterBackground();
        }
        unregisterReceiver(headsetStateReceiver);
        //gvrApi.pauseTracking();
    }

    @Override
    protected void onStart() {
        super.onStart();
        nativeEnterBackgroundCallEnqueued = false;
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
    }

    @Override
    protected void onStop() {
        super.onStop();

    }

    @Override
    protected void onResume() {
        super.onResume();
        nativeEnterForeground();
        surfacesWorkaround();
        registerReceiver(headsetStateReceiver, new IntentFilter(Intent.ACTION_HEADSET_PLUG));
        //gvrApi.resumeTracking();
    }

    @Override
    protected void onDestroy() {
        nativeOnDestroy();
        /*
        cduarte https://highfidelity.manuscript.com/f/cases/16712/App-freezes-on-opening-randomly
        After Qt upgrade to 5.11 we had a black screen crash after closing the application with
        the hardware button "Back" and trying to start the app again. It could only be fixed after
        totally closing the app swiping it in the list of running apps.
        This problem did not happen with the previous Qt version.
        After analysing changes we came up with this case and change:
            https://codereview.qt-project.org/#/c/218882/
        In summary they've moved libs loading to the same thread as main() and as a matter of correctness
        in the onDestroy method in QtActivityDelegate, they exit that thread with `QtNative.m_qtThread.exit();`
        That exit call is the main reason of this problem.

        In this fix we just replace the `QtApplication.invokeDelegate();` call that may end using the
        entire onDestroy method including that thread exit line for other three lines that purposely
        terminate qt (borrowed from QtActivityDelegate::onDestroy as well).
         */
        QtNative.terminateQt();
        QtNative.setActivity(null, null);
        System.exit(0);
        super.onDestroy();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        // Checks the orientation of the screen
        if (newConfig.orientation == Configuration.ORIENTATION_PORTRAIT){
//            Log.d("[VR]", "Portrait, forcing landscape");
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
            if (inVrMode) {
                inVrMode = false;
//                Log.d("[VR]", "Starting VR exit");
                nativeOnExitVr();                
            } else {
                Log.w("[VR]", "Portrait detected but not in VR mode. Should not happen");
            }
        }
        surfacesWorkaround();
    }

    private void surfacesWorkaround() {
        FrameLayout fl = findViewById(android.R.id.content);
        if (fl.getChildCount() > 0) {
            QtLayout qtLayout = (QtLayout) fl.getChildAt(0);
            List<QtSurface> surfaces = new ArrayList<>();
            for (int i = 0; i < qtLayout.getChildCount(); i++) {
                Object ch = qtLayout.getChildAt(i);
                if (ch instanceof QtSurface) {
                    surfaces.add((QtSurface) ch);
                }
            }
            if (surfaces.size() > 1) {
                QtSurface s1 = surfaces.get(0);
                QtSurface s2 = surfaces.get(1);
                Integer subLayer1 = 0;
                Integer subLayer2 = 0;
                try {
                    String field;
                    if (android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                        field = "mSubLayer";
                    } else {
                        field = "mWindowType";
                    }
                    Field f = s1.getClass().getSuperclass().getDeclaredField(field);
                    f.setAccessible(true);
                    subLayer1 = (Integer) f.get(s1);
                    subLayer2 = (Integer) f.get(s2);
                    if (subLayer1 < subLayer2) {
                        s1.setVisibility(View.VISIBLE);
                        s2.setVisibility(View.INVISIBLE);
                    } else {
                        s1.setVisibility(View.INVISIBLE);
                        s2.setVisibility(View.VISIBLE);
                    }
                } catch (ReflectiveOperationException e) {
                    Log.e(TAG, "Workaround failed");
                }
            }
        }
    }

    public void openUrlInAndroidWebView(String urlString) {
        Log.d("openUrl", "Received in open " + urlString);
        Intent openUrlIntent = new Intent(this, WebViewActivity.class);
        openUrlIntent.putExtra(WebViewActivity.WEB_VIEW_ACTIVITY_EXTRA_URL, urlString);
        startActivity(openUrlIntent);
    }

    /**
     * Called when view focus is changed
     */
    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            final int uiOptions = View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;

            getWindow().getDecorView().setSystemUiVisibility(uiOptions);
        }
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        if (intent.hasExtra(DOMAIN_URL)) {
            hideWebDrawer();
            nativeGotoUrl(intent.getStringExtra(DOMAIN_URL));
        } else if (intent.hasExtra(EXTRA_GOTO_USERNAME)) {
            hideWebDrawer();
            nativeGoToUser(intent.getStringExtra(EXTRA_GOTO_USERNAME));
        }
    }

    private void hideWebDrawer() {
        if (mWebSlidingDrawer != null) {
            mWebSlidingDrawer.setVisibility(View.GONE);
        }
    }

    public void showWebDrawer() {
        if (mWebSlidingDrawer == null) {
            FrameLayout mainLayout = findViewById(android.R.id.content);
            LayoutInflater inflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            QtLayout qtLayout = (QtLayout) mainLayout.getChildAt(0);
            mWebSlidingDrawer = (SlidingDrawer) inflater.inflate(R.layout.web_drawer, mainLayout, false);

            QtLayout.LayoutParams layoutParams = new QtLayout.LayoutParams(mWebSlidingDrawer.getLayoutParams());
            mWebSlidingDrawer.setOnDrawerCloseListener(() -> {
                WebViewFragment webViewFragment = (WebViewFragment) getFragmentManager().findFragmentByTag("webViewFragment");
                webViewFragment.close();
            });

            Point size = new Point();
            getWindowManager().getDefaultDisplay().getRealSize(size);
            int widthPx = Math.max(size.x, size.y);
            int heightPx = Math.min(size.x, size.y);

            layoutParams.x = (int) (widthPx - WEB_DRAWER_RIGHT_MARGIN * getResources().getDisplayMetrics().xdpi / NORMAL_DPI);
            layoutParams.y = (int) (heightPx - WEB_DRAWER_BOTTOM_MARGIN * getResources().getDisplayMetrics().ydpi / NORMAL_DPI);

            layoutParams.resolveLayoutDirection(View.LAYOUT_DIRECTION_RTL);
            qtLayout.addView(mWebSlidingDrawer, layoutParams);
        }
        mWebSlidingDrawer.setVisibility(View.VISIBLE);
    }

    public void openAndroidActivity(String activityName, boolean backToScene) {
        openAndroidActivity(activityName, backToScene, null);
    }

    public void openAndroidActivity(String activityName, boolean backToScene, HashMap args) {
        switch (activityName) {
            case "Home":
            case "Privacy Policy":
                nativeBeforeEnterBackground();
                Intent intent = new Intent(this, MainActivity.class);
                intent.putExtra(MainActivity.EXTRA_FRAGMENT, activityName);
                intent.putExtra(MainActivity.EXTRA_BACK_TO_SCENE, backToScene);
                startActivity(intent);
                break;
            case "Login":
                nativeBeforeEnterBackground();
                Intent loginIntent = new Intent(this, LoginMenuActivity.class);
                loginIntent.putExtra(LoginMenuActivity.EXTRA_BACK_TO_SCENE, backToScene);
                loginIntent.putExtra(LoginMenuActivity.EXTRA_BACK_ON_SKIP, true);
                if (args != null && args.containsKey(DOMAIN_URL)) {
                    loginIntent.putExtra(LoginMenuActivity.EXTRA_DOMAIN_URL, (String) args.get(DOMAIN_URL));
                }
                startActivity(loginIntent);
                break;
            case "WebView":
                runOnUiThread(() -> {
                    showWebDrawer();
                    if (!mWebSlidingDrawer.isOpened()) {
                        mWebSlidingDrawer.animateOpen();
                    }
                    if (args != null && args.containsKey(WebViewActivity.WEB_VIEW_ACTIVITY_EXTRA_URL)) {
                        WebViewFragment webViewFragment = (WebViewFragment) getFragmentManager().findFragmentByTag("webViewFragment");
                        webViewFragment.loadUrl((String) args.get(WebViewActivity.WEB_VIEW_ACTIVITY_EXTRA_URL), true);
                        webViewFragment.setToolbarVisible(true);
                        webViewFragment.setCloseAction(() -> {
                            if (mWebSlidingDrawer.isOpened()) {
                                mWebSlidingDrawer.animateClose();
                            }
                            hideWebDrawer();
                        });
                    }
                });
                break;
            default: {
                Log.w(TAG, "Could not open activity by name " + activityName);
                break;
            }
        }
    }

    public void onAppLoadedComplete() {
        isLoading = false;
        if (nativeEnterBackgroundCallEnqueued) {
            nativeEnterBackground();
        }
        runOnUiThread(() -> {
            nativeInitAfterAppLoaded();
        });
    }

    public void performHapticFeedback(int duration) {
        if (duration > 0) {
            mVibrator.vibrate(duration);
        }
    }

    @Override
    public void onBackPressed() {
        openAndroidActivity("Home", false);
    }

    @Override
    public void processURL(String url) { }

    @Override
    public void onWebLoaded(String url, WebViewFragment.SafenessLevel safenessLevel) { }

    @Override
    public void onTitleReceived(String title) { }

    @Override
    public void onExpand() {
    }

    @Override
    public void onOAuthAuthorizeCallback(Uri uri) { }
}
