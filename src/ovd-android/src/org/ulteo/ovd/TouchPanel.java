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

import org.ulteo.ovd.GestureDetector.OnGestureListener;
import android.annotation.TargetApi;
import android.content.Context;
import android.graphics.Matrix;
import android.graphics.Point;
import android.graphics.PointF;
import android.util.Log;
import android.view.InputDevice;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewConfiguration;

public abstract class TouchPanel extends View implements OnGestureListener {
	private static final int ANDROID_BUTTON_MIDDLE = 0x40000000;

	private int mWheelTreshold;
	private float mDragTreshold;
	private float mZoomTreshold;
	private int bitmapWidth = 0, bitmapHeight = 0;
	private int displayWidth = 0, displayHeight = 0;
	private Matrix matrix = new Matrix();
	private Matrix savedMatrix = new Matrix();
	private Matrix savedMatrixZoom = new Matrix();
	private float lastWheel;

	private enum Modes {
		NONE, ZOOMDRAG, WHEEL
	};

	public Modes mode = Modes.NONE;
	private PointF mid = new PointF();
	private float oldDist = 1f;
	private int oldMouseButtons = 0;

	private GestureDetector mGestureDetector;

	abstract public void onTwoFingersTap();

	abstract public void onThreeFingersTap();

	abstract protected void onClickDown(int x, int y, int button);

	abstract protected void onClickUp(int x, int y, int button);

	abstract protected void onClickMove(int x, int y);

	public TouchPanel(Context context) {
		super(context);
		final ViewConfiguration configuration = ViewConfiguration.get(context);
		mWheelTreshold = configuration.getScaledTouchSlop();
		mDragTreshold = configuration.getScaledTouchSlop();
		mZoomTreshold = configuration.getScaledTouchSlop();
		mGestureDetector = new GestureDetector(getContext(), this);
	}

	@Override
	@TargetApi(14)
	public boolean onTouchEvent(MotionEvent event) {
		if (Config.DEBUG)
			Log.i(Config.TAG, "TouchPanel.onTouchEvent " + event);

		// Disable this event if it is a mouse
		if (event.getDeviceId() >= 0) {
			InputDevice dev = InputDevice.getDevice(event.getDeviceId());
			if (dev.getSources() == InputDevice.SOURCE_MOUSE || dev.getSources() == InputDevice.SOURCE_TOUCHPAD) {
				Point pos = currentPosition(event.getX(), event.getY());
				switch (event.getActionMasked()) {
				case MotionEvent.ACTION_MOVE:
					onClickMove(pos.x, pos.y);
					break;
				case MotionEvent.ACTION_DOWN:
					if (android.os.Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.ICE_CREAM_SANDWICH)
						break;
					if (event.getToolType(0) == MotionEvent.TOOL_TYPE_FINGER)
						onClickDown(pos.x, pos.y, Rdp.BUTTON_LEFT);
					break;
				case MotionEvent.ACTION_UP:
					if (android.os.Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.ICE_CREAM_SANDWICH)
						break;
					if (event.getToolType(0) == MotionEvent.TOOL_TYPE_FINGER)
						onClickUp(pos.x, pos.y, Rdp.BUTTON_LEFT);
					break;
				}
				return true;
			}
		}

		return mGestureDetector.onTouchEvent(event);
	}

	@TargetApi(14)
	@Override
	public boolean onGenericMotionEvent(MotionEvent event) {
		if (Config.DEBUG) {
			Log.d(Config.TAG, "onGenericMotionEvent " + event);
		}
		switch (event.getActionMasked()) {
		case MotionEvent.ACTION_HOVER_MOVE:
		case MotionEvent.ACTION_HOVER_EXIT:
		case MotionEvent.ACTION_HOVER_ENTER:
			if (android.os.Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.ICE_CREAM_SANDWICH)
				break;
			int mouseButtons = event.getButtonState();
			Point pos = currentPosition(event.getX(), event.getY());
			if (oldMouseButtons != mouseButtons) {
				if ((mouseButtons & MotionEvent.BUTTON_PRIMARY) != 0
						&& (oldMouseButtons & MotionEvent.BUTTON_PRIMARY) == 0) {
					onClickDown(pos.x, pos.y, Rdp.BUTTON_LEFT);
				} else if ((mouseButtons & MotionEvent.BUTTON_PRIMARY) == 0
						&& (oldMouseButtons & MotionEvent.BUTTON_PRIMARY) != 0) {
					onClickUp(pos.x, pos.y, Rdp.BUTTON_LEFT);
				}
				if ((mouseButtons & MotionEvent.BUTTON_SECONDARY) != 0
						&& (oldMouseButtons & MotionEvent.BUTTON_SECONDARY) == 0) {
					onClickDown(pos.x, pos.y, Rdp.BUTTON_RIGHT);
				} else if ((mouseButtons & MotionEvent.BUTTON_SECONDARY) == 0
						&& (oldMouseButtons & MotionEvent.BUTTON_SECONDARY) != 0) {
					onClickUp(pos.x, pos.y, Rdp.BUTTON_RIGHT);
				}
				if ((mouseButtons & MotionEvent.BUTTON_BACK) != 0 && (oldMouseButtons & MotionEvent.BUTTON_BACK) == 0) {
					onClickDown(pos.x, pos.y, Rdp.BUTTON_RIGHT);
				} else if ((mouseButtons & MotionEvent.BUTTON_BACK) == 0
						&& (oldMouseButtons & MotionEvent.BUTTON_BACK) != 0) {
					onClickUp(pos.x, pos.y, Rdp.BUTTON_RIGHT);
				}
				if ((mouseButtons & ANDROID_BUTTON_MIDDLE) != 0 && (oldMouseButtons & ANDROID_BUTTON_MIDDLE) == 0) {
					onClickDown(pos.x, pos.y, Rdp.BUTTON_MIDDLE);
				} else if ((mouseButtons & ANDROID_BUTTON_MIDDLE) == 0
						&& (oldMouseButtons & ANDROID_BUTTON_MIDDLE) != 0) {
					onClickUp(pos.x, pos.y, Rdp.BUTTON_MIDDLE);
				}
				if ((mouseButtons & MotionEvent.BUTTON_TERTIARY) != 0
						&& (oldMouseButtons & MotionEvent.BUTTON_TERTIARY) == 0) {
					onClickDown(pos.x, pos.y, Rdp.BUTTON_MIDDLE);
				} else if ((mouseButtons & MotionEvent.BUTTON_TERTIARY) == 0
						&& (oldMouseButtons & MotionEvent.BUTTON_TERTIARY) != 0) {
					onClickUp(pos.x, pos.y, Rdp.BUTTON_MIDDLE);
				}
				oldMouseButtons = mouseButtons;
			} else {
				onClickMove(pos.x, pos.y);
			}
			break;
		case MotionEvent.ACTION_SCROLL:
			if (android.os.Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.HONEYCOMB_MR1)
				break;

			int vscroll = (int) Math.ceil(event.getAxisValue(MotionEvent.AXIS_VSCROLL));
			while (vscroll > 0) {
				onClickDown(0, 0, Rdp.WHEEL_UP);
				vscroll--;
			}
			while (vscroll < 0) {
				onClickDown(0, 0, Rdp.WHEEL_DOWN);
				vscroll++;
			}
			break;
		}
		return true;
	}

	protected void onClickLeft(int x, int y) {
		onClickDown(x, y, Rdp.BUTTON_LEFT);
		try {
			Thread.sleep(200);
		} catch (InterruptedException e) {}
		onClickUp(x, y, Rdp.BUTTON_LEFT);
	}

	protected void onClickRight(int x, int y) {
		onClickDown(x, y, Rdp.BUTTON_RIGHT);
		try {
			Thread.sleep(200);
		} catch (InterruptedException e) {}
		onClickUp(x, y, Rdp.BUTTON_RIGHT);
	}

	protected boolean isDraggable() {
		float minZoom = Math.min(displayWidth / (float) bitmapWidth, displayHeight / (float) bitmapHeight);
		float[] value = new float[9];
		matrix.getValues(value);

		return value[Matrix.MSCALE_X] > minZoom;
	}

	// rearrange the matrix
	protected void matrixTuning() {
		float[] value = new float[9];
		matrix.getValues(value);

		if (value[Matrix.MSCALE_X] > Config.MAX_ZOOM) {
			float[] savedValue = new float[9];
			savedMatrixZoom.getValues(savedValue);
			value[Matrix.MSCALE_X] = savedValue[Matrix.MSCALE_X];
			value[Matrix.MSCALE_Y] = savedValue[Matrix.MSCALE_Y];
			value[Matrix.MTRANS_X] = savedValue[Matrix.MTRANS_X];
			value[Matrix.MTRANS_Y] = savedValue[Matrix.MTRANS_Y];
		}

		float minZoom = Math.min(displayWidth / (float) bitmapWidth, displayHeight / (float) bitmapHeight);

		if (value[Matrix.MSCALE_X] < minZoom) {
			value[Matrix.MSCALE_X] = minZoom;
			value[Matrix.MSCALE_Y] = minZoom;
		}

		if (value[Matrix.MTRANS_X] < displayWidth - bitmapWidth * value[Matrix.MSCALE_X])
			value[Matrix.MTRANS_X] = displayWidth - bitmapWidth * value[Matrix.MSCALE_X];

		if (value[Matrix.MTRANS_Y] < displayHeight - bitmapHeight * value[Matrix.MSCALE_X])
			value[Matrix.MTRANS_Y] = displayHeight - bitmapHeight * value[Matrix.MSCALE_X];

		if (value[Matrix.MTRANS_X] > 0)
			value[Matrix.MTRANS_X] = 0;

		if (value[Matrix.MTRANS_Y] > 0)
			value[Matrix.MTRANS_Y] = 0;

		matrix.setValues(value);
		savedMatrixZoom.set(matrix);
	}

	public void setBitmapSize(int width, int height) {
		this.bitmapWidth = width;
		this.bitmapHeight = height;
		matrix.reset();
		matrixTuning();
	}

	public void setDisplaySize(int width, int height) {
		this.displayWidth = width;
		this.displayHeight = height;
		matrixTuning();
	}

	public Matrix getTransformMatrix() {
		return matrix;
	}

	private Point currentPosition(float x, float y) {
		float[] value = new float[9];
		Point ret = new Point();
		matrix.getValues(value);
		ret.x = (int) ((x - value[Matrix.MTRANS_X]) / value[Matrix.MSCALE_X]);
		ret.y = (int) ((y - value[Matrix.MTRANS_Y]) / value[Matrix.MSCALE_Y]);
		return ret;
	}

	private Point currentPosition(PointF pos) {
		return currentPosition(pos.x, pos.y);
	}

	// distance between two points
	private float distance(PointF pos0, PointF pos1) {
		float x = pos0.x - pos1.x;
		float y = pos0.y - pos1.y;
		return (float) Math.sqrt(x * x + y * y);
	}

	// mid point two fingers
	private PointF middle(PointF pos0, PointF pos1) {
		float[] ret = new float[2];
		ret[0] = (pos0.x + pos1.x) / 2;
		ret[1] = (pos0.y + pos1.y) / 2;
		// matrix.mapPoints(ret);
		return new PointF(ret[0], ret[1]);
	}

	@Override
	public void onTap(PointF pos) {
		Point curpos = currentPosition(pos);
		onClickLeft(curpos.x, curpos.y);
	}

	@Override
	public void onLongTap(PointF pos) {
		Point curpos = currentPosition(pos);
		onClickRight(curpos.x, curpos.y);
	}

	@Override
	public void onTouchDown(PointF pos) {
		Point curpos = currentPosition(pos);
		onClickDown(curpos.x, curpos.y, Rdp.BUTTON_LEFT);
	}

	@Override
	public void onTouchMove(PointF pos) {
		Point curpos = currentPosition(pos);
		onClickMove(curpos.x, curpos.y);
	}

	@Override
	public void onTouchUp(PointF pos) {
		Point curpos = currentPosition(pos);
		onClickUp(curpos.x, curpos.y, Rdp.BUTTON_LEFT);
	}

	@Override
	public void onTwoFingersDown(PointF pos0, PointF pos1) {
		mode = Modes.NONE;
		savedMatrix.set(matrix);
		oldDist = distance(pos0, pos1);
		mid = middle(pos0, pos1);
	}

	@Override
	public void onTwoFingersMove(PointF pos0, PointF pos1) {
		float[] value = new float[9];
		float newDist = distance(pos0, pos1);
		float scale = newDist / oldDist;
		PointF delta = new PointF();

		matrix.set(savedMatrix);
		PointF mid2 = middle(pos0, pos1);
		Point pos = currentPosition(pos0);

		if (mode == Modes.NONE && (Math.abs(newDist - oldDist) > mZoomTreshold)) {
			mode = Modes.ZOOMDRAG;
		}

		if (mode == Modes.NONE
				&& (mid.x - mid2.x > mDragTreshold || mid.y - mid2.y > mDragTreshold || mid.x - mid2.x < -mDragTreshold || mid.y
						- mid2.y < -mDragTreshold))
			if (isDraggable()) {
				mode = Modes.ZOOMDRAG;
			}

		if (mode == Modes.ZOOMDRAG) {
			matrix.getValues(value);
			if (value[Matrix.MSCALE_X] * scale > Config.MAX_ZOOM) {
				scale = Config.MAX_ZOOM / value[Matrix.MSCALE_X];
			}
			matrix.postScale(scale, scale, mid.x, mid.y);

			matrix.getValues(value);
			delta.x = (mid2.x - mid.x) / value[Matrix.MSCALE_X];
			delta.y = (mid2.y - mid.y) / value[Matrix.MSCALE_X];
			matrix.preTranslate(delta.x, delta.y);

			matrixTuning();
			invalidate();
		}

		if (mode == Modes.NONE) {
			if (Math.abs(mid.x - mid2.x) < mDragTreshold && Math.abs(mid.y - mid2.y) > mDragTreshold) {
				mode = Modes.WHEEL;
				lastWheel = mid.y - mid2.y;
				// move the cursor at first finger position
				onClickMove(pos.x, pos.y);
			}
		}
		if (mode == Modes.WHEEL) {
			float curWheel = mid.y - mid2.y;
			if (Math.abs(curWheel - lastWheel) > mWheelTreshold) {
				if (curWheel - lastWheel > 0) {
					onClickDown(pos.x, pos.y, Rdp.WHEEL_DOWN);
				} else {
					onClickDown(pos.x, pos.y, Rdp.WHEEL_UP);
				}
				lastWheel = curWheel;
			}
		}
	}

	@Override
	public void onTwoFingersUp(PointF pos0, PointF pos1) {
		mode = Modes.NONE;
	}

}
