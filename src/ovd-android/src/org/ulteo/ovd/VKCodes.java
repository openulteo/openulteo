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

import android.annotation.SuppressLint;
import android.util.SparseIntArray;
import android.view.KeyEvent;

@SuppressLint("InlinedApi")
public final class VKCodes {
	/* Mouse buttons */

	public static final int VK_LBUTTON = 0x01; /* Left mouse button */
	public static final int VK_RBUTTON = 0x02; /* Right mouse button */
	public static final int VK_CANCEL = 0x03; /* Control-break processing */
	public static final int VK_MBUTTON = 0x04; /*
												 * Middle mouse button
												 * (three-button mouse)
												 */
	public static final int VK_XBUTTON1 = 0x05; /*
												 * Windows 2000/XP: X1 mouse
												 * button
												 */
	public static final int VK_XBUTTON2 = 0x06; /*
												 * Windows 2000/XP: X2 mouse
												 * button
												 */

	/* 0x07 is undefined */

	public static final int VK_BACK = 0x08; /* BACKSPACE key */
	public static final int VK_TAB = 0x09; /* TAB key */

	/* 0x0A to 0x0B are reserved */

	public static final int VK_CLEAR = 0x0C; /* CLEAR key */
	public static final int VK_RETURN = 0x0D; /* ENTER key */

	/* 0x0E to 0x0F are undefined */

	public static final int VK_SHIFT = 0x10; /* SHIFT key */
	public static final int VK_CONTROL = 0x11; /* CTRL key */
	public static final int VK_MENU = 0x12; /* ALT key */
	public static final int VK_PAUSE = 0x13; /* PAUSE key */
	public static final int VK_CAPITAL = 0x14; /* CAPS LOCK key */
	public static final int VK_KANA = 0x15; /*
											 * Input Method Editor (IME) Kana
											 * mode
											 */
	public static final int VK_HANGUEL = 0x15; /*
												 * IME Hanguel mode (maintained
												 * for compatibility; use
												 * #define VK_HANGUL)
												 */
	public static final int VK_HANGUL = 0x15; /* IME Hangul mode */

	/* 0x16 is undefined */

	public static final int VK_JUNJA = 0x17; /* IME Junja mode */
	public static final int VK_FINAL = 0x18; /* IME final mode */
	public static final int VK_HANJA = 0x19; /* IME Hanja mode */
	public static final int VK_KANJI = 0x19; /* IME Kanji mode */

	/* 0x1A is undefined */

	public static final int VK_ESCAPE = 0x1B; /* ESC key */
	public static final int VK_CONVERT = 0x1C; /* IME convert */
	public static final int VK_NONCONVERT = 0x1D; /* IME nonconvert */
	public static final int VK_ACCEPT = 0x1E; /* IME accept */
	public static final int VK_MODECHANGE = 0x1F; /* IME mode change request */

	public static final int VK_SPACE = 0x20; /* SPACEBAR */
	public static final int VK_PRIOR = 0x21; /* PAGE UP key */
	public static final int VK_NEXT = 0x22; /* PAGE DOWN key */
	public static final int VK_END = 0x23; /* END key */
	public static final int VK_HOME = 0x24; /* HOME key */
	public static final int VK_LEFT = 0x25; /* LEFT ARROW key */
	public static final int VK_UP = 0x26; /* UP ARROW key */
	public static final int VK_RIGHT = 0x27; /* RIGHT ARROW key */
	public static final int VK_DOWN = 0x28; /* DOWN ARROW key */
	public static final int VK_SELECT = 0x29; /* SELECT key */
	public static final int VK_PRINT = 0x2A; /* PRINT key */
	public static final int VK_EXECUTE = 0x2B; /* EXECUTE key */
	public static final int VK_SNAPSHOT = 0x2C; /* PRINT SCREEN key */
	public static final int VK_INSERT = 0x2D; /* INS key */
	public static final int VK_DELETE = 0x2E; /* DEL key */
	public static final int VK_HELP = 0x2F; /* HELP key */

	/* Digits, the last 4 bits of the code represent the corresponding digit */

	public static final int VK_KEY_0 = 0x30; /* '0' key */
	public static final int VK_KEY_1 = 0x31; /* '1' key */
	public static final int VK_KEY_2 = 0x32; /* '2' key */
	public static final int VK_KEY_3 = 0x33; /* '3' key */
	public static final int VK_KEY_4 = 0x34; /* '4' key */
	public static final int VK_KEY_5 = 0x35; /* '5' key */
	public static final int VK_KEY_6 = 0x36; /* '6' key */
	public static final int VK_KEY_7 = 0x37; /* '7' key */
	public static final int VK_KEY_8 = 0x38; /* '8' key */
	public static final int VK_KEY_9 = 0x39; /* '9' key */

	/* 0x3A to 0x40 are undefined */

	/*
	 * The alphabet, the code corresponds to the capitalized letter in the ASCII
	 * code
	 */

	public static final int VK_KEY_A = 0x41; /* 'A' key */
	public static final int VK_KEY_B = 0x42; /* 'B' key */
	public static final int VK_KEY_C = 0x43; /* 'C' key */
	public static final int VK_KEY_D = 0x44; /* 'D' key */
	public static final int VK_KEY_E = 0x45; /* 'E' key */
	public static final int VK_KEY_F = 0x46; /* 'F' key */
	public static final int VK_KEY_G = 0x47; /* 'G' key */
	public static final int VK_KEY_H = 0x48; /* 'H' key */
	public static final int VK_KEY_I = 0x49; /* 'I' key */
	public static final int VK_KEY_J = 0x4A; /* 'J' key */
	public static final int VK_KEY_K = 0x4B; /* 'K' key */
	public static final int VK_KEY_L = 0x4C; /* 'L' key */
	public static final int VK_KEY_M = 0x4D; /* 'M' key */
	public static final int VK_KEY_N = 0x4E; /* 'N' key */
	public static final int VK_KEY_O = 0x4F; /* 'O' key */
	public static final int VK_KEY_P = 0x50; /* 'P' key */
	public static final int VK_KEY_Q = 0x51; /* 'Q' key */
	public static final int VK_KEY_R = 0x52; /* 'R' key */
	public static final int VK_KEY_S = 0x53; /* 'S' key */
	public static final int VK_KEY_T = 0x54; /* 'T' key */
	public static final int VK_KEY_U = 0x55; /* 'U' key */
	public static final int VK_KEY_V = 0x56; /* 'V' key */
	public static final int VK_KEY_W = 0x57; /* 'W' key */
	public static final int VK_KEY_X = 0x58; /* 'X' key */
	public static final int VK_KEY_Y = 0x59; /* 'Y' key */
	public static final int VK_KEY_Z = 0x5A; /* 'Z' key */

	public static final int VK_LWIN = 0x5B; /*
											 * Left Windows key (Microsoft
											 * Natural keyboard)
											 */
	public static final int VK_RWIN = 0x5C; /*
											 * Right Windows key (Natural
											 * keyboard)
											 */
	public static final int VK_APPS = 0x5D; /*
											 * Applications key (Natural
											 * keyboard)
											 */

	/* 0x5E is reserved */

	public static final int VK_SLEEP = 0x5F; /* Computer Sleep key */

	/*
	 * Numeric keypad digits, the last four bits of the code represent the
	 * corresponding digit
	 */

	public static final int VK_NUMPAD0 = 0x60; /* Numeric keypad '0' key */
	public static final int VK_NUMPAD1 = 0x61; /* Numeric keypad '1' key */
	public static final int VK_NUMPAD2 = 0x62; /* Numeric keypad '2' key */
	public static final int VK_NUMPAD3 = 0x63; /* Numeric keypad '3' key */
	public static final int VK_NUMPAD4 = 0x64; /* Numeric keypad '4' key */
	public static final int VK_NUMPAD5 = 0x65; /* Numeric keypad '5' key */
	public static final int VK_NUMPAD6 = 0x66; /* Numeric keypad '6' key */
	public static final int VK_NUMPAD7 = 0x67; /* Numeric keypad '7' key */
	public static final int VK_NUMPAD8 = 0x68; /* Numeric keypad '8' key */
	public static final int VK_NUMPAD9 = 0x69; /* Numeric keypad '9' key */

	/* Numeric keypad operators and special keys */

	public static final int VK_MULTIPLY = 0x6A; /* Multiply key */
	public static final int VK_ADD = 0x6B; /* Add key */
	public static final int VK_SEPARATOR = 0x6C; /* Separator key */
	public static final int VK_SUBTRACT = 0x6D; /* Subtract key */
	public static final int VK_DECIMAL = 0x6E; /* Decimal key */
	public static final int VK_DIVIDE = 0x6F; /* Divide key */

	/* Function keys, from F1 to F24 */

	public static final int VK_F1 = 0x70; /* F1 key */
	public static final int VK_F2 = 0x71; /* F2 key */
	public static final int VK_F3 = 0x72; /* F3 key */
	public static final int VK_F4 = 0x73; /* F4 key */
	public static final int VK_F5 = 0x74; /* F5 key */
	public static final int VK_F6 = 0x75; /* F6 key */
	public static final int VK_F7 = 0x76; /* F7 key */
	public static final int VK_F8 = 0x77; /* F8 key */
	public static final int VK_F9 = 0x78; /* F9 key */
	public static final int VK_F10 = 0x79; /* F10 key */
	public static final int VK_F11 = 0x7A; /* F11 key */
	public static final int VK_F12 = 0x7B; /* F12 key */
	public static final int VK_F13 = 0x7C; /* F13 key */
	public static final int VK_F14 = 0x7D; /* F14 key */
	public static final int VK_F15 = 0x7E; /* F15 key */
	public static final int VK_F16 = 0x7F; /* F16 key */
	public static final int VK_F17 = 0x80; /* F17 key */
	public static final int VK_F18 = 0x81; /* F18 key */
	public static final int VK_F19 = 0x82; /* F19 key */
	public static final int VK_F20 = 0x83; /* F20 key */
	public static final int VK_F21 = 0x84; /* F21 key */
	public static final int VK_F22 = 0x85; /* F22 key */
	public static final int VK_F23 = 0x86; /* F23 key */
	public static final int VK_F24 = 0x87; /* F24 key */

	/* 0x88 to 0x8F are unassigned */

	public static final int VK_NUMLOCK = 0x90; /* NUM LOCK key */
	public static final int VK_SCROLL = 0x91; /* SCROLL LOCK key */

	/* 0x92 to 0x96 are OEM specific */
	/* 0x97 to 0x9F are unassigned */

	/* Modifier keys */

	public static final int VK_LSHIFT = 0xA0; /* Left SHIFT key */
	public static final int VK_RSHIFT = 0xA1; /* Right SHIFT key */
	public static final int VK_LCONTROL = 0xA2; /* Left CONTROL key */
	public static final int VK_RCONTROL = 0xA3; /* Right CONTROL key */
	public static final int VK_LMENU = 0xA4; /* Left MENU key */
	public static final int VK_RMENU = 0xA5; /* Right MENU key */

	/* Browser related keys */

	public static final int VK_BROWSER_BACK = 0xA6; /*
													 * Windows 2000/XP: Browser
													 * Back key
													 */
	public static final int VK_BROWSER_FORWARD = 0xA7; /*
														 * Windows 2000/XP:
														 * Browser Forward key
														 */
	public static final int VK_BROWSER_REFRESH = 0xA8; /*
														 * Windows 2000/XP:
														 * Browser Refresh key
														 */
	public static final int VK_BROWSER_STOP = 0xA9; /*
													 * Windows 2000/XP: Browser
													 * Stop key
													 */
	public static final int VK_BROWSER_SEARCH = 0xAA; /*
													 * Windows 2000/XP: Browser
													 * Search key
													 */
	public static final int VK_BROWSER_FAVORITES = 0xAB; /*
														 * Windows 2000/XP:
														 * Browser Favorites key
														 */
	public static final int VK_BROWSER_HOME = 0xAC; /*
													 * Windows 2000/XP: Browser
													 * Start and Home key
													 */

	/* Volume related keys */

	public static final int VK_VOLUME_MUTE = 0xAD; /*
													 * Windows 2000/XP: Volume
													 * Mute key
													 */
	public static final int VK_VOLUME_DOWN = 0xAE; /*
													 * Windows 2000/XP: Volume
													 * Down key
													 */
	public static final int VK_VOLUME_UP = 0xAF; /*
												 * Windows 2000/XP: Volume Up
												 * key
												 */

	/* Media player related keys */

	public static final int VK_MEDIA_NEXT_TRACK = 0xB0; /*
														 * Windows 2000/XP: Next
														 * Track key
														 */
	public static final int VK_MEDIA_PREV_TRACK = 0xB1; /*
														 * Windows 2000/XP:
														 * Previous Track key
														 */
	public static final int VK_MEDIA_STOP = 0xB2; /*
												 * Windows 2000/XP: Stop Media
												 * key
												 */
	public static final int VK_MEDIA_PLAY_PAUSE = 0xB3; /*
														 * Windows 2000/XP:
														 * Play/Pause Media key
														 */

	/* Application launcher keys */

	public static final int VK_LAUNCH_MAIL = 0xB4; /*
													 * Windows 2000/XP: Start
													 * Mail key
													 */
	public static final int VK_LAUNCH_MEDIA_SELECT = 0xB5; /*
															 * Windows 2000/XP:
															 * Select Media key
															 */
	public static final int VK_LAUNCH_APP1 = 0xB6; /*
													 * Windows 2000/XP: Start
													 * Application 1 key
													 */
	public static final int VK_LAUNCH_APP2 = 0xB7; /*
													 * Windows 2000/XP: Start
													 * Application 2 key
													 */

	/* 0xB8 and 0xB9 are reserved */

	/* OEM keys */

	public static final int VK_OEM_1 = 0xBA; /*
											 * Used for miscellaneous
											 * characters; it can vary by
											 * keyboard.
											 */
	/* Windows 2000/XP: For the US standard keyboard, the ';:' key */

	public static final int VK_OEM_PLUS = 0xBB; /*
												 * Windows 2000/XP: For any
												 * country/region, the '+' key
												 */
	public static final int VK_OEM_COMMA = 0xBC; /*
												 * Windows 2000/XP: For any
												 * country/region, the ',' key
												 */
	public static final int VK_OEM_MINUS = 0xBD; /*
												 * Windows 2000/XP: For any
												 * country/region, the '-' key
												 */
	public static final int VK_OEM_PERIOD = 0xBE; /*
												 * Windows 2000/XP: For any
												 * country/region, the '.' key
												 */

	public static final int VK_OEM_2 = 0xBF; /*
											 * Used for miscellaneous
											 * characters; it can vary by
											 * keyboard.
											 */
	/* Windows 2000/XP: For the US standard keyboard, the '/?' key */

	public static final int VK_OEM_3 = 0xC0; /*
											 * Used for miscellaneous
											 * characters; it can vary by
											 * keyboard.
											 */
	/* Windows 2000/XP: For the US standard keyboard, the '`~' key */

	/* 0xC1 to 0xD7 are reserved */
	public static final int VK_ABNT_C1 = 0xC1; /* Brazilian (ABNT) Keyboard */
	public static final int VK_ABNT_C2 = 0xC2; /* Brazilian (ABNT) Keyboard */

	/* 0xD8 to 0xDA are unassigned */

	public static final int VK_OEM_4 = 0xDB; /*
											 * Used for miscellaneous
											 * characters; it can vary by
											 * keyboard. Windows 2000/XP: For
											 * the US standard keyboard, the
											 * '[{' key
											 */

	public static final int VK_OEM_5 = 0xDC; /*
											 * Used for miscellaneous
											 * characters; it can vary by
											 * keyboard.
											 */
	; /* Windows 2000/XP: For the US standard keyboard, the '\|' key */

	public static final int VK_OEM_6 = 0xDD; /*
											 * Used for miscellaneous
											 * characters; it can vary by
											 * keyboard.
											 */
	; /* Windows 2000/XP: For the US standard keyboard, the ']}' key */

	public static final int VK_OEM_7 = 0xDE; /*
											 * Used for miscellaneous
											 * characters; it can vary by
											 * keyboard.
											 */
	; /*
	 * Windows 2000/XP: For the US standard keyboard, the
	 * 'single-quote/double-quote' key
	 */

	public static final int VK_OEM_8 = 0xDF; /*
											 * Used for miscellaneous
											 * characters; it can vary by
											 * keyboard.
											 */

	/* 0xE0 is reserved */
	/* 0xE1 is OEM specific */

	public static final int VK_OEM_102 = 0xE2; /*
												 * Windows 2000/XP: Either the
												 * angle bracket key or
												 */
	; /* the backslash key on the RT 102-key keyboard */

	/* 0xE3 and 0xE4 are OEM specific */

	public static final int VK_PROCESSKEY = 0xE5; /*
												 * Windows 95/98/Me, Windows NT
												 * 4.0, Windows 2000/XP: IME
												 * PROCESS key
												 */

	/* 0xE6 is OEM specific */

	public static final int VK_PACKET = 0xE7; /*
											 * Windows 2000/XP: Used to pass
											 * Unicode characters as if they
											 * were keystrokes.
											 */
	/*
	 * The #define VK_PACKET key is the low word of a 32-bit Virtual Key value
	 * used
	 */
	/* for non-keyboard input methods. For more information, */
	/* see Remark in KEYBDINPUT, SendInput, WM_KEYDOWN, and WM_KEYUP */

	/* 0xE8 is unassigned */
	/* 0xE9 to 0xF5 are OEM specific */

	public static final int VK_ATTN = 0xF6; /* Attn key */
	public static final int VK_CRSEL = 0xF7; /* CrSel key */
	public static final int VK_EXSEL = 0xF8; /* ExSel key */
	public static final int VK_EREOF = 0xF9; /* Erase EOF key */
	public static final int VK_PLAY = 0xFA; /* Play key */
	public static final int VK_ZOOM = 0xFB; /* Zoom key */
	public static final int VK_NONAME = 0xFC; /* Reserved */
	public static final int VK_PA1 = 0xFD; /* PA1 key */
	public static final int VK_OEM_CLEAR = 0xFE; /* Clear key */

	private static final SparseIntArray androidKeysMap;
	static {
		SparseIntArray aMap = new SparseIntArray();
		aMap.append(KeyEvent.KEYCODE_DEL, VK_BACK);
		aMap.append(KeyEvent.KEYCODE_TAB, VK_TAB);
		aMap.append(KeyEvent.KEYCODE_CLEAR, VK_CLEAR);
		aMap.append(KeyEvent.KEYCODE_ENTER, VK_RETURN);
		aMap.append(KeyEvent.KEYCODE_MENU, VK_MENU);
		aMap.append(KeyEvent.KEYCODE_CAPS_LOCK, VK_CAPITAL);
		aMap.append(KeyEvent.KEYCODE_KANA, VK_KANA);
		aMap.append(KeyEvent.KEYCODE_ESCAPE, VK_ESCAPE);
		aMap.append(KeyEvent.KEYCODE_SPACE, VK_SPACE);
		aMap.append(KeyEvent.KEYCODE_PAGE_UP, VK_PRIOR);
		aMap.append(KeyEvent.KEYCODE_PAGE_DOWN, VK_NEXT);
		aMap.append(KeyEvent.KEYCODE_MOVE_END, VK_END);
		aMap.append(KeyEvent.KEYCODE_MOVE_HOME, VK_HOME);
		aMap.append(KeyEvent.KEYCODE_DPAD_LEFT, VK_LEFT);
		aMap.append(KeyEvent.KEYCODE_DPAD_UP, VK_UP);
		aMap.append(KeyEvent.KEYCODE_DPAD_RIGHT, VK_RIGHT);
		aMap.append(KeyEvent.KEYCODE_DPAD_DOWN, VK_DOWN);
		aMap.append(KeyEvent.KEYCODE_BUTTON_SELECT, VK_SELECT);
		aMap.append(KeyEvent.KEYCODE_SYSRQ, VK_SNAPSHOT);
		aMap.append(KeyEvent.KEYCODE_INSERT, VK_INSERT);
		aMap.append(KeyEvent.KEYCODE_FORWARD_DEL, VK_DELETE);

		aMap.append(KeyEvent.KEYCODE_0, VK_KEY_0);
		aMap.append(KeyEvent.KEYCODE_1, VK_KEY_1);
		aMap.append(KeyEvent.KEYCODE_2, VK_KEY_2);
		aMap.append(KeyEvent.KEYCODE_3, VK_KEY_3);
		aMap.append(KeyEvent.KEYCODE_4, VK_KEY_4);
		aMap.append(KeyEvent.KEYCODE_5, VK_KEY_5);
		aMap.append(KeyEvent.KEYCODE_6, VK_KEY_6);
		aMap.append(KeyEvent.KEYCODE_7, VK_KEY_7);
		aMap.append(KeyEvent.KEYCODE_8, VK_KEY_8);
		aMap.append(KeyEvent.KEYCODE_9, VK_KEY_9);

		aMap.append(KeyEvent.KEYCODE_A, VK_KEY_A);
		aMap.append(KeyEvent.KEYCODE_B, VK_KEY_B);
		aMap.append(KeyEvent.KEYCODE_C, VK_KEY_C);
		aMap.append(KeyEvent.KEYCODE_D, VK_KEY_D);
		aMap.append(KeyEvent.KEYCODE_E, VK_KEY_E);
		aMap.append(KeyEvent.KEYCODE_F, VK_KEY_F);
		aMap.append(KeyEvent.KEYCODE_G, VK_KEY_G);
		aMap.append(KeyEvent.KEYCODE_H, VK_KEY_H);
		aMap.append(KeyEvent.KEYCODE_I, VK_KEY_I);
		aMap.append(KeyEvent.KEYCODE_J, VK_KEY_J);
		aMap.append(KeyEvent.KEYCODE_K, VK_KEY_K);
		aMap.append(KeyEvent.KEYCODE_L, VK_KEY_L);
		aMap.append(KeyEvent.KEYCODE_M, VK_KEY_M);
		aMap.append(KeyEvent.KEYCODE_N, VK_KEY_N);
		aMap.append(KeyEvent.KEYCODE_O, VK_KEY_O);
		aMap.append(KeyEvent.KEYCODE_P, VK_KEY_P);
		aMap.append(KeyEvent.KEYCODE_Q, VK_KEY_Q);
		aMap.append(KeyEvent.KEYCODE_R, VK_KEY_R);
		aMap.append(KeyEvent.KEYCODE_S, VK_KEY_S);
		aMap.append(KeyEvent.KEYCODE_T, VK_KEY_T);
		aMap.append(KeyEvent.KEYCODE_U, VK_KEY_U);
		aMap.append(KeyEvent.KEYCODE_V, VK_KEY_V);
		aMap.append(KeyEvent.KEYCODE_W, VK_KEY_W);
		aMap.append(KeyEvent.KEYCODE_X, VK_KEY_X);
		aMap.append(KeyEvent.KEYCODE_Y, VK_KEY_Y);
		aMap.append(KeyEvent.KEYCODE_Z, VK_KEY_Z);

		aMap.append(KeyEvent.KEYCODE_META_LEFT, VK_LWIN);
		aMap.append(KeyEvent.KEYCODE_META_RIGHT, VK_RWIN);

		aMap.append(KeyEvent.KEYCODE_A, VK_RWIN);

		aMap.append(KeyEvent.KEYCODE_NUMPAD_0, VK_NUMPAD0);
		aMap.append(KeyEvent.KEYCODE_NUMPAD_1, VK_NUMPAD1);
		aMap.append(KeyEvent.KEYCODE_NUMPAD_2, VK_NUMPAD2);
		aMap.append(KeyEvent.KEYCODE_NUMPAD_3, VK_NUMPAD3);
		aMap.append(KeyEvent.KEYCODE_NUMPAD_4, VK_NUMPAD4);
		aMap.append(KeyEvent.KEYCODE_NUMPAD_5, VK_NUMPAD5);
		aMap.append(KeyEvent.KEYCODE_NUMPAD_6, VK_NUMPAD6);
		aMap.append(KeyEvent.KEYCODE_NUMPAD_7, VK_NUMPAD7);
		aMap.append(KeyEvent.KEYCODE_NUMPAD_8, VK_NUMPAD8);
		aMap.append(KeyEvent.KEYCODE_NUMPAD_9, VK_NUMPAD9);

		aMap.append(KeyEvent.KEYCODE_NUMPAD_MULTIPLY, VK_MULTIPLY);
		aMap.append(KeyEvent.KEYCODE_NUMPAD_ADD, VK_ADD);
		aMap.append(KeyEvent.KEYCODE_NUMPAD_SUBTRACT, VK_SUBTRACT);
		aMap.append(KeyEvent.KEYCODE_NUMPAD_DOT, VK_DECIMAL);
		aMap.append(KeyEvent.KEYCODE_NUMPAD_DIVIDE, VK_DIVIDE);
		aMap.append(KeyEvent.KEYCODE_NUMPAD_COMMA, VK_SEPARATOR);
		aMap.append(KeyEvent.KEYCODE_NUMPAD_ENTER, VK_RETURN);

		aMap.append(KeyEvent.KEYCODE_F1, VK_F1);
		aMap.append(KeyEvent.KEYCODE_F2, VK_F2);
		aMap.append(KeyEvent.KEYCODE_F3, VK_F3);
		aMap.append(KeyEvent.KEYCODE_F4, VK_F4);
		aMap.append(KeyEvent.KEYCODE_F5, VK_F5);
		aMap.append(KeyEvent.KEYCODE_F6, VK_F6);
		aMap.append(KeyEvent.KEYCODE_F7, VK_F7);
		aMap.append(KeyEvent.KEYCODE_F8, VK_F8);
		aMap.append(KeyEvent.KEYCODE_F9, VK_F9);
		aMap.append(KeyEvent.KEYCODE_F10, VK_F10);
		aMap.append(KeyEvent.KEYCODE_F11, VK_F11);
		aMap.append(KeyEvent.KEYCODE_F12, VK_F12);

		aMap.append(KeyEvent.KEYCODE_NUM_LOCK, VK_NUMLOCK);
		aMap.append(KeyEvent.KEYCODE_SCROLL_LOCK, VK_SCROLL);

		aMap.append(KeyEvent.KEYCODE_SHIFT_LEFT, VK_LSHIFT);
		aMap.append(KeyEvent.KEYCODE_SHIFT_RIGHT, VK_RSHIFT);
		aMap.append(KeyEvent.KEYCODE_CTRL_LEFT, VK_LCONTROL);
		aMap.append(KeyEvent.KEYCODE_CTRL_RIGHT, VK_RCONTROL);

		aMap.append(KeyEvent.KEYCODE_BACK, VK_BROWSER_BACK);
		aMap.append(KeyEvent.KEYCODE_FORWARD, VK_BROWSER_FORWARD);

		aMap.append(KeyEvent.KEYCODE_VOLUME_MUTE, VK_VOLUME_MUTE);
		aMap.append(KeyEvent.KEYCODE_VOLUME_DOWN, VK_VOLUME_DOWN);
		aMap.append(KeyEvent.KEYCODE_VOLUME_UP, VK_VOLUME_UP);

		aMap.append(KeyEvent.KEYCODE_MEDIA_NEXT, VK_MEDIA_NEXT_TRACK);
		aMap.append(KeyEvent.KEYCODE_MEDIA_PREVIOUS, VK_MEDIA_PREV_TRACK);
		aMap.append(KeyEvent.KEYCODE_MEDIA_STOP, VK_MEDIA_STOP);
		aMap.append(KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE, VK_MEDIA_PLAY_PAUSE);
		aMap.append(KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE, VK_MEDIA_PLAY_PAUSE);

		aMap.append(KeyEvent.KEYCODE_ENVELOPE, VK_LAUNCH_MAIL);
		aMap.append(KeyEvent.KEYCODE_MUSIC, VK_LAUNCH_MEDIA_SELECT);
		aMap.append(KeyEvent.KEYCODE_SETTINGS, VK_LAUNCH_APP1);
		aMap.append(KeyEvent.KEYCODE_CALCULATOR, VK_LAUNCH_APP1);
		aMap.append(KeyEvent.KEYCODE_EXPLORER, VK_BROWSER_HOME);

		aMap.append(KeyEvent.KEYCODE_PLUS, VK_OEM_PLUS);
		aMap.append(KeyEvent.KEYCODE_COMMA, VK_OEM_COMMA);
		aMap.append(KeyEvent.KEYCODE_MINUS, VK_OEM_MINUS);
		aMap.append(KeyEvent.KEYCODE_PERIOD, VK_OEM_PERIOD);

		aMap.append(KeyEvent.KEYCODE_ZOOM_IN, VK_ZOOM);
		aMap.append(KeyEvent.KEYCODE_MEDIA_PLAY, VK_PLAY);
		aMap.append(KeyEvent.KEYCODE_CLEAR, VK_OEM_CLEAR);

		androidKeysMap = aMap;
	}
	
	public static int KeyCodeToVKeyCode(int keycode) {
		return androidKeysMap.get(keycode, 0);
	}

}
