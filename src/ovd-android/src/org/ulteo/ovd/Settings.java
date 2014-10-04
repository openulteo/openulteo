/*
 * Copyright (C) 2012 Ulteo SAS
 * http://www.ulteo.com
 * Author David PHAM-VAN <d.pham-van@ulteo.com> 2012
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

import android.app.backup.BackupManager;
import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Point;
import android.os.Bundle;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceManager;
import android.text.InputType;
import android.util.Log;

public class Settings extends PreferenceActivity {
	private static final String LOGIN = "login";
	private static final String PASSWD = "password";
	private static final String IP = "ip";
	private static final String RESOLUTION = "resolution";
	private static final String RESOLUTION_WIDE = "resolution_wide";
	private static final String RESOLUTION_AUTO = "resolution_auto";
	private static final String HIDE_SM = "hide_session_manager";
	private static final String SAVE_PASSWD = "save_password";
	private static final String HIDE_SYSTEM_UI = "hide_system_ui";
	private static final String BULK_COMPRESSION = "bulk_compression";
	private static final String FULLSCREEN = "fullscreen";
	private static final String IME = "ime";
	private static final String CONNEXION_TYPE = "connexion_type";

	private static SharedPreferences prefs = null;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		SharedPreferences params = getSharedPrefs(this);
		addPreferencesFromResource(R.xml.settings);

		Preference pw = findPreference(SAVE_PASSWD);
		if (!Config.SAVE_PASSWORD) {
			PreferenceCategory cat = (PreferenceCategory) findPreference("sm_category");
			cat.removePreference(pw);
		}

		OnPreferenceChangeListener summary_update = new OnPreferenceChangeListener() {
			@Override
			public boolean onPreferenceChange(Preference preference, Object newValue) {
				preference.setSummary((String) newValue);
				return true;
			}
		};

		OnPreferenceChangeListener summary_update_array = new OnPreferenceChangeListener() {
			@Override
			public boolean onPreferenceChange(Preference preference, Object newValue) {
				int index = ((ListPreference) preference).findIndexOfValue((String) newValue);
				preference.setSummary(((ListPreference) preference).getEntries()[index]);
				return true;
			}
		};

		Preference res = findPreference(RESOLUTION);
		res.setSummary(params.getString(RESOLUTION, ""));
		res.setOnPreferenceChangeListener(summary_update);

		EditTextPreference ip = (EditTextPreference) findPreference(IP);
		ip.getEditText().setRawInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_URI);
		ip.setSummary(params.getString(IP, ""));
		ip.setOnPreferenceChangeListener(summary_update);

		res = findPreference(HIDE_SYSTEM_UI);
		if (android.os.Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.HONEYCOMB) {
			PreferenceCategory cat = (PreferenceCategory) findPreference("category_session");
			cat.removePreference(res);
		}

		res = findPreference(CONNEXION_TYPE);
		res.setSummary(((ListPreference) res).getEntry().toString());
		res.setOnPreferenceChangeListener(summary_update_array);
	}

	public static SharedPreferences getSharedPrefs(Context context) {
		if (prefs == null) {
			prefs = PreferenceManager.getDefaultSharedPreferences(context);
			PreferenceManager.setDefaultValues(context, R.xml.settings, false);
		}

		return prefs;
	}

	public static String getLogin(Context context) {
		return getSharedPrefs(context).getString(LOGIN, "");
	}

	public static void setLogin(Context context, String value) {
		SharedPreferences.Editor prefEditor = getSharedPrefs(context).edit();
		prefEditor.putString(Settings.LOGIN, value);
		prefEditor.commit();
		dataChanged(context);
	}

	public static String getPasswd(Context context) {
		return getSharedPrefs(context).getString(PASSWD, "");
	}

	public static void setPasswd(Context context, String value) {
		if (Config.SAVE_PASSWORD) {
			SharedPreferences.Editor prefEditor = getSharedPrefs(context).edit();
			if (getSavePasswd(context))
				prefEditor.putString(Settings.PASSWD, value);
			else
				prefEditor.remove(Settings.PASSWD);
			prefEditor.commit();
			dataChanged(context);
		}
	}

	public static String getIp(Context context) {
		return getSharedPrefs(context).getString(IP, "");
	}

	public static void setIp(Context context, String value) {
		if (!getHideSm(context)) {
			SharedPreferences.Editor prefEditor = getSharedPrefs(context).edit();
			prefEditor.putString(Settings.IP, value);
			prefEditor.commit();
			dataChanged(context);
		}
	}

	public static Boolean getResolutionAuto(Context context) {
		return !getSharedPrefs(context).getBoolean(RESOLUTION_AUTO, false);
	}

	public static Boolean getResolutionWide(Context context) {
		return !getSharedPrefs(context).getBoolean(RESOLUTION_WIDE, false);
	}

	public static Point getResolution(Context context) {
		Point res = new Point();
		String prefResolution = getSharedPrefs(context).getString(RESOLUTION, null);
		res.x = Integer.parseInt(prefResolution.split("x")[0]);
		res.y = Integer.parseInt(prefResolution.split("x")[1]);
		return res;
	}

	public static Boolean getHideSm(Context context) {
		return getSharedPrefs(context).getBoolean(HIDE_SM, false);
	}

	public static Boolean getSavePasswd(Context context) {
		return getSharedPrefs(context).getBoolean(SAVE_PASSWD, false);
	}

	public static Boolean getSystemUiLowProfile(Context context) {
		return getSharedPrefs(context).getBoolean(HIDE_SYSTEM_UI, false);
	}

	public static Boolean getBulkCompression(Context context) {
		return getSharedPrefs(context).getBoolean(BULK_COMPRESSION, false);
	}

	public static Boolean getFullscreen(Context context) {
		return getSharedPrefs(context).getBoolean(FULLSCREEN, false);
	}

	public static Boolean getIme(Context context) {
		return getSharedPrefs(context).getBoolean(IME, false);
	}

	public static int getConnexionType(Context context) {
		try {
			return Integer.valueOf(getSharedPrefs(context).getString(CONNEXION_TYPE, "0"));
		} catch (NumberFormatException e) {
			return 0;
		}
	}

	@Override
	protected void onStop() {
		dataChanged(this);
		super.onStop();
	}

	public static void dataChanged(Context context) {
		if (Config.DEBUG) {
			Log.d(Config.TAG, "Data changed, schedule backup");
		}
		new BackupManager(context).dataChanged();
	}

}
