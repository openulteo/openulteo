/*
 * Copyright (C) 2011-2014 Ulteo SAS
 * http://www.ulteo.com
 * Author Pierre Laine <plaine@ulteo.com> 2011
 * Author Clément Bizeau <cbizeau@ulteo.com> 2011
 * Author David PHAM-VAN <d.pham-van@ulteo.com> 2012, 2013, 2014
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

package org.ulteo.ovd;

import org.ulteo.ovd.sm.Properties;
import org.ulteo.ovd.sm.SessionManagerCommunication;
import android.annotation.TargetApi;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.graphics.Point;
import android.media.AudioManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.nfc.NdefMessage;
import android.nfc.NdefRecord;
import android.nfc.NfcAdapter;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v4.app.NotificationCompat;
import android.support.v4.view.MenuItemCompat;
import android.util.Log;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewTreeObserver.OnGlobalLayoutListener;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Toast;

/**
 * rdp avtivity view for rdp drawing, load all libraries for jni freerdp and
 * start freerdp, drawing is done in c, this class manage the transformation
 * like rotation and zoom
 * 
 */
public class AndRdpActivity extends Activity implements Panel.OnThreeFingersHandler {
	public static final int EXIT_WITH_LOGOUT = 1;
	public static final int EXIT_WITH_NOLOGOUT = 2;
	public static final int EXIT_WITH_SUSPEND = 3;

	public static final int ERROR_ALLOC = 1;
	public static final int ERROR_RDP_INIT = 101;
	public static final int ERROR_RDP_CONNECT = 102;
	public static final int ERROR_JNI_FINDCLASS = 201;
	public static final int ERROR_JNI_ATTACHTHREAD = 202;
	public static final int ERROR_JNI_DETACHTHREAD = 203;

	private static final int MENU_EXIT = 1;
	private static final int MENU_HELP = 2;
	private static final int MENU_KBD = 3;

	private static final int MSG_DIM_SU = 1;
	private static final int MSG_DIM_AB = 2;

	public static final String PARAM_LOGIN = "login";
	public static final String PARAM_PASSWD = "password";
	public static final String PARAM_IP = "ip";
	public static final String PARAM_PORT = "port";
	public static final String PARAM_GATEWAYMODE = "gateway_mode";
	public static final String PARAM_TOKEN = "token";
	public static final String PARAM_RDPSHELL = "rdpshell";
	public static final String PARAM_SM_URI = "sm_uri";

	private Panel view;
	private static SmHandler smHandler = new SmHandler() {

		@Override
		public void handleMessage(Message msg) {
			if (context == null)
				return;
			
			switch (msg.what) {
			case SmHandler.SM_STATUS_ERROR:
				context.stopActivity(EXIT_WITH_NOLOGOUT);
				break;
			case org.ulteo.ovd.SmHandler.SM_ERROR:
				Toast.makeText(context, R.string.error_connection, Toast.LENGTH_LONG).show();
				context.stopActivity(EXIT_WITH_NOLOGOUT);
				break;
			case SmHandler.SM_EXCEPTION:
				String errorMsg = (String) msg.obj;
				Toast.makeText(context, errorMsg, Toast.LENGTH_LONG).show();
				context.stopActivity(EXIT_WITH_NOLOGOUT);
				break;
			}
		}

	};

	private static Handler dimHandler = new Handler() {
		@TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH)
		public void handleMessage(Message msg) {
			if (context == null)
				return;
			
			if (msg.what == MSG_DIM_SU) {
				if (Settings.getSystemUiLowProfile(context)) {
					context.hideSystemUi();
				}
			} else if (msg.what == MSG_DIM_AB) {
				context.getActionBar().hide();
				if (Settings.getSystemUiLowProfile(context)) {
					context.hideSystemUi();
				}
			}
		};
	};

	private StatusConnection statusRun = new StatusConnection();
	private static Rdp rdp;
	private static AndRdpActivity context = null;

	public static Rdp getRdp() {
		return rdp;
	}
	
	public static boolean isConnected() {
		return rdp != null;
	}

	public static boolean isLoggedIn() {
		return rdp != null && rdp.isLoggedIn();
	}

	private ClipboardManager.OnPrimaryClipChangedListener clipChangedListener;
	
	@TargetApi(Build.VERSION_CODES.KITKAT)
	private void hideSystemUi() {
		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT) {
			view.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE
					| View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
					| View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
					| View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
					| View.SYSTEM_UI_FLAG_FULLSCREEN
					| View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
		} else {
			view.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LOW_PROFILE);
		}
	}

	@TargetApi(Build.VERSION_CODES.KITKAT)
	private void showSystemUi() {
		view.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
	}

	@Override
	@TargetApi(Build.VERSION_CODES.KITKAT)
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		context = this;

		Log.i(Config.TAG, "Android SDK " + android.os.Build.VERSION.SDK_INT);
		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.HONEYCOMB) {
			requestWindowFeature(Window.FEATURE_ACTION_BAR_OVERLAY);
			getActionBar().setBackgroundDrawable(getResources().getDrawable(R.drawable.actionbar_bg));
			getActionBar().hide();
		} else {
			requestWindowFeature(Window.FEATURE_NO_TITLE);
		}
		int flags = WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN;
		if (Settings.getFullscreen(this)) {
			flags |= WindowManager.LayoutParams.FLAG_FULLSCREEN;
		}
		getWindow().addFlags(flags);

		view = new Panel(this);
		view.threeFingersHandler = this;
		setContentView(view);

		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.HONEYCOMB
				&& Settings.getSystemUiLowProfile(this)) {
			hideSystemUi();
			view.setOnSystemUiVisibilityChangeListener(new View.OnSystemUiVisibilityChangeListener() {
				@Override
				public void onSystemUiVisibilityChange(int visibility) {
					Log.i(Config.TAG, "SystemUiVisibilityChange to " + visibility);
					if (visibility == View.SYSTEM_UI_FLAG_VISIBLE
							&& Settings.getSystemUiLowProfile(AndRdpActivity.this))
						dimHandler.sendEmptyMessageDelayed(MSG_DIM_SU, Config.SYSTEM_UI_DIM_TIME);
				}
			});
		}

		// add callback if sm is allocated
		smHandler.listenSm();
		smHandler.startStatusPoll();

		view.getViewTreeObserver().addOnGlobalLayoutListener(new OnGlobalLayoutListener() {
			@Override
			public void onGlobalLayout() {
				startSession(view.getWidth(), view.getHeight());
				view.surfaceCreated();
			}	
		});
	}

	@TargetApi(Build.VERSION_CODES.JELLY_BEAN)
	private void startSession(int screen_width, int screen_height) {
		if (isConnected())
			return;

		Bundle bundle = getIntent().getExtras();

		if (bundle == null) {
			finish();
			return;
		}

		Point res;
		// check if user has chosen a specific resolution
		if (!Settings.getResolutionAuto(this)) {
			res = Settings.getResolution(this);
		} else {
			res = new Point(screen_width, screen_height);
			// Prefer wide screen
			if (!Settings.getResolutionWide(this) &&  res.y > res.x) {
				int w = res.x;
				res.x = res.y;
				res.y = w;
			}
		}
		Log.i(Config.TAG, "Resolution: " + res.x + "x" + res.y);

		String gateway_token = null;
		Boolean gateway_mode = bundle.getBoolean(PARAM_GATEWAYMODE);
		if (gateway_mode)
			gateway_token = bundle.getString(PARAM_TOKEN);

		int drives = Properties.REDIRECT_DRIVES_FULL;
		Properties prop = smHandler.getResponseProperties();
		if (prop != null)
			drives = prop.isDrives();
		
		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.JELLY_BEAN && bundle.getString(PARAM_SM_URI) != null) {
			NfcAdapter nfcAdapter = NfcAdapter.getDefaultAdapter(this);
			if (nfcAdapter != null) {
				NdefRecord rtdUriRecord = NdefRecord.createUri(bundle.getString(PARAM_SM_URI));
				NdefMessage ndefMessage = new NdefMessage(rtdUriRecord);
				nfcAdapter.setNdefPushMessage(ndefMessage, this);
			}
		}

		rdp = new Rdp(res, bundle.getString(PARAM_LOGIN), bundle.getString(PARAM_PASSWD), bundle.getString(PARAM_IP),
				bundle.getInt(PARAM_PORT, SessionManagerCommunication.DEFAULT_RDP_PORT), gateway_mode, gateway_token,
				drives, bundle.getString(PARAM_RDPSHELL) == null ? "" : bundle.getString(PARAM_RDPSHELL), 
				Settings.getBulkCompression(AndRdpActivity.this), Settings.getConnexionType(AndRdpActivity.this));

		Resources resources = getResources();
		Intent notificationIntent = new Intent(this, AndRdpActivity.class);
		PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);
		NotificationCompat.Builder mBuilder = new NotificationCompat.Builder(this).setSmallIcon(R.drawable.icon_bw)
				.setContentTitle(resources.getText(R.string.app_name))
				.setOngoing(true)
				.setContentText(resources.getText(R.string.desktop_session_active)).setContentIntent(pendingIntent);
		NotificationManager mNotificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
		mNotificationManager.notify(1, mBuilder.build());
		
		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.HONEYCOMB) {
			ClipboardManager clipboard = (ClipboardManager)getSystemService(Context.CLIPBOARD_SERVICE);
			clipChangedListener = new ClipboardManager.OnPrimaryClipChangedListener() {
					@Override
					@TargetApi(Build.VERSION_CODES.HONEYCOMB)
					public void onPrimaryClipChanged() {
						ClipboardManager clipboard = (ClipboardManager)AndRdpActivity.this.getSystemService(Context.CLIPBOARD_SERVICE);
						ClipData clip = clipboard.getPrimaryClip();
						String text = clip.getItemAt(0).coerceToText(AndRdpActivity.this).toString();
						if (Config.DEBUG)
							Log.d(Config.TAG, "Android clipboard : " + text);
	
						if (isLoggedIn())
							rdp.sendClipboard(text);
					}
				};
			clipboard.addPrimaryClipChangedListener(clipChangedListener);
		}
	}

	@Override
	public boolean dispatchKeyEvent(KeyEvent event) {
		AudioManager audioManager;
		
		if (Config.DEBUG)
			Log.i(Config.TAG, "AndRdpActivity.dispatchKeyEvent " + event);

		// Disable keypress if it is from a mouse or touchpad.
		if (event.getDeviceId() >= 0) {
			InputDevice dev = InputDevice.getDevice(event.getDeviceId());
			if (dev.getSources() == InputDevice.SOURCE_MOUSE || dev.getSources() == InputDevice.SOURCE_TOUCHPAD) 
				return true;
		}

		if (event.getAction() == KeyEvent.ACTION_DOWN) {
			switch (event.getKeyCode()) {
			case KeyEvent.KEYCODE_MENU:
				OnThreeFingers(null);
				return true;
			case KeyEvent.KEYCODE_BACK:
				Log.i(Config.TAG, "Back key pressed");
				askLogoutDialog();
				return true;
			case KeyEvent.KEYCODE_VOLUME_UP:
				audioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
				audioManager.adjustStreamVolume(AudioManager.STREAM_MUSIC, AudioManager.ADJUST_RAISE,
						AudioManager.FLAG_SHOW_UI);
				return true;
			case KeyEvent.KEYCODE_VOLUME_DOWN:
				audioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
				audioManager.adjustStreamVolume(AudioManager.STREAM_MUSIC, AudioManager.ADJUST_LOWER,
						AudioManager.FLAG_SHOW_UI);
				return true;
			}
		} else if (event.getAction() == KeyEvent.ACTION_UP) {
			switch (event.getKeyCode()) {
			case KeyEvent.KEYCODE_MENU:
			case KeyEvent.KEYCODE_BACK:
			case KeyEvent.KEYCODE_VOLUME_UP:
			case KeyEvent.KEYCODE_VOLUME_DOWN:
				return true;
			}
		}

		if (isLoggedIn() && rdp.dispatchKeyEvent(event))
			return true;

		return super.dispatchKeyEvent(event);
	}

	private void askLogoutDialog() {
		DialogInterface.OnClickListener dialogClickListener = new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				if (which == DialogInterface.BUTTON_POSITIVE) {
					stopActivity(EXIT_WITH_LOGOUT);
				} else if (which == DialogInterface.BUTTON_NEUTRAL) {
					stopActivity(EXIT_WITH_SUSPEND);
				}

			}
		};

		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		Builder msg = builder.setMessage(R.string.leave_session)
				.setPositiveButton(android.R.string.yes, dialogClickListener)
				.setNegativeButton(android.R.string.no, dialogClickListener);

		Properties prop = smHandler.getResponseProperties();
		if (prop != null && prop.isPersistent())
			msg.setNeutralButton(R.string.suspend, dialogClickListener);

		msg.show();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		MenuItemCompat.setShowAsAction(menu.add(0, MENU_HELP, 0, R.string.Help)
				.setIcon(android.R.drawable.ic_menu_help), MenuItemCompat.SHOW_AS_ACTION_IF_ROOM
				| MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
		MenuItemCompat.setShowAsAction(
				menu.add(0, MENU_KBD, 0, R.string.toggle_keyboard).setIcon(android.R.drawable.ic_menu_edit),
				MenuItemCompat.SHOW_AS_ACTION_IF_ROOM | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
		MenuItemCompat.setShowAsAction(
				menu.add(0, MENU_EXIT, 0, R.string.logout).setIcon(android.R.drawable.ic_menu_close_clear_cancel),
				MenuItemCompat.SHOW_AS_ACTION_IF_ROOM | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);

		return true;
	}

	@TargetApi(11)
	@Override
	public boolean onMenuItemSelected(int featureId, MenuItem item) {
		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.HONEYCOMB) {
			getActionBar().hide();
		}
		switch (item.getItemId()) {
		case MENU_HELP:
			HelpDialog.showHelp(this);
			return true;
		case MENU_EXIT:
			askLogoutDialog();
			return true;
		case MENU_KBD:
			view.toggleKeyboard();
			return true;
		}
		return false;
	}

	@Override
	@TargetApi(Build.VERSION_CODES.HONEYCOMB)
	protected void onDestroy() {
		smHandler.unlistenSm();
		smHandler.stopStatusPoll();
		NotificationManager mNotificationManager = (NotificationManager) context
				.getSystemService(Context.NOTIFICATION_SERVICE);
		mNotificationManager.cancel(1);
		
		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.HONEYCOMB) {
			ClipboardManager clipboard = (ClipboardManager)getSystemService(Context.CLIPBOARD_SERVICE);
			clipboard.removePrimaryClipChangedListener(clipChangedListener);
		}
		
		super.onDestroy();
	}

	/**
	 * stop activity, stopping freerdp is needed to close the connection stop
	 * asking for session manager status
	 * 
	 * @param result
	 *            result of the activity
	 */
	private void stopActivity(int result) {
		setResult(result);
		if (isLoggedIn())
			rdp.logout();

		finish();
	}

	public static void deleteRdp() {
		rdp = null;
		NotificationManager mNotificationManager = (NotificationManager) context
				.getSystemService(Context.NOTIFICATION_SERVICE);
		mNotificationManager.cancel(1);
	}

	private class StatusConnection extends BroadcastReceiver {
		private ProgressDialog connectionLostDialog;

		@Override
		public void onReceive(Context context, Intent intent) {
			NetworkInfo info = (NetworkInfo) intent.getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);
			if (info.getState().equals(NetworkInfo.State.CONNECTED)) {
				if (connectionLostDialog != null) {
					connectionLostDialog.dismiss();
					connectionLostDialog = null;
				}
			} else {
				if (connectionLostDialog == null) {
					connectionLostDialog = new ProgressDialog(context);
					connectionLostDialog.setMessage(context.getString(R.string.wait_reco));
					connectionLostDialog.setTitle(context.getString(R.string.connection_lost));
					connectionLostDialog.setButton(DialogInterface.BUTTON_NEGATIVE, context.getString(R.string.logout),
							new DialogInterface.OnClickListener() {
								@Override
								public void onClick(DialogInterface dialog, int which) {
									finish();
								}
							});
					connectionLostDialog.show();
				}
			}
		}
	}

	@Override
	protected void onPause() {
		unregisterReceiver(statusRun);
		dimHandler.removeMessages(MSG_DIM_SU);
		super.onPause();
	}

	@TargetApi(14)
	@Override
	protected void onResume() {
		IntentFilter filter = new IntentFilter();
		filter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
		registerReceiver(statusRun, filter);
		if (isConnected())
			rdp.draw();
		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.HONEYCOMB
				&& Settings.getSystemUiLowProfile(this)) {
			hideSystemUi();
		}
		super.onResume();
	}

	@TargetApi(11)
	@Override
	public void OnThreeFingers(Panel panel) {
		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.HONEYCOMB) {
			if (getActionBar().isShowing())
				getActionBar().hide();
			else {
				getActionBar().show();
				if (Settings.getSystemUiLowProfile(context)) {
					showSystemUi();
				}
				dimHandler.sendEmptyMessageDelayed(MSG_DIM_AB, Config.ACTION_BAR_DIM_TIME);
			}
		} else {
			openOptionsMenu();
		}
	}
}
