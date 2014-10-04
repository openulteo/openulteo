/*
 * Copyright (C) 2011-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author Pierre Laine <plaine@ulteo.com> 2011
 * Author Clément Bizeau <cbizeau@ulteo.com> 2011
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

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.List;
import android.annotation.TargetApi;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.ColorMatrixColorFilter;
import android.graphics.Paint;
import android.graphics.Rect;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.os.Handler;
import android.text.InputType;
import android.view.KeyEvent;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;
import android.widget.Toast;

/**
 * surface to draw rdp orders
 * 
 */
public class Panel extends TouchPanel implements Rdp.RdpDraw {
	private Rdp rdp;
	private Bitmap backbuffer = null;
	private Bitmap buffer = null;
	private Rect invRect = new Rect();

	private Handler notify;

	public interface OnThreeFingersHandler {
		public void OnThreeFingers(Panel panel);
	}

	public OnThreeFingersHandler threeFingersHandler = null;

	public Panel(Context context) {
		super(context);
		notify = new Handler();
		setFocusable(true);
		setFocusableInTouchMode(true);
		requestFocus();
	}

	/**
	 * refresh screen and apply matrix put black background for zooming back
	 */
	public void rdpDraw(int x, int y, int w, int h) {
		invRect.left = x;
		invRect.top = y;
		invRect.right = x + w;
		invRect.bottom = y + h;
		Canvas canvas = new Canvas(backbuffer);
		canvas.drawBitmap(buffer, invRect, invRect, null);
		postInvalidate();
	}

	protected void onDrawEmpty(Canvas canvas) {
		int w = canvas.getWidth();
		int h = canvas.getHeight();

		canvas.drawARGB(255, 50, 50, 50);

		Bitmap bm = BitmapFactory.decodeResource(getResources(), R.drawable.icon);
		int bmw = bm.getWidth();
		int bmh = bm.getHeight();
		int dx = (w - bmw * 2) / 2;
		int dy = (h - bmh * 2) / 2;

		Paint bp = new Paint();
		float[] matrix = {
			0.5f, 0.5f, 0.5f, 0.5f, 0,
			0.5f, 0.5f, 0.5f, 0.5f, 0,
			0.5f, 0.5f, 0.5f, 0.5f, 0,
			0, 0, 0, 0.2f, 0 //alpha
		};
		bp.setColorFilter(new ColorMatrixColorFilter(matrix));

		canvas.drawBitmap(bm, new Rect(0, 0, bmw, bmh), new Rect(dx, dy, dx + bmw * 2, dy + bmh * 2), bp);
	}

	@Override
	public void onDraw(Canvas canvas) {
		if (buffer != null) {
			canvas.drawBitmap(backbuffer, getTransformMatrix(), null);
		} else {
			onDrawEmpty(canvas);
		}
	}

	@Override
	protected void onWindowVisibilityChanged(int visibility) {
		super.onWindowVisibilityChanged(visibility);
		if (visibility == VISIBLE) {
			surfaceCreated();
		} else {
			surfaceDestroyed();
		}
	}

	@Override
	protected void onSizeChanged(int w, int h, int oldw, int oldh) {
		super.onSizeChanged(w, h, oldw, oldh);
		setDisplaySize(getMeasuredWidth(), getMeasuredHeight());
		if (AndRdpActivity.isLoggedIn())
			rdp.draw();
	}

	/*
	 * screen orientation changed (non-Javadoc)
	 * @see
	 * android.view.SurfaceHolder.Callback#surfaceChanged(android.view.SurfaceHolder
	 * , int, int, int)
	 * @Override public void surfaceChanged(SurfaceHolder holder, int format,
	 * int width, int height) { Configuration config =
	 * getResources().getConfiguration(); if (config.orientation ==
	 * Configuration.ORIENTATION_PORTRAIT) {
	 * mat.setWidthPortrait(getMeasuredWidth());
	 * mat.setHeightPortrait(getMeasuredHeight()); mat.setPortraitMode();
	 * rdp.draw(); } else { mat.setLandscapeMode(); rdp.draw(); } }
	 */
	@TargetApi(Build.VERSION_CODES.HONEYCOMB)
	public void surfaceCreated() {
		if (!AndRdpActivity.isConnected())
			return;

		rdp = AndRdpActivity.getRdp();
		if (rdp.isLoggedIn()) {
			setDisplaySize(getMeasuredWidth(), getMeasuredHeight());
			rdp.registerDraw(this);
			rdp.draw();
		} else {
			onRdpClose(rdp.getLastExitCode());
		}
	}

	public void surfaceDestroyed() {
		if (!AndRdpActivity.isConnected())
			return;

		rdp.unRegisterDraw();
	}

	@Override
	public void rdpUpdateSize(int width, int height, Bitmap aBuffer) {
		setBitmapSize(width, height);
		backbuffer = Bitmap.createBitmap(width, height, aBuffer.getConfig());
		buffer = aBuffer;
	}

	public void toggleKeyboard() {
		InputMethodManager mgr = (InputMethodManager) this.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
		mgr.toggleSoftInput(InputMethodManager.SHOW_FORCED, 0);

		/*
		Window win = ((AndRdpActivity) getContext()).getWindow();
		if (keyb == false) {
			mgr.showSoftInput(this.getRootView(), InputMethodManager.SHOW_IMPLICIT | InputMethodManager.SHOW_FORCED);
			win.addFlags(WindowManager.LayoutParams.FLAG_FORCE_NOT_FULLSCREEN);
			win.clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
			keyb = true;
		} else {
			mgr.hideSoftInputFromWindow(this.getWindowToken(), 0);
			win.addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
			win.clearFlags(WindowManager.LayoutParams.FLAG_FORCE_NOT_FULLSCREEN);
			keyb = false;
		}
		*/
	}

	@Override
	public void onTwoFingersTap() {
		toggleKeyboard();
		requestFocus();
	}

	@Override
	public void onThreeFingersTap() {
		if (threeFingersHandler != null)
			threeFingersHandler.OnThreeFingers(this);
	}

	@Override
	protected void onClickDown(int x, int y, int button) {
		if (AndRdpActivity.isLoggedIn())
			rdp.clickDown(x, y, button);
	}

	@Override
	protected void onClickUp(int x, int y, int button) {
		if (AndRdpActivity.isLoggedIn())
			rdp.clickUp(x, y, button);
	}

	@Override
	protected void onClickMove(int x, int y) {
		if (AndRdpActivity.isLoggedIn())
			rdp.clickMove(x, y);
	}

	@Override
	public void onRdpClose(int why) {
		if (why == AndRdpActivity.ERROR_RDP_CONNECT) {
			notify.post(new Runnable() {
				@Override
				public void run() {
					Toast.makeText(Panel.this.getContext(), R.string.error_connection, Toast.LENGTH_LONG).show();
				}
			});
		}
		AndRdpActivity.deleteRdp();
		((AndRdpActivity) getContext()).finish();
	}

	@Override
	public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
		boolean ime = Settings.getIme(getContext());
		outAttrs.inputType = ime ? InputType.TYPE_CLASS_TEXT : InputType.TYPE_NULL;
		return new RdpInputConnection() {

			@Override
			protected void sendVkcode(int up, int code) {
				if (AndRdpActivity.isLoggedIn())
					rdp.sendVkcode(up, code);
			}

			@Override
			protected void sendUnicode(int code) {
				if (AndRdpActivity.isLoggedIn())
					rdp.sendUnicode(code);
			}

			@Override
			public boolean sendKeyEvent(KeyEvent event) {
				if (AndRdpActivity.isLoggedIn())
					return rdp.dispatchKeyEvent(event);
				return false;
			}

			@Override
			public void sendImePreeditString(String data) {
				if (AndRdpActivity.isLoggedIn())
					rdp.sendImePreeditString(data);
			}

			@Override
			public void sendImePreeditStringStop() {
				if (AndRdpActivity.isLoggedIn())
					rdp.sendImePreeditStringStop();
			}
		};
	}

	@Override
	public boolean onCheckIsTextEditor() {
		return true;
	}

	@Override
	public void onUpdateClipboard(final String data) {
		notify.post(new Runnable() {
			@Override
			@TargetApi(Build.VERSION_CODES.HONEYCOMB)
			public void run() {
				ClipboardManager clipboard = (ClipboardManager) getContext()
						.getSystemService(Context.CLIPBOARD_SERVICE);
				ClipData clip = ClipData.newPlainText(getContext().getResources().getText(R.string.app_name), data);
				clipboard.setPrimaryClip(clip);
			}
		});
	}

	@Override
	public void onPrintJob(String filename) {
		File spooldir = new File(Environment.getExternalStorageDirectory().getAbsolutePath() + "/Android/data/" + getContext().getPackageName());
		spooldir.mkdirs();

		String sdcard = spooldir.getAbsolutePath() + "/" + (new File(filename)).getName();

		try {
			InputStream in = new FileInputStream(filename);
			OutputStream out = new FileOutputStream(sdcard);
			byte[] buf = new byte[1024];
			int len;
			while ((len = in.read(buf)) > 0) {
				out.write(buf, 0, len);
			}
			in.close();
			out.close();
			(new File(filename)).delete();
			filename = sdcard;
			(new File(filename)).deleteOnExit();
		} catch (FileNotFoundException e) {
		} catch (IOException e) {
		}

		Intent sharingIntent = new Intent(Intent.ACTION_VIEW);
		sharingIntent.setDataAndType(Uri.parse("file://" + filename), "application/pdf");
		PackageManager pm = getContext().getPackageManager();
		List<ResolveInfo> activities = pm.queryIntentActivities(sharingIntent, 0);
		
		if (activities.size() > 0) {
			getContext().startActivity(sharingIntent);
		} else {
			getContext().startActivity(Intent.createChooser(sharingIntent, "Print via"));
		}
		
		// TODO: Delete printed files
	}
}
