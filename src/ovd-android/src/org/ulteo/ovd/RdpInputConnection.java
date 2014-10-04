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

import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.inputmethod.CompletionInfo;
import android.view.inputmethod.CorrectionInfo;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.ExtractedText;
import android.view.inputmethod.ExtractedTextRequest;
import android.view.inputmethod.InputConnection;

abstract public class RdpInputConnection implements InputConnection {

	protected boolean composition;

	abstract protected void sendVkcode(int up, int code);

	abstract protected void sendUnicode(int code);

	abstract protected void sendImePreeditString(String data);

	abstract protected void sendImePreeditStringStop();

	@Override
	abstract public boolean sendKeyEvent(KeyEvent event);

	public RdpInputConnection() {
		composition = false;
	}

	private void sendText(String buffer) {
		if (Config.DEBUG) {
			Log.d(Config.TAG, "Send : " + buffer);
		}

		for (int i=0 ; i < buffer.length(); i++) {
			char c = buffer.charAt(i);
			switch (c) {
			case 0x09:
				sendVkcode(0, VKCodes.VK_TAB);
				sendVkcode(1, VKCodes.VK_TAB);
				break;
			case 0x0a:
				sendVkcode(0, VKCodes.VK_RETURN);
				sendVkcode(1, VKCodes.VK_RETURN);
				break;
			case 0x20:
				sendVkcode(0, VKCodes.VK_SPACE);
				sendVkcode(1, VKCodes.VK_SPACE);
				break;
			default:
				sendUnicode(c);
			}
		}
	}

	@Override
	public boolean beginBatchEdit() {
		if (Config.DEBUG) {
			Log.w(Config.TAG, "beginBatchEdit");
		}
		return true;
	}

	@Override
	public boolean clearMetaKeyStates(int states) {
		if (Config.DEBUG) {
			Log.w(Config.TAG, "clearMetaKeyStates " + states);
		}
		return false;
	}

	@Override
	public boolean commitCompletion(CompletionInfo text) {
		if (Config.DEBUG) {
			Log.w(Config.TAG, "commitCompletion " + text);
		}
		return false;
	}

	@Override
	public boolean commitCorrection(CorrectionInfo correctionInfo) {
		if (Config.DEBUG) {
			Log.w(Config.TAG, "commitCorrection " + correctionInfo);
		}
		return true;
	}

	@Override
	public boolean commitText(CharSequence text, int newCursorPosition) {
		if (Config.DEBUG) {
			Log.w(Config.TAG, "commitText(\"" + text + "\", " + newCursorPosition + ")");
		}

		if(composition) {
			sendImePreeditString(String.valueOf(text));
			sendImePreeditStringStop();
			composition = false;
		} else {
			sendText(String.valueOf(text));
		}

		return true;
	}

	@Override
	public boolean deleteSurroundingText(int beforeLength, int afterLength) {
		if (Config.DEBUG) {
			Log.w(Config.TAG, "deleteSurroundingText(" + beforeLength + "," + afterLength + ")");
		}
		return true;
	}

	@Override
	public boolean endBatchEdit() {
		if (Config.DEBUG) {
			Log.w(Config.TAG, "endBatchEdit");
		}
		return true;
	}

	@Override
	public boolean finishComposingText() {
		if (Config.DEBUG) {
			Log.w(Config.TAG, "finishComposingText");
		}
		return true;
	}

	@Override
	public int getCursorCapsMode(int reqModes) {
		if (Config.DEBUG) {
			Log.w(Config.TAG, "getCursorCapsMode(" + reqModes + ")");
		}
		return 0;
	}

	@Override
	public ExtractedText getExtractedText(ExtractedTextRequest request, int flags) {
		return null;
	}

	@Override
	public CharSequence getSelectedText(int flags) {
		if (Config.DEBUG) {
			Log.w(Config.TAG, "getSelectedText " + flags);
		}
		return null;
	}

	@Override
	public CharSequence getTextAfterCursor(int n, int flags) {
		if (Config.DEBUG) {
			Log.w(Config.TAG, "getTextAfterCursor(" + n + "," + flags + ")");
		}
		return "";
	}

	@Override
	public CharSequence getTextBeforeCursor(int n, int flags) {
		if (Config.DEBUG) {
			Log.w(Config.TAG, "getTextBeforeCursor(" + n + "," + flags + ")");
		}
		return "";
	}

	@Override
	public boolean performContextMenuAction(int id) {
		if (Config.DEBUG) {
			Log.w(Config.TAG, "performContextMenuAction " + id);
		}
		return true;
	}

	@Override
	public boolean performEditorAction(int editorAction) {
		if (Config.DEBUG) {
			Log.w(Config.TAG, "performEditorAction(" + editorAction + ")");
		}
		if (editorAction == EditorInfo.IME_ACTION_UNSPECIFIED) {
			// The "return" key has been pressed on the IME.
			sendVkcode(0, VKCodes.VK_RETURN);
			sendVkcode(1, VKCodes.VK_RETURN);
		}
		return true;
	}

	@Override
	public boolean performPrivateCommand(String action, Bundle data) {
		if (Config.DEBUG) {
			Log.w(Config.TAG, "performPrivateCommand" + action + "," + data);
		}
		return true;
	}

	@Override
	public boolean reportFullscreenMode(boolean enabled) {
		if (Config.DEBUG) {
			Log.w(Config.TAG, "reportFullscreenMode" + enabled);
		}
		return true;
	}

	@Override
	public boolean setComposingRegion(int start, int end) {
		if (Config.DEBUG) {
			Log.w(Config.TAG, "setComposingRegion " + start + "," + end);
		}

		return true;
	}

	@Override
	public boolean setComposingText(CharSequence text, int newCursorPosition) {
		if (Config.DEBUG) {
			Log.w(Config.TAG, "setComposingText(\"" + text + "\", " + newCursorPosition + ")");
		}

		composition = true;
		sendImePreeditString(String.valueOf(text));
		return true;
	}

	@Override
	public boolean setSelection(int start, int end) {
		if (Config.DEBUG) {
			Log.w(Config.TAG, "setSelection" + start + "," + end);
		}
		return false;
	}

}
