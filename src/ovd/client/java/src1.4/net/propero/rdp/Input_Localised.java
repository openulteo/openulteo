/* Input_Localised.java
 * Component: ProperJavaRDP
 * 
 * Revision: $Revision: 1.1.1.1 $
 * Author: $Author: suvarov $
 * Date: $Date: 2007/03/08 00:26:53 $
 *
 * Copyright (c) 2005 Propero Limited
 *
 * Purpose: Java 1.4 specific extension of Input class
 */
// Created on 07-Jul-2003
package net.propero.rdp;

import java.awt.*;
import java.awt.event.*;
import java.util.Collections;

import net.propero.rdp.keymapping.KeyCode;
import net.propero.rdp.keymapping.KeyCode_FileBased;


public class Input_Localised extends Input {
	private Options opt = null;
	
	@SuppressWarnings("unchecked")
	public Input_Localised(RdesktopCanvas c, Rdp r, KeyCode_FileBased k, Options opt_){
		super(c,r,k,opt_);
		this.opt = opt_;
		KeyboardFocusManager.getCurrentKeyboardFocusManager().setDefaultFocusTraversalKeys (KeyboardFocusManager.FORWARD_TRAVERSAL_KEYS, Collections.EMPTY_SET);
		KeyboardFocusManager.getCurrentKeyboardFocusManager().setDefaultFocusTraversalKeys (KeyboardFocusManager.BACKWARD_TRAVERSAL_KEYS, Collections.EMPTY_SET);		
	}
	
	@SuppressWarnings("unchecked")
	public Input_Localised(RdesktopCanvas c, Rdp r, String k, Options opt_, Common common_){
		super(c,r,k,opt_,common_);
		this.opt = opt_;
		KeyboardFocusManager.getCurrentKeyboardFocusManager().setDefaultFocusTraversalKeys (KeyboardFocusManager.FORWARD_TRAVERSAL_KEYS, Collections.EMPTY_SET);
		KeyboardFocusManager.getCurrentKeyboardFocusManager().setDefaultFocusTraversalKeys (KeyboardFocusManager.BACKWARD_TRAVERSAL_KEYS, Collections.EMPTY_SET);		
	}
	
	public void clearKeys(){
		super.clearKeys();
		if (lastKeyEvent != null && lastKeyEvent.isAltGraphDown()) sendScancode(getTime(),RDP_KEYRELEASE,0x38 | KeyCode.SCANCODE_EXTENDED); //r.alt
	}
	public void setKeys(){
		super.setKeys();
		if (lastKeyEvent != null && lastKeyEvent.isAltGraphDown()) sendScancode(getTime(),RDP_KEYPRESS,0x38 | KeyCode.SCANCODE_EXTENDED); //r.alt
	}

	protected void doLockKeys(){
		// 	doesn't work on Java 1.4.1_02 or 1.4.2 on Linux, there is a bug in java....
		// does work on the same version on Windows.
		if(!this.opt.readytosend) return;
		if(!this.opt.useLockingKeyState) return;
		if(Constants.OS == Constants.LINUX) return; // broken for linux
		if(Constants.OS == Constants.MAC) return; // unsupported operation for mac
		logger.debug("doLockKeys");
		
		try {
			Toolkit tk = Toolkit.getDefaultToolkit();	   
			if (tk.getLockingKeyState(KeyEvent.VK_CAPS_LOCK) != capsLockOn){
				capsLockOn = !capsLockOn;
				logger.debug("CAPS LOCK toggle");
				sendScancode(getTime(),RDP_KEYPRESS,0x3a);
				sendScancode(getTime(),RDP_KEYRELEASE,0x3a);
		
			}
			if (tk.getLockingKeyState(KeyEvent.VK_SCROLL_LOCK) != scrollLockOn){
				scrollLockOn = !scrollLockOn;
				logger.debug("SCROLL LOCK toggle");
				sendScancode(getTime(),RDP_KEYPRESS,0x46);
				sendScancode(getTime(),RDP_KEYRELEASE,0x46);
			}
	  }catch(Exception e){
		  this.opt.useLockingKeyState = false;
	  }
	}
	
	public boolean handleShortcutKeys(long time, KeyEvent e, boolean pressed){
		if (super.handleShortcutKeys(time,e,pressed)) return true;

		if (!altDown) return false; // all of the below have ALT on
		 
		switch(e.getKeyCode()){		
		case KeyEvent.VK_MINUS: // for laptops that can't do Ctrl+Alt+Minus
			if (ctrlDown){
					if(pressed){
						sendScancode(time,RDP_KEYRELEASE,0x1d); // Ctrl
						sendScancode(time,RDP_KEYPRESS,0x37 | KeyCode.SCANCODE_EXTENDED); // PrtSc
						logger.debug("shortcut pressed: sent ALT+PRTSC");
					}
					else{
						sendScancode(time,RDP_KEYRELEASE,0x37 | KeyCode.SCANCODE_EXTENDED); // PrtSc
						sendScancode(time,RDP_KEYPRESS,0x1d); // Ctrl
					}
			}
			break;
		default: return false;
		}
		return true;
	}
	
	@SuppressWarnings("unused")
	private class RdesktopMouseWheelAdapter implements MouseWheelListener {
		public void mouseWheelMoved (MouseWheelEvent e){
				 int time = getTime();
					 //   if(logger.isInfoEnabled()) logger.info("mousePressed at "+time);
					if(rdp != null) {
						 if(e.getWheelRotation()<0){ // up
							 rdp.sendInput(time, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON4 | MOUSE_FLAG_DOWN, e.getX(), e.getY());
						 } 
						 else{ // down
							 rdp.sendInput(time, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON5 | MOUSE_FLAG_DOWN, e.getX(), e.getY());
						 }
					}
			 }
	}
}
