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

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.CharBuffer;
import java.nio.IntBuffer;
import java.nio.charset.Charset;
import java.util.ArrayList;
import org.ulteo.ovd.sm.Properties;
import android.graphics.Bitmap;
import android.graphics.Point;
import android.os.Environment;
import android.util.Log;
import android.view.KeyEvent;

public class Rdp {
	public static final int BUTTON_LEFT = 1;
	public static final int BUTTON_MIDDLE = 2;
	public static final int BUTTON_RIGHT = 3;
	public static final int WHEEL_UP = 4;
	public static final int WHEEL_DOWN = 5;

	private static final String UNICODE_TYPE = "UTF-16LE";
	private boolean loggedin = false;

	public interface RdpDraw {
		public void rdpDraw(int x, int y, int w, int h);

		public void rdpUpdateSize(int width, int height, Bitmap buffer);

		public void onRdpClose(int why);

		public void onUpdateClipboard(String data);
		
		public void onPrintJob(String filename);
	}

	private Charset csets = Charset.forName(UNICODE_TYPE);
	private boolean shift_on = false;
	private boolean alt_on = false;
	private boolean ctrl_on = false;
	private Bitmap buffer;
	private RdpDraw rdpDraw;
	private int mDrives;
	private int lastExitCode = 0;

	// load freerdp and andrdp libs
	static {
		System.loadLibrary("crypto");
		System.loadLibrary("ssl");
		System.loadLibrary("freerdp");
		System.loadLibrary("cliprdr");
		System.loadLibrary("rdpdr");
		System.loadLibrary("ukbrdr");
		System.loadLibrary("disk");
		System.loadLibrary("rdpdr_pdf");
		System.loadLibrary("rdpsnd");
		System.loadLibrary("rdpsnd_sles");
		System.loadLibrary("andrdp");
	}

	// native function to start rdp connection and give the backbuffer to the
	// native code
	private native int rdp(int width, int height, String username, String password, String ip, int port,
			boolean use_gateway, String token, String rdpshell, boolean bulk_compress, int compression_type);

	public native int sendUnicode(int unicode);

	private native int sendScancode(int up, int extended, int keyCode);

	public native int sendVkcode(int up, int vkeyCode);

	public native int sendImePreeditString(String data);

	public native int sendImePreeditStringStop();

	public native int sendClipboard(String data);

	private native int stop();

	public native int clickDown(int x, int y, int button);

	public native int clickUp(int x, int y, int button);

	public native int clickMove(int x, int y);

	public Rdp(Point res, String username, String password, String ip, int port, boolean use_gateway, String token,
			int drives, String rdpshell, boolean bulk_compress, int compression_type) {
		mDrives = drives;
		rdp(res.x, res.y, username, password, ip, port, use_gateway, token, rdpshell, bulk_compress, compression_type);
		loggedin = true;
	}

	/**
	 * manage sending of specials chars that needs multiple touch to be done
	 */
	public boolean dispatchKeyEvent(KeyEvent event) {
		if (event.getAction() == KeyEvent.ACTION_MULTIPLE) {
			int key = 0;
			// convert utf8 to utf16-le for windows
			String schars = event.getCharacters();
			if (schars == null)
				return false;

			ByteBuffer chars = csets.encode(schars);
			chars.order(ByteOrder.LITTLE_ENDIAN);
			for (int i = 0; i < chars.capacity(); i = i + 2) {
				key = chars.getShort();
				if (Config.DEBUG)
					Log.d(Config.TAG, "Unicode2 : " + Integer.toHexString(key) + " " + key);
				sendUnicode(key);
			}

		} else if (event.getAction() == KeyEvent.ACTION_DOWN) {
			return onKeyDown(event.getKeyCode(), event);
		} else if (event.getAction() == KeyEvent.ACTION_UP) {
			return onKeyUp(event.getKeyCode(), event);
		}

		return false;
	}

	private boolean processMetaKeys(int vkeycode, int keyCode, KeyEvent event) {
		if (Config.DEBUG)
			Log.d(Config.TAG, "metakey : " + Integer.toHexString(event.getScanCode()) + " alt=" + alt_on + " shift=" + shift_on
					+ " ctrl=" + ctrl_on);

		if (alt_on)
			sendScancode(0, 0, 0x38);

		if (ctrl_on)
			sendScancode(0, 0, 0x1D);

		if (shift_on)
			sendScancode(0, 0, 0x2A);

		if (vkeycode > 0) {
			sendVkcode(0, vkeycode);
			sendVkcode(1, vkeycode);
		} else {
			/* Try physical keyboard ScanCode. Not Working with virtual keyboards ! */
			sendScancode(0, 0, event.getScanCode()); 
			sendScancode(1, 0, event.getScanCode());
		}

		if (alt_on)
			sendScancode(1, 0, 0x38);

		if (ctrl_on)
			sendScancode(1, 0, 0x1D);

		if (shift_on)
			sendScancode(1, 0, 0x2A);

		return true;
	}

	/*
	 * send unicode to the server, and send special chars like enter or delete
	 * (non-Javadoc)
	 * @see android.app.Activity#onKeyDown(int, android.view.KeyEvent)
	 */
	private boolean onKeyDown(int keyCode, KeyEvent event) {
		int unicode = event.getUnicodeChar();
		int vkeycode = VKCodes.KeyCodeToVKeyCode(keyCode);
		if (Config.DEBUG)
			Log.d(Config.TAG, "onKeyDown = keycode=" + keyCode + " vkeycode=" + vkeycode + " unicode=" + unicode + " alt=" + alt_on + " shift=" + shift_on + " ctrl=" + ctrl_on);

		switch (keyCode) {
		case KeyEvent.KEYCODE_SHIFT_LEFT:
		case KeyEvent.KEYCODE_SHIFT_RIGHT:
			shift_on = true;
			break;
		case KeyEvent.KEYCODE_ALT_LEFT:
		case KeyEvent.KEYCODE_ALT_RIGHT:
			alt_on = true;
			break;
		case KeyEvent.KEYCODE_CTRL_LEFT:
		case KeyEvent.KEYCODE_CTRL_RIGHT:
			ctrl_on = true;
			break;
		default: // default chars sending
			if (unicode <= 0x20) {
				if (alt_on || ctrl_on)
						return processMetaKeys(vkeycode, keyCode, event);

				if (vkeycode > 0) {
					sendVkcode(0, vkeycode);
				} else {
					/* Try physical keyboard ScanCode. Not Working with virtual keyboards ! */
					sendScancode(0, 0, event.getScanCode());
				}
				return true;
			}

			ByteBuffer bBuff = ByteBuffer.allocate(4);
			IntBuffer iBuff = bBuff.asIntBuffer();
			CharBuffer cBuff = bBuff.asCharBuffer();
			iBuff.put(unicode);
			// convert ut8 to utf16-le for windows
			ByteBuffer chars = csets.encode(cBuff);
			chars.order(ByteOrder.LITTLE_ENDIAN);
			int key = chars.getShort(2);
			if (Config.DEBUG)
				Log.d(Config.TAG, "Unicode1 : " + Integer.toHexString(key));
			sendUnicode(key);
			break;
		}
		return true;
	}

	private boolean onKeyUp(int keyCode, KeyEvent event) {
		int unicode = event.getUnicodeChar();
		int vkeycode = VKCodes.KeyCodeToVKeyCode(keyCode);
		if (Config.DEBUG)
			Log.d(Config.TAG, "onKeyUp = keycode=" + keyCode + " vkeycode=" + vkeycode + " unicode=" + unicode + " alt=" + alt_on + " shift=" + shift_on + " ctrl=" + ctrl_on);

		switch (keyCode) {
		case KeyEvent.KEYCODE_SHIFT_LEFT:
		case KeyEvent.KEYCODE_SHIFT_RIGHT:
			shift_on = false;
			break;
		case KeyEvent.KEYCODE_ALT_LEFT:
		case KeyEvent.KEYCODE_ALT_RIGHT:
			alt_on = false;
			break;
		case KeyEvent.KEYCODE_CTRL_LEFT:
		case KeyEvent.KEYCODE_CTRL_RIGHT:
			ctrl_on = false;
			break;
		default:
			if (unicode <= 0x20) {
				if (alt_on || ctrl_on)
					return true;
				
				if (vkeycode > 0) {
					sendVkcode(1, vkeycode);
				} else {
					/* Try physical keyboard ScanCode. Not Working with virtual keyboards ! */
					sendScancode(1, 0, event.getScanCode());
				}
			}
		}
		return true;
	}
	
	public void draw() {
		if (buffer != null)
			draw(0, 0, buffer.getWidth(), buffer.getHeight());
	}

	/*
	 * Called from native library
	 */
	public void draw(int x, int y, int w, int h) {
		if (rdpDraw != null)
			rdpDraw.rdpDraw(x, y, w, h);
	}

	/*
	 * Called from native library
	 */
	public Bitmap allocBuffer(int width, int height, int bpp) {
		Bitmap.Config color_config;
		if (bpp > 16)
			color_config = Bitmap.Config.ARGB_8888;
		else
			color_config = Bitmap.Config.RGB_565;

		buffer = Bitmap.createBitmap(width, height, color_config);
		if (rdpDraw != null)
			rdpDraw.rdpUpdateSize(width, height, buffer);
		return buffer;
	}

	/*
	 * Called from native library returns an array with disk name followed by
	 * disk path
	 */
	public String[] getDisks() {
		ArrayList<StorageObserver.StorageEntry> list;
		switch (mDrives) {
		case Properties.REDIRECT_DRIVES_FULL:
			list = StorageObserver.getStorageDirectories();
			break;
		case Properties.REDIRECT_DRIVES_PARTIAL:
			list = new ArrayList<StorageObserver.StorageEntry>();
			
			list.add(new StorageObserver.StorageEntry(Config.SDCARD, Environment.getExternalStorageDirectory().getPath()));
			break;
		default:
			list = new ArrayList<StorageObserver.StorageEntry>();
		}
		String[] disks = new String[list.size() * 2];
		for (int i = 0; i < list.size(); i++) {
			disks[i * 2] = list.get(i).name;
			disks[i * 2 + 1] = list.get(i).path;
		}
		return disks;
	}

	/*
	 * Called from native library returns the name of the device
	 */
	public String getClientName() {
		return android.os.Build.MODEL;
	}

	/*
	 * Called from native library to inform that the connection is closed.
	 */
	public void onRdpClose(int why) {
		lastExitCode  = why;
		Log.i(Config.TAG, "Rdp connection closed exit code = " + why);
		if (rdpDraw != null)
			rdpDraw.onRdpClose(why);
		loggedin = false;
	}

	public void logout() {
		if (loggedin) {
			Log.i(Config.TAG, "Rdp Logout");
			stop();
		}
		loggedin = false;
	}

	public void registerDraw(RdpDraw aRdpDraw) {
		rdpDraw = aRdpDraw;
		if (rdpDraw != null && buffer != null)
			rdpDraw.rdpUpdateSize(buffer.getWidth(), buffer.getHeight(), buffer);
	}

	public void unRegisterDraw() {
		rdpDraw = null;
	}

	public boolean isLoggedIn() {
		return loggedin;
	}

	public int getLastExitCode() {
		return lastExitCode;
	}

	/*
	 * Called from native library
	 */
	public void onUpdateClipboard(String data) {
		if (Config.DEBUG)
			Log.d(Config.TAG, "New Clipboard from RDP : " + data);

		if (rdpDraw != null)
			rdpDraw.onUpdateClipboard(data);
	}

	/*
	 * Called from native library
	 */
	public void onPrintJob(String filename) {
		if (Config.DEBUG)
			Log.d(Config.TAG, "New printed job from RDP : " + filename);

		if (rdpDraw != null)
			rdpDraw.onPrintJob(filename);
	}

}
