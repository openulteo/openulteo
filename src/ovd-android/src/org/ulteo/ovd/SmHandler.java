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

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.List;
import java.util.Locale;
import java.util.TimeZone;
import java.util.Timer;
import java.util.TimerTask;
import org.ulteo.ovd.sm.Properties;
import org.ulteo.ovd.sm.ServerAccess;
import org.ulteo.ovd.sm.SessionManagerCommunication;
import org.ulteo.ovd.sm.SessionManagerException;
import org.ulteo.ovd.sm.SmUser;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

/**
 * specific callback for android the handler must implement all messages types
 * callback send messages to the main thread to show error messages to the user
 * 
 */
public class SmHandler extends Handler implements org.ulteo.ovd.sm.Callback {
	private static final String SM_STATUS_LOGGED = "logged";
	private static final String SM_CONNECTION_REFUSED_STR = "Connection refused";
	private static final String ERROR_AUTHENTICATION_FAILED = "auth_failed";
	private static final String ERROR_IN_MAINTENANCE = "in_maintenance";
	private static final String ERROR_INTERNAL = "internal_error";
	private static final String ERROR_INVALID_USER = "invalid_user";
	private static final String ERROR_SERVICE_NOT_AVAILABLE = "service_not_available";
	private static final String ERROR_UNAUTHORIZED_SESSION_MODE = "unauthorized_session_mode";
	private static final String ERROR_ACTIVE_SESSION = "user_with_active_session";
	private static final String ERROR_DEFAULT = "default";

	public static final int SM_ACTIVESESSION = 1;
	public static final int SM_ERROR = 2;
	public static final int SM_AUTH_FAILED = 3;
	public static final int SM_MAINTENANCE = 4;
	public static final int SM_INTERNAL = 5;
	public static final int SM_INVALID_USER = 6;
	public static final int SM_SERV_NOTAVAIL = 7;
	public static final int SM_BAD_SESSION_MODE = 8;
	public static final int SM_ERROR_DEFAULT = 9;
	public static final int SM_STATUS_ERROR = 10;
	public static final int SM_EXCEPTION = 11;
	public static final int SM_CONNECTION = 12;
	public static final int SM_LOGOUT = 13;
	public static final int SM_CONN_REFUSED = 14;
	public static final int SM_NO_CERTIFICATE = 15;
	public static final String OVD_RDP_SHELL = "OvdDesktop";

	private static SessionManagerCommunication sm;
	private boolean listen = false;
	private boolean poll = false;
	private TimerTask smStatus;
	private Timer statusTimer;
	private boolean cancel;
	private boolean errorCalled;
	
	private String sm_login;
	private String sm_passwd;
	private String sm_host;
	private int sm_port;

	public void disconnect() {
		if (listen)
			unlistenSm();

		if (poll)
			stopStatusPoll();

		sm = null;
	}

	public void connect(String host, int port, boolean use_https) {
		if (sm != null) {
			disconnect();
		}
		sm_host = host;
		sm_port = port;
		sm = new SessionManagerCommunication(host, port, use_https);
	}

	public void listenSm() {
		if (sm != null)
			sm.addCallbackListener(this);

		listen = true;
	}

	public void unlistenSm() {
		if (sm != null)
			sm.removeCallbackListener(this);

		listen = false;
	}

	/**
	 * manage dialog qith session manager for status
	 * 
	 */
	public void startStatusPoll() {
		poll = true;
		if (sm == null)
			return;
		smStatus = new TimerTask() {
			@Override
			public void run() {
				try {
					String result = sm.askForSessionStatus();
					if (result == null)
						return;
					if (result.compareToIgnoreCase(SM_STATUS_LOGGED) != 0)
						// bad status
						sendEmptyMessage(SM_STATUS_ERROR);
				} catch (NullPointerException e) {
					sendEmptyMessage(SM_INTERNAL);
					return;
				} catch (SessionManagerException e) {
					Message msg = new Message();
					msg.what = SM_EXCEPTION;
					msg.obj = new String(e.getMessage());
					sendMessage(msg);
				}
			}
		};
		statusTimer = new Timer();
		// launch periodic task for session status
		statusTimer.schedule(smStatus, Config.SM_STATUS_UPDATE_TIME, Config.SM_STATUS_UPDATE_TIME);
	}

	public void stopStatusPoll() {
		poll = false;
		if (sm == null)
			return;

		statusTimer.cancel();
		statusTimer.purge();
		smStatus = null;
		statusTimer = null;
	}

	public void cancelLogin() {
		cancel = true;
	}

	/**
	 * manage dialog with session manager for connection ask for desktop mode
	 * 
	 */
	public void login(final Properties request) {
		if (sm == null)
			return;

		cancel = false;
		sm_login = request.getLogin();
		sm_passwd = request.getPassword();

		Runnable smConnect = new Runnable() {

			@Override
			public void run() {
				Message msg = new Message();
				try {
					if (Config.PREMIUM) {
						try {
							Class<?> certificate_class = Class.forName("org.ulteo.ovd.premium.Certificate");
							Constructor<?> certificate_constructor = certificate_class
									.getConstructor(SessionManagerCommunication.class);
							Object l = certificate_constructor.newInstance(sm);
							if (Config.DEBUG)
								Log.d(Config.TAG, "Certificate system successfully loaded");
							java.lang.reflect.Method certificate_check = certificate_class.getMethod("check");
							certificate_check.invoke(l);
						} catch (ClassNotFoundException e) {
							if (Config.DEBUG)
								Log.d(Config.TAG, "No Certificate system found");
							sendEmptyMessage(SmHandler.SM_NO_CERTIFICATE);
							return;
						} catch (InvocationTargetException e) {
							if (Config.DEBUG)
								Log.d(Config.TAG, "Certificate error: " + e.getCause().getMessage());

							if (e.getCause().getMessage() != null) {
								if (e.getCause().getMessage().contains(SM_CONNECTION_REFUSED_STR)) {
									sendEmptyMessage(SM_CONN_REFUSED);
									return;
								} else if (sm.getLastResponseCode() != 501 &&
										sm.getLastResponseCode() != 200) {
									sm.setServiceSuffix(Config.SERVICE_SUFFIX_ALT);
								} else {
									sendEmptyMessage(SmHandler.SM_NO_CERTIFICATE);
									return;
								}
							} else {
								sendEmptyMessage(SmHandler.SM_NO_CERTIFICATE);
								return;
							}
						} catch (Exception e) {
							if (Config.DEBUG)
								Log.d(Config.TAG,
										"Certificate system error: " + e.getClass().getName() + " " + e.getMessage());

							sendEmptyMessage(SmHandler.SM_NO_CERTIFICATE);
							return;
						}
					}

					// ask sm for session
					errorCalled = false;
					if (!sm.askForSession(request)) {
						if (!errorCalled)
							sendEmptyMessage(SmHandler.SM_CONN_REFUSED);
						return;
					}
					// list of servers
					List<ServerAccess> access = sm.getServers();
					SmUser user = new SmUser();
					// get all infos from the first server
					user.login = access.get(0).getLogin();
					user.password = access.get(0).getPassword();
					user.serverAddr = access.get(0).getHost();
					user.port = access.get(0).getPort();
					// check if gateway is in use
					if (access.get(0).token == null)
						user.Gateway = false;
					else {
						user.Gateway = true;
						user.token = access.get(0).token;
					}

					while (sm != null && !cancel) {
						String status = sm.askForSessionStatus();
						if (Config.DEBUG)
							Log.d(Config.TAG, "Session status = " + status);
						if (status == null) {
							Log.e(Config.TAG, "Session status is null");
							sendEmptyMessage(SmHandler.SM_INTERNAL);
							return;
						}
						if (status.equals(SessionManagerCommunication.SESSION_STATUS_INITED)
								|| status.equals(SessionManagerCommunication.SESSION_STATUS_INACTIVE)) {
							msg.obj = user;
							msg.what = SM_CONNECTION;
							sendMessage(msg);
							return;
						} else if (!status.equals(SessionManagerCommunication.SESSION_STATUS_INIT)) {
							Log.e(Config.TAG, "Session not 'init' or 'ready' (" + status + ")");
							sendEmptyMessage(SmHandler.SM_INTERNAL);
							return;
						}

						Thread.sleep(Config.SM_STATUS_INIT_UPATE_TIME);
					}
					if (cancel) {
						Properties prop = getResponseProperties();
						if (prop != null && prop.isPersistent())
							sm.askForLogout(true);
						else
							sm.askForLogout(false);

						disconnect();
					}
				} catch (NullPointerException e) {
					sendEmptyMessage(SM_INTERNAL);
					return;
				} catch (SessionManagerException e) {
					if (e.getMessage() != null) {
						if (e.getMessage().contains(SM_CONNECTION_REFUSED_STR)) {
							sendEmptyMessage(SM_CONN_REFUSED);
							return;
						}
						msg.what = SM_EXCEPTION;
						msg.obj = new String(e.getMessage());
						sendMessage(msg);
					} else {
						sendEmptyMessage(SM_INTERNAL);
					}
				} catch (InterruptedException e) {
					msg.what = SM_EXCEPTION;
					msg.obj = new String(e.getMessage());
					sendMessage(msg);
				}
			}
		};

		new Thread(smConnect).start();
	}

	public void login(String login, String passwd) {
		// desktop is the only mode available on android
		Properties request = new Properties(Properties.MODE_DESKTOP);
		// set language and timezone for sm request
		request.setLang(Locale.getDefault().getLanguage());
		request.setTimeZone(TimeZone.getDefault().getID());
		request.setLogin(login);
		request.setPassword(passwd);
		login(request);
	}

	/**
	 * manage dialog with session manager for logout
	 * 
	 */
	public void logout(final boolean persistent) {
		Runnable smLogout = new Runnable() {
			@Override
			public void run() {
				try {
					if (!sm.askForLogout(persistent))
						return;

					sendEmptyMessage(SM_LOGOUT);
				} catch (SessionManagerException e) {
					Message msg = new Message();
					msg.what = SM_EXCEPTION;
					msg.obj = new String(e.getMessage());
					sendMessage(msg);
				}
			}
		};
		new Thread(smLogout).start();
	}

	@Override
	public void reportError(int code, String msg) {
		if (Config.DEBUG)
			Log.d(Config.TAG, "callback : reportError : code = " + code + " - msg = " + msg);
		// sendEmptyMessage(SM_ERROR);
	}

	@Override
	public void reportErrorStartSession(String code) {
		if (Config.DEBUG)
			Log.d(Config.TAG, "callback : reportErrorStartSession : code = " + code);
		if (code.compareToIgnoreCase(ERROR_ACTIVE_SESSION) == 0)
			sendEmptyMessage(SM_ACTIVESESSION);
		else if (code.compareToIgnoreCase(ERROR_AUTHENTICATION_FAILED) == 0)
			sendEmptyMessage(SM_AUTH_FAILED);
		else if (code.compareToIgnoreCase(ERROR_IN_MAINTENANCE) == 0)
			sendEmptyMessage(SM_MAINTENANCE);
		else if (code.compareToIgnoreCase(ERROR_INTERNAL) == 0)
			sendEmptyMessage(SM_INTERNAL);
		else if (code.compareToIgnoreCase(ERROR_INVALID_USER) == 0)
			sendEmptyMessage(SM_INVALID_USER);
		else if (code.compareToIgnoreCase(ERROR_SERVICE_NOT_AVAILABLE) == 0)
			sendEmptyMessage(SM_SERV_NOTAVAIL);
		else if (code.compareToIgnoreCase(ERROR_UNAUTHORIZED_SESSION_MODE) == 0)
			sendEmptyMessage(SM_BAD_SESSION_MODE);
		else if (code.compareToIgnoreCase(ERROR_DEFAULT) == 0)
			sendEmptyMessage(SM_ERROR_DEFAULT);
		else
			return;

		errorCalled = true;
	}

	@Override
	public void reportBadXml(String data) {
		if (Config.DEBUG)
			Log.d(Config.TAG, "callback : reportBadXml : data = " + data);
	}

	@Override
	public void reportUnauthorizedHTTPResponse(String moreInfos) {
		if (Config.DEBUG)
			Log.d(Config.TAG, "callback : reportUnauthorizedHTTPResponse : moreinfos = " + moreInfos);
	}

	@Override
	public void reportNotFoundHTTPResponse(String moreInfos) {
		if (Config.DEBUG)
			Log.d(Config.TAG, "callback : reportNotFoundHTTPResponse : moreinfos = " + moreInfos);
	}

	@Override
	public void sessionConnected() {
		if (Config.DEBUG)
			Log.d(Config.TAG, "callback : sessionConnected");
	}

	@Override
	public void sessionDisconnecting() {
		if (Config.DEBUG)
			Log.d(Config.TAG, "callback : sessionDisconnected");
	}

	public Properties getResponseProperties() {
		if (sm == null)
			return null;

		return sm.getResponseProperties();
	}

	public SessionManagerCommunication getSessionManagerCommunication() {
		return sm;
	}
	
	public String getConnectionUri() {
		return "ovd://"+sm_login+":"+sm_passwd+"@"+sm_host+":"+sm_port;
	}

}
