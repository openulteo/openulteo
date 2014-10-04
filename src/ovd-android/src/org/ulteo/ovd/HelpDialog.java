/*
 * Copyright (C) 2011-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author Pierre Laine <plaine@ulteo.com> 2011
 * Author Clément Bizeau <cbizeau@ulteo.com> 2011
 * Author David PHAM-VAN <d.pham-van@ulteo.com> 2013
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

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import android.app.Dialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.res.Resources;
import android.view.ViewGroup.LayoutParams;
import android.webkit.WebView;
import android.widget.LinearLayout;

public class HelpDialog {

	private static HashMap<String, String> getFields(Context context) {
		HashMap<String, String> map = new HashMap<String, String>();
		try {
			ComponentName comp = new ComponentName(context, context.getClass());
			PackageInfo pinfo = context.getPackageManager().getPackageInfo(comp.getPackageName(), 0);
			map.put("version", pinfo.versionName);
			map.put("version_code", String.valueOf(pinfo.versionCode));
		} catch (android.content.pm.PackageManager.NameNotFoundException e) {}
		map.put("copyright", Config.COPYRIGHT);
		map.put("website", Config.WEBSITE);
		if (Config.DEBUG)
			map.put("debug", "Debugging mode enabled");
		else
			map.put("debug", "");
		return map;
	}

	private static String readFully(InputStream inputStream) throws IOException {
		ByteArrayOutputStream baos = new ByteArrayOutputStream();
		byte[] buffer = new byte[1024];
		int length = 0;
		while ((length = inputStream.read(buffer)) != -1) {
			baos.write(buffer, 0, length);
		}
		return new String(baos.toByteArray());
	}

	private static Dialog setupHtmlDialog(Context context, int title, String template) {
		Dialog dialog = new Dialog(context);
		dialog.setTitle(title);
		WebView webView = new WebView(context);
		webView.getSettings().setJavaScriptEnabled(false);
		Resources res = context.getResources();
		String packageName = context.getPackageName();
		Pattern tag = Pattern.compile("\\{\\{(\\w+)\\}\\}");
		HashMap<String, String> fields = getFields(context);
		try {
			InputStream is = context.getAssets().open(template);
			String data = readFully(is);
			Matcher matcher = tag.matcher(data);
			while (matcher.find()) {
				String name = matcher.group(1);
				int rep_id = res.getIdentifier(name, "string", packageName);
				if (rep_id > 0) {
					String rep = res.getString(rep_id);
					data = data.replace(matcher.group(), rep);
				} else if (fields.containsKey(name)) {
					String rep = fields.get(name);
					data = data.replace(matcher.group(), rep);
				}
			}
			webView.loadDataWithBaseURL("file:///android_asset/" + template, data, "text/html", "UTF-8", null);
			dialog.addContentView(webView, new LinearLayout.LayoutParams(LayoutParams.MATCH_PARENT,
					LayoutParams.WRAP_CONTENT));
		} catch (IOException e) {}
		return dialog;
	}

	public static void showHelp(Context context) {
		Dialog helpDialog = setupHtmlDialog(context, R.string.Help, "help.html");
		helpDialog.show();
	}
}
