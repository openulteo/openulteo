/*
 * Copyright (C) 2011-2014 Ulteo SAS
 * http://www.ulteo.com
 * Author Clément Bizeau <cbizeau@ulteo.com> 2011
 * Author David PHAM-VAN <d.pham-van@ulteo.com> 2012-2014
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

import java.util.regex.Matcher;
import java.util.regex.Pattern;
import org.ulteo.ovd.sm.SessionManagerCommunication;
import org.ulteo.ovd.sm.SmUser;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.Intent.ShortcutIconResource;
import android.content.IntentFilter;
import android.content.pm.PackageInfo;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Message;
import android.support.v4.view.MenuItemCompat;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

/**
 * Main window of application, this window is used for the connection to OVD
 * user can enter password, login and address of the session manager, it will
 * also check the network state and check if gateway is in use, check the result
 * of OVD rdp connection launched from this class
 */
public class MainWindow extends Activity implements OnClickListener {
	private static final String REGEX_CHECK_IP = "[-a-zA-Z0-9+&@#/%?=~_|!:,.;]*[-a-zA-Z0-9+&@#/%=~_|]";
	private static final String REGEX_CHECK_PORT = ".*:[0-9]{1,5}";
	private static final String REGEX_CHECK_DIRECTRDP = "rdp://(.*)";

	private static final int STRING_START = 0;
	private static final int STRING_EMPTY = 0;
	private static final char PORT_SEPARATOR = ':';
	private static final int USE_DEFAULT_PORT = -1;

	public static final int MENU_HELP = 1;
	public static final int MENU_WEB = 2;
	public static final int MENU_SETTINGS = 3;
	public static final int MENU_SHORTCUT = 4;

	private EditText loginTxtField;
	private EditText passwdTxtField;
	private EditText sessionmTxtField;
	private TextView tvNoNet;

	private Button startBtn;
	private static MainWindow context = null;

	// handle messages for handling session manager connection
	protected static SmHandler smHandler = new SmHandler() {
		/*
		 * handle message for session manager thread manage errors and
		 * connection (non-Javadoc)
		 * @see android.os.Handler#handleMessage(android.os.Message)
		 */
		@Override
		public void handleMessage(Message msg) {
			if (context == null)
				return;
			
			switch (msg.what) {
			case SmHandler.SM_CONNECTION: // connection to sm is ok, start
											// rdp
				// connection with infos from sm
				SmUser user = (SmUser) msg.obj;
				Intent rdpIntent = new Intent(context, AndRdpActivity.class);
				Bundle params = new Bundle();
				params.putString(AndRdpActivity.PARAM_SM_URI, getConnectionUri());
				params.putString(AndRdpActivity.PARAM_RDPSHELL, SmHandler.OVD_RDP_SHELL);
				params.putString(AndRdpActivity.PARAM_LOGIN, user.login);
				params.putString(AndRdpActivity.PARAM_PASSWD, user.password);
				params.putBoolean(AndRdpActivity.PARAM_GATEWAYMODE, user.Gateway);
				if (user.Gateway) { // gateway is in use
					params.putString(AndRdpActivity.PARAM_IP, smHandler.getSessionManagerCommunication().getHost());
					params.putInt(AndRdpActivity.PARAM_PORT, user.port);
					params.putString(AndRdpActivity.PARAM_TOKEN, user.token);
				} else { // no gateway
					params.putString(AndRdpActivity.PARAM_IP, user.serverAddr);
					params.putInt(AndRdpActivity.PARAM_PORT, user.port);
				}
				rdpIntent.putExtras(params);
				smHandler.unlistenSm();
				context.startActivityForResult(rdpIntent, 1);
				break;
			case SmHandler.SM_ACTIVESESSION:
				Toast.makeText(context, R.string.active_session, Toast.LENGTH_LONG).show();
				break;
			case SmHandler.SM_LOGOUT:
				Toast.makeText(context, R.string.close_session, Toast.LENGTH_LONG).show();
				break;
			case SmHandler.SM_ERROR:
				Toast.makeText(context, R.string.error_connection, Toast.LENGTH_LONG).show();
				break;
			case SmHandler.SM_AUTH_FAILED:
				Toast.makeText(context, R.string.auth_failed, Toast.LENGTH_LONG).show();
				break;
			case SmHandler.SM_INVALID_USER:
				Toast.makeText(context, R.string.invalid_user, Toast.LENGTH_LONG).show();
				break;
			case SmHandler.SM_BAD_SESSION_MODE:
				Toast.makeText(context, R.string.bad_session_mode, Toast.LENGTH_LONG).show();
				break;
			case SmHandler.SM_SERV_NOTAVAIL:
				Toast.makeText(context, R.string.service_not_avail, Toast.LENGTH_LONG).show();
				break;
			case SmHandler.SM_MAINTENANCE:
				Toast.makeText(context, R.string.maintenance_mode, Toast.LENGTH_LONG).show();
				break;
			case SmHandler.SM_INTERNAL:
				Toast.makeText(context, R.string.internal_error, Toast.LENGTH_LONG).show();
				break;
			case SmHandler.SM_NO_CERTIFICATE:
				Toast.makeText(context, R.string.service_not_avail, Toast.LENGTH_LONG).show();
				break;
			case SmHandler.SM_ERROR_DEFAULT:
				Toast.makeText(context, R.string.default_error, Toast.LENGTH_LONG).show();
				break;
			case SmHandler.SM_CONN_REFUSED:
				Toast.makeText(context, R.string.conn_refused, Toast.LENGTH_LONG).show();
				break;
			case SmHandler.SM_EXCEPTION:
				String errorMsg = (String) msg.obj;
				Toast.makeText(context, errorMsg, Toast.LENGTH_LONG).show();
				break;
			}

			context.OnEndSmLogin();
		}

	};

	private InputMethodManager imm;

	protected String array_spinner[];
	protected ProgressDialog progressDialog = null;
	protected StatusConnection statusRun = new StatusConnection();

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		context = this;

		TextView tvVersion = (TextView) findViewById(R.id.tvVersion);
		try {
			ComponentName comp = new ComponentName(this, getClass());
			PackageInfo pinfo = getPackageManager().getPackageInfo(comp.getPackageName(), 0);
			tvVersion.setText(Config.VERSION.replace("%", pinfo.versionName));
		} catch (android.content.pm.PackageManager.NameNotFoundException e) {
			tvVersion.setText(Config.VERSION.replace("%", ""));
		}
		tvVersion.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				showWebsite();
			}
		});

		loginTxtField = (EditText) findViewById(R.id.login);
		passwdTxtField = (EditText) findViewById(R.id.password);
		sessionmTxtField = (EditText) findViewById(R.id.sessionm);
		tvNoNet = (TextView) findViewById(R.id.tvNoNet);

		imm = (InputMethodManager) getSystemService(INPUT_METHOD_SERVICE);

		startBtn = (Button) findViewById(R.id.button_Start);
		startBtn.setOnClickListener(this);
	}

	protected void OnEndSmLogin() {
		if (progressDialog != null) {
			progressDialog.dismiss();
			progressDialog = null;
		}
	}

	/*
	 * manage event for the window start and help buttons events (non-Javadoc)
	 * @see android.view.View.OnClickListener#onClick(android.view.View)
	 */
	@Override
	public void onClick(View v) {
		if (v.getId() == R.id.button_Start) {
			imm.hideSoftInputFromWindow(sessionmTxtField.getWindowToken(), 0);

			if (!checkConnectivity()) {
				Toast.makeText(MainWindow.this, R.string.error_internet, Toast.LENGTH_LONG).show();
				return;
			}

			// check if user enter a login
			if (loginTxtField.length() <= STRING_EMPTY) {
				Toast.makeText(MainWindow.this, R.string.error_miss_login, Toast.LENGTH_LONG).show();
				return;
			}
			// check if user enter a password
			if (passwdTxtField.length() <= STRING_EMPTY) {
				Toast.makeText(MainWindow.this, R.string.error_miss_passwd, Toast.LENGTH_LONG).show();
				return;
			}
			int port = USE_DEFAULT_PORT;
			String addr;
			if (Settings.getHideSm(this))
				addr = Settings.getIp(this);
			else
				addr = sessionmTxtField.getText().toString();

			// pattern for port checking
			Pattern p = Pattern.compile(REGEX_CHECK_PORT);
			Matcher m = p.matcher(addr);
			// check if the user specified the port
			if (m.matches()) {
				String portStr = addr.substring(addr.lastIndexOf(PORT_SEPARATOR) + 1);
				port = Integer.valueOf(portStr);
				addr = addr.substring(STRING_START, addr.lastIndexOf(PORT_SEPARATOR));
			}
			// pattern for rdp direct connection checking
			p = Pattern.compile(REGEX_CHECK_DIRECTRDP);
			m = p.matcher(addr);
			if (m.matches()) { // direct rdp connection
				String subaddr = m.group(1);
				// pattern for ip addr checking
				p = Pattern.compile(REGEX_CHECK_IP);
				m = p.matcher(subaddr);
				// check ip addr
				if (!m.matches()) {
					Toast.makeText(MainWindow.this, R.string.error_ipaddr, Toast.LENGTH_LONG).show();
					return;
				}
				// start activity for rdp connection
				Intent rdpIntent = new Intent(MainWindow.this, AndRdpActivity.class);
				Bundle params = new Bundle();
				// give all parameters to the activity
				params.putString(AndRdpActivity.PARAM_LOGIN, loginTxtField.getText().toString());
				params.putString(AndRdpActivity.PARAM_PASSWD, passwdTxtField.getText().toString());
				params.putString(AndRdpActivity.PARAM_IP, subaddr);
				params.putInt(AndRdpActivity.PARAM_PORT, port);
				rdpIntent.putExtras(params);
				startActivity(rdpIntent);
			} else { // use session manager
				p = Pattern.compile(REGEX_CHECK_IP);
				m = p.matcher(addr);
				// check ip addr
				if (!m.matches()) {
					Toast.makeText(MainWindow.this, R.string.error_ipaddr, Toast.LENGTH_LONG).show();
					return;
				}
				if (port < 0) // use default port for sm connection
					port = SessionManagerCommunication.DEFAULT_PORT;
				// save gateway addr, it can be necessary after to connect rdp
				// server
				smHandler.connect(addr, port, true);
				smHandler.listenSm();
				progressDialog = ProgressDialog.show(this, getString(R.string.loading_open_virtual_desktop),
						getString(R.string.waiting_server_for_session),
						true, true, new DialogInterface.OnCancelListener() {
							@Override
							public void onCancel(DialogInterface dialog) {
								Log.i(Config.TAG, "Cancelling connection");
								smHandler.cancelLogin();
							}
						});
				
				// start thread for sm connection
				smHandler.login(loginTxtField.getText().toString(), passwdTxtField.getText().toString());
			}
		} else
			HelpDialog.showHelp(this);
	}

	@Override
	protected synchronized void onPause() {
		// save user preferences when application goes invisible
		SaveSelections();
		unregisterReceiver(statusRun);
		if (progressDialog != null) {
			if (Config.DEBUG)
				Log.d(Config.TAG, "Ahead dismiss the progressDialog");

			progressDialog.dismiss();
			progressDialog = null;
		}
		super.onPause();
	}

	@Override
	protected synchronized void onResume() {
		// restart internet connection status watching thread
		IntentFilter filter = new IntentFilter();
		filter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
		registerReceiver(statusRun, filter);
		Boolean net = checkConnectivity();
		startBtn.setEnabled(net);
		tvNoNet.setVisibility(net ? View.GONE : View.VISIBLE);
		findViewById(R.id.sessionm_layout).setVisibility(Settings.getHideSm(this) ? View.GONE : View.VISIBLE);
		// reload user preferences
		LoadSelections();

		// Always show the session if one is opened.
		if (AndRdpActivity.getRdp() != null)
			startActivityForResult(new Intent(MainWindow.this, AndRdpActivity.class), 1);

		super.onResume();
	}

	@Override
	protected void onStop() {
		// set password to null when application is stopped
		if (!Config.SAVE_PASSWORD)
			if (Settings.getSavePasswd(this))
				passwdTxtField.setText("");

		super.onStop();
	}

	/**
	 * save user preferences
	 */
	private void SaveSelections() {
		Settings.setLogin(this, loginTxtField.getText().toString());
		Settings.setIp(this, sessionmTxtField.getText().toString());
		Settings.setPasswd(this, passwdTxtField.getText().toString());
	}

	/**
	 * load user preferences
	 */
	private void LoadSelections() {
		loginTxtField.setText(Settings.getLogin(this));
		if (Config.SAVE_PASSWORD) {
			if (Settings.getSavePasswd(this))
				passwdTxtField.setText(Settings.getPasswd(this));
			else
				passwdTxtField.setText("");
		} else
			passwdTxtField.setText("");

		sessionmTxtField.setText(Settings.getIp(this));
	}

	/*
	 * check the result of child activity check if sm logout is needed
	 * (non-Javadoc)
	 * @see android.app.Activity#onActivityResult(int, int,
	 * android.content.Intent)
	 */
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);
		if (resultCode == AndRdpActivity.EXIT_WITH_LOGOUT) { // check if logout
																// is needed,
																// result
			// for child activity
			smHandler.listenSm();
			smHandler.logout(false);
		} else if (resultCode == AndRdpActivity.EXIT_WITH_SUSPEND) { // check if
																		// suspend
																		// is
																		// needed,
																		// result
			// for child activity
			smHandler.listenSm();
			smHandler.logout(true);
		}

	}

	private boolean checkConnectivity() {
		ConnectivityManager connectionManager = (ConnectivityManager) getSystemService(CONNECTIVITY_SERVICE);
		try {
			if (connectionManager.getActiveNetworkInfo().isConnected()) {
				return true;
			} else {
				return false;
			}
		} catch (NullPointerException e) {
			return false;
		}
	}

	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		if (keyCode == KeyEvent.KEYCODE_ENTER) {
			if (!Settings.getHideSm(this)) {
				if (!loginTxtField.equals("") && !passwdTxtField.equals("") && sessionmTxtField.isFocused())
					startBtn.performClick();
			} else {
				if (!loginTxtField.equals("") && !passwdTxtField.equals("") && passwdTxtField.isFocused())
					startBtn.performClick();
			}
			return true;
		}

		return super.onKeyUp(keyCode, event);
	}

	public class StatusConnection extends BroadcastReceiver {
		@Override
		public void onReceive(Context context, Intent intent) {
			NetworkInfo info = (NetworkInfo) intent.getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);
			Boolean net = info.getState().equals(NetworkInfo.State.CONNECTED);
			startBtn.setEnabled(net);
			tvNoNet.setVisibility(net ? View.GONE : View.VISIBLE);
		}
	}

	public void showWebsite() {
		Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(Config.WEBSITE));
		startActivity(browserIntent);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		MenuItemCompat.setShowAsAction(menu.add(0, MENU_HELP, 0, R.string.Help)
				.setIcon(android.R.drawable.ic_menu_help), MenuItemCompat.SHOW_AS_ACTION_IF_ROOM
				| MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
		menu.add(0, MENU_WEB, 0, R.string.ulteo_website).setIcon(android.R.drawable.ic_menu_more);
		menu.add(0, MENU_SHORTCUT, 0, R.string.create_shortcut).setIcon(android.R.drawable.ic_menu_add);
		MenuItemCompat.setShowAsAction(
				menu.add(0, MENU_SETTINGS, 0, "Settings").setIcon(android.R.drawable.ic_menu_preferences),
				MenuItemCompat.SHOW_AS_ACTION_IF_ROOM);
		return true;
	}

	private void addShortcut(){
		// check if user enter a login
		if (loginTxtField.length() <= STRING_EMPTY) {
			Toast.makeText(MainWindow.this, R.string.error_miss_login, Toast.LENGTH_LONG).show();
			return;
		}
		// check if user enter a password
		if (passwdTxtField.length() <= STRING_EMPTY) {
			Toast.makeText(MainWindow.this, R.string.error_miss_passwd, Toast.LENGTH_LONG).show();
			return;
		}
		// check if user enter an address
		String addr;
		if (Settings.getHideSm(this))
			addr = Settings.getIp(this);
		else
			addr = sessionmTxtField.getText().toString();
		if (addr.length() <= STRING_EMPTY) {
			Toast.makeText(MainWindow.this, R.string.error_ipaddr, Toast.LENGTH_LONG).show();
			return;
		}

		Intent shortcut = new Intent("com.android.launcher.action.INSTALL_SHORTCUT");
		shortcut.putExtra(Intent.EXTRA_SHORTCUT_NAME, loginTxtField.getText().toString() + "@" + addr);
		shortcut.putExtra("duplicate", false); // Just create once
		String url = "ovd://" + loginTxtField.getText().toString() + ":" + passwdTxtField.getText().toString() + "@" + addr;
		Intent shortcutIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
		shortcut.putExtra(Intent.EXTRA_SHORTCUT_INTENT, shortcutIntent);
		ShortcutIconResource iconRes = Intent.ShortcutIconResource.fromContext(this, R.drawable.icon);
		shortcut.putExtra(Intent.EXTRA_SHORTCUT_ICON_RESOURCE, iconRes);
		sendBroadcast(shortcut);
	}

	@Override
	public boolean onMenuItemSelected(int featureId, MenuItem item) {
		switch (item.getItemId()) {
		case MENU_HELP:
			HelpDialog.showHelp(this);
			return true;
		case MENU_WEB:
			showWebsite();
			return true;
		case MENU_SETTINGS:
			startActivity(new Intent(this, Settings.class));
			return true;
		case MENU_SHORTCUT:
			addShortcut();
			return true;
		}
		return false;
	}
}
