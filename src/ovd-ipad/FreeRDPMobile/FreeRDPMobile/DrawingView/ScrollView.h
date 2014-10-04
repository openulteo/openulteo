/**
 * Copyright (C) 2011-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author Harold LEBOULANGER <harold@ulteo.com> 2011
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
 **/


#import "OVDFloatingToolbarViewController.h"

@class DrawView;
@class OVDFloatingToolbarViewController;
@class OVDClientViewController;


@interface ScrollView : UIScrollView <UIScrollViewDelegate, UITextFieldDelegate, OVDFloatingToolbarDelegate, UIGestureRecognizerDelegate, UIAlertViewDelegate> {
	DrawView *drawView;
  OVDFloatingToolbarViewController *toolbar;
  IBOutlet OVDClientViewController *clientViewController;   
  
	CGPoint tapLocation;
  
  UITextField *textField;
  
  BOOL toolbarDisplayed;
  
  UIToolbar *keyboardToolbar;
  BOOL isLongPressed;
  
  NSString *previousString;
  BOOL shouldChangedWasCalled;
  BOOL preventNotification;
  BOOL composition;
  
  int width;
  int height;
  
  CGFloat currentViewWidth;
  
  UIPinchGestureRecognizer *pinchGR;
  UIPanGestureRecognizer   *panGR;
  BOOL firstPan;
  CGPoint firstPoint;
}

- (void)createGestureRecognizer;

//- (CGRect)zoomRectForScrollView:(UIScrollView *)scrollView withScale:(float)scale withCenter:(CGPoint)center;

- (void)launchConnection;
- (void)logout;

- (void)toggleToolbar;
//- (void)updateToolbarWidth;

- (void)keyPressed:(NSNotification*)notification;
- (void)resetTextField;

@property (retain)OVDFloatingToolbarViewController *toolbar;
@property (retain)OVDClientViewController *clientViewController;

@end
