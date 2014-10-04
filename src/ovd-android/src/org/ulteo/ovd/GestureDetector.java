/*
 * Copyright (C) 2013 Ulteo SAS
 * http://www.ulteo.com
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

import android.annotation.TargetApi;
import android.content.Context;
import android.graphics.PointF;
import android.os.Handler;
import android.view.MotionEvent;
import android.view.ViewConfiguration;

public class GestureDetector {

	public interface OnGestureListener {

		void onTwoFingersTap();

		void onThreeFingersTap();

		void onTouchUp(PointF pos);

		void onTap(PointF pos);

		void onTouchDown(PointF pos);

		void onTouchMove(PointF pos);

		void onLongTap(PointF pos);

		void onTwoFingersDown(PointF pos0, PointF pos1);

		void onTwoFingersMove(PointF pos0, PointF pos1);

		void onTwoFingersUp(PointF pos0, PointF pos1);

	}

	private float mDragTreshold;
	private float mClickMoveTreshold;
	private float mZoomTreshold;
	private int mDoubleTapTime;
	private int mLongPressTimeout;
	private int mTapTimeout;
	private int mMultiFingersTimeout;

	private enum Modes {
		NONE, ONE_FINGER, TWO_FINGERS, THREE_FINGERS, ZOOMDRAG, DRAG
	};

	private Modes mode = Modes.NONE;

	private PointF mid1 = new PointF();
	private boolean doubleClick = false;
	private float last_touch_time;
	private PointF first_touch = new PointF();
	private PointF last_touch = new PointF();
	private float pixels_moved = 0;
	private PointF mid = new PointF();
	private float oldDist = 1f;
	private Handler clickHandler;
	private OnGestureListener mListener;

	public GestureDetector(Context context, OnGestureListener listener) {
		mListener = listener;
		clickHandler = new Handler();

		final ViewConfiguration configuration = ViewConfiguration.get(context);
		mDragTreshold = configuration.getScaledTouchSlop();
		mClickMoveTreshold = configuration.getScaledTouchSlop();
		mZoomTreshold = configuration.getScaledTouchSlop();
		mDoubleTapTime = ViewConfiguration.getDoubleTapTimeout();
		mTapTimeout = ViewConfiguration.getTapTimeout();
		mLongPressTimeout = ViewConfiguration.getLongPressTimeout();
		mMultiFingersTimeout = ViewConfiguration.getJumpTapTimeout();
	}

	@TargetApi(14)
	public boolean onTouchEvent(MotionEvent event) {
		switch (event.getActionMasked()) {
		case MotionEvent.ACTION_DOWN:
		case MotionEvent.ACTION_POINTER_DOWN:
			onTouchDown(event);
			break;
		case MotionEvent.ACTION_UP:
		case MotionEvent.ACTION_POINTER_UP:
			onTouchUp(event);
			break;
		case MotionEvent.ACTION_MOVE:
			onTouchMove(event);
			break;
		}
		return true;
	}

	private void onTouchMove(MotionEvent event) {
		PointF mid2 = new PointF();
		int nbPointers = event.getPointerCount();
		PointF pos = new PointF();
		PointF pos1 = new PointF();
		PointF delta = new PointF();
		float touchInitialTime = event.getDownTime();
		float touchCurrentTime = event.getEventTime();
		float touchTimer = touchCurrentTime - touchInitialTime;

		if (mode == Modes.THREE_FINGERS) {
			return;
		}

		delta.x = event.getX() - last_touch.x;
		delta.y = event.getY() - last_touch.y;

		// Track pixels moved
		pixels_moved += delta.length();

		pos.x = event.getX();
		pos.y = event.getY();

		if ((nbPointers == 1 && pixels_moved > mClickMoveTreshold && mode == Modes.ONE_FINGER && touchTimer > mTapTimeout) || mode == Modes.DRAG) {
			last_touch.x = event.getX();
			last_touch.y = event.getY();
			if (mode == Modes.ONE_FINGER) {
				mListener.onTouchDown(first_touch);
				mode = Modes.DRAG;
			}
			mListener.onTouchMove(pos);
		} else if (nbPointers == 2) {
			pos1.x = event.getX(1);
			pos1.y = event.getY(1);
			float newDist = spacing(event);

			midPoint(mid2, event);
			if (mode == Modes.TWO_FINGERS
					&& ((Math.abs(newDist - oldDist) > mZoomTreshold) || (mid.x - mid2.x > mDragTreshold
							|| mid.y - mid2.y > mDragTreshold || mid.x - mid2.x < -mDragTreshold || mid.y - mid2.y < -mDragTreshold))) {
				mListener.onTwoFingersDown(pos, pos1);
				mode = Modes.ZOOMDRAG;
			}

			if (mode == Modes.ZOOMDRAG) {
				mListener.onTwoFingersMove(pos, pos1);
			}

		}
	}

	private void onTouchDown(MotionEvent event) {
		int nbPointers = event.getPointerCount();
		pixels_moved = 0;
		if (nbPointers > 1) {
			oldDist = spacing(event);
			if (oldDist > mDragTreshold) {
				midPoint(mid, event);
			}
			if (mode == Modes.ONE_FINGER) {
				midPoint(mid1, event);
				mode = Modes.TWO_FINGERS;
			}
			if (nbPointers == 3 && mode == Modes.TWO_FINGERS) {
				mode = Modes.THREE_FINGERS;
			}
			clickHandler.removeCallbacks(rightClick);
		} else {
			last_touch.x = event.getX();
			last_touch.y = event.getY();
			float touchInitialTime = event.getDownTime();
			if (touchInitialTime - last_touch_time > mDoubleTapTime) {
				first_touch.x = (int) event.getX();
				first_touch.y = (int) event.getY();
				doubleClick = false;
			} else {
				doubleClick = true;
			}
			last_touch_time = touchInitialTime;
			mode = Modes.ONE_FINGER;
			clickHandler.removeCallbacks(rightClick);
			clickHandler.postDelayed(rightClick, mLongPressTimeout);
		}
	}

	private void onTouchUp(MotionEvent event) {
		float touchInitialTime = event.getDownTime();
		float touchCurrentTime = event.getEventTime();
		float touchTimer = touchCurrentTime - touchInitialTime;
		PointF pos = new PointF();
		PointF pos1 = new PointF();

		pos.x = (int) event.getX();
		pos.y = (int) event.getY();
		clickHandler.removeCallbacks(rightClick);

		int nbPointers = event.getPointerCount();
		if (nbPointers > 1) {
			pos1.x = (int) event.getX(1);
			pos1.y = (int) event.getY(1);
			if (mode == Modes.ZOOMDRAG) {
				mListener.onTwoFingersUp(pos, pos1);
			}
		} else {
			touchInitialTime = 0;
			if (mode == Modes.ONE_FINGER && touchTimer < mLongPressTimeout) {
				if (doubleClick) {
					pos.x = first_touch.x;
					pos.y = first_touch.y;
				}
				mListener.onTap(pos);
			} else if (mode == Modes.DRAG) {
				mListener.onTouchUp(pos);
			}
			if (mode == Modes.THREE_FINGERS && touchTimer < mMultiFingersTimeout) {
				mListener.onThreeFingersTap();
			}
			if (mode == Modes.TWO_FINGERS && touchTimer < mMultiFingersTimeout) {
				mListener.onTwoFingersTap();
			}
			mode = Modes.NONE;
		}
	}

	private Runnable rightClick = new Runnable() {
		@Override
		public void run() {
			if (pixels_moved < mClickMoveTreshold) {
				mListener.onLongTap(first_touch);
			}

		}
	};

	// space between two fingers
	private float spacing(MotionEvent event) {
		float x = event.getX(0) - event.getX(1);
		float y = event.getY(0) - event.getY(1);
		return (float) Math.sqrt(x * x + y * y);
	}

	// mid point two fingers
	private void midPoint(PointF point, MotionEvent event) {
		point.x = (event.getX(0) + event.getX(1)) / 2;
		point.y = (event.getY(0) + event.getY(1)) / 2;
	}

}
