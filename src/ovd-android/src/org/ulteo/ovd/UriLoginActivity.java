/*
 * Copyright (C) 2013 Ulteo SAS
 * http://www.ulteo.com
 * Author David PHAM-VAN <d.pham-van@ulteo.com> 2012, 2013
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

import java.util.Arrays;
import java.util.Locale;
import java.util.TimeZone;
import org.ulteo.ovd.sm.Properties;
import org.ulteo.ovd.sm.SessionManagerCommunication;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.nfc.FormatException;
import android.nfc.NdefMessage;
import android.nfc.NdefRecord;
import android.nfc.NfcAdapter;
import android.os.Bundle;
import android.os.Parcelable;
import android.util.Log;
import android.view.View;

public class UriLoginActivity extends MainWindow {

	/*
	 * uri support
	 * ovd://user:pass@sm.domain?no_desktop=1&app_id=1&token=tk
	 */
	protected void parseOvdUri(Uri data) {
		if (Config.DEBUG)
			Log.d(Config.TAG, "Called with uri : " + data);

		int port = data.getPort();
		if (port < 0) // use default port for sm connection
			port = SessionManagerCommunication.DEFAULT_PORT;

		smHandler.connect(data.getHost(), port, true);
		smHandler.listenSm();
		progressDialog = ProgressDialog.show(this, getString(R.string.loading_open_virtual_desktop),
				getString(R.string.waiting_server_for_session),
				true, true, new DialogInterface.OnCancelListener() {
					@Override
					public void onCancel(DialogInterface dialog) {
						Log.i(Config.TAG, "Cancelling connection");
						smHandler.cancelLogin();
						UriLoginActivity.this.finish();
					}
				});
		// start thread for sm connection
		Properties request = new Properties(Properties.MODE_DESKTOP);
		// set language and timezone for sm request
		request.setLang(Locale.getDefault().getLanguage());
		request.setTimeZone(TimeZone.getDefault().getID());

		String param;
		param = data.getQueryParameter("no_desktop");
		if (param != null && param.equals("1"))
			request.setShowDesktop(false);

		param = data.getQueryParameter("app_id");
		if (param != null)
			request.setApplicationId(Integer.valueOf(param));

		param = data.getEncodedUserInfo();
		if (param != null) {
			String[] userInfo = param.split(":", 2);
			request.setLogin(userInfo[0]);
			request.setPassword(userInfo[1]);
		}

		param = data.getQueryParameter("token");
		if (param != null)
			request.setToken(param);

		smHandler.login(request);
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		// Remove all controls of this view
		View a = findViewById(R.id.main);
		a.setVisibility(View.GONE);

		Intent intent = getIntent();
		if (Intent.ACTION_VIEW.equals(intent.getAction())) {
			parseOvdUri(intent.getData());
		} else if (NfcAdapter.ACTION_NDEF_DISCOVERED.equals(getIntent().getAction())) {
			Parcelable[] rawMsgs = intent.getParcelableArrayExtra(NfcAdapter.EXTRA_NDEF_MESSAGES);
			if (rawMsgs != null) {
				for (int i = 0; i < rawMsgs.length; i++) {
					NdefMessage msg = (NdefMessage) rawMsgs[i];
					for (NdefRecord record : msg.getRecords()) {
						Uri uri = parseUri(record);
						if (uri != null) {
							parseOvdUri(uri);
							break;
						}
					}
				}
			}
		}
	}

	private Uri parseUri(NdefRecord record) {
		int tnf = record.getTnf();
		if (tnf == NdefRecord.TNF_ABSOLUTE_URI) {
			return Uri.parse(new String(record.getType()));
		}
		if (tnf == NdefRecord.TNF_WELL_KNOWN && Arrays.equals(NdefRecord.RTD_URI, record.getType())) {
			byte[] payload = record.getPayload();
			String uriStr = new String(payload, 1, payload.length - 1);
			return Uri.parse(uriStr);
		}

		if (tnf == NdefRecord.TNF_WELL_KNOWN && Arrays.equals(NdefRecord.RTD_SMART_POSTER, record.getType())) {
			NdefMessage n;
			try {
				n = new NdefMessage(record.getPayload());
			} catch (FormatException e) {
				return null; // not an NDEF message, we're done
			}

			for (NdefRecord r : n.getRecords()) {
				Uri uri = parseUri(r);
				if (uri != null)
					return uri;
			}

		}
		return null;
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);
		finish();
	}

	@Override
	protected void OnEndSmLogin() {
		super.OnEndSmLogin();
		finish();
	}

}
