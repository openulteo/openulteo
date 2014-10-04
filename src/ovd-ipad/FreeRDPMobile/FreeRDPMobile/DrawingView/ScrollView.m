/**
 * Copyright (C) 2011-2014 Ulteo SAS
 * http://www.ulteo.com
 * Author Harold LEBOULANGER <harold@ulteo.com> 2011
 * Author David PHAM-VAN <d.pham-van@ulteo.com> 2012, 2013
 * Author Alexandre CONFIANT-LATOUR <a.confiant@ulteo.com> 2014
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


#import "ScrollView.h"

#import "urdp_event.h"

#import "DrawView.h"
#import "OVDFloatingToolbarViewController.h"
#import "OVDClientViewController.h"
#import "OVDHelpViewController.h"

#define HEIGHT_TOOLBAR   40
#define DOUBLE_TAP_DELAY 0.35

CGPoint midpointBetweenPoints(CGPoint a, CGPoint b);

@interface ScrollView ()
- (void)handleSingleTap:(UIGestureRecognizer *)sender;
- (void)handlePanCust:(UIGestureRecognizer *)sender;
- (void)handleLongTap:(UIGestureRecognizer *)sender;
- (void)handleDoubleTap:(UIGestureRecognizer *)sender;
- (void)handleTwoFingersTap:(UIGestureRecognizer *)sender;
- (void)handleThreeFingersTap:(UIGestureRecognizer *)sender;

//- (CGPoint)computeRealPoint:(CGPoint)zoomedPoint;

- (void)keyboardWillShow:(NSNotification *)note;
- (void)keyboardWillHide:(NSNotification *)note;

@end

@implementation ScrollView

@synthesize toolbar;
@synthesize clientViewController;

- (void)dealloc {
	DESTROY(drawView);
  DESTROY(toolbar);
  DESTROY(clientViewController);
  DESTROY(textField);
	[super dealloc];
}

// Method called by Interface Builder during deserialization 
- (id)initWithCoder:(NSCoder *)sourceCoder {
	if ((self = [super initWithCoder:sourceCoder])) {
    width = [[NSUserDefaults standardUserDefaults] integerForKey:kWidth];
    height = [[NSUserDefaults standardUserDefaults] integerForKey:kHeight];
    
    [[UIApplication sharedApplication] setStatusBarHidden:YES withAnimation:YES];
    
    // Setup the scroll view
    [self setContentSize:CGSizeMake(width, height)];
    [self setDelegate:self];

    self.contentScaleFactor = 1.0;
    self.layer.contentsScale = 1.0;

    [self setShowsHorizontalScrollIndicator:NO]; // Hide scroll indicator
    [self setShowsVerticalScrollIndicator:NO];
    [self setBounces:NO]; // Ugly if set to YES
    [self setBouncesZoom:NO];
    [self setBackgroundColor:[UIColor scrollViewTexturedBackgroundColor]];

    // The view where all the drawing orders are displayed
		drawView = [[DrawView alloc] initWithFrame:CGRectMake(0, 0, width, height)];
    [self addSubview:drawView];
    
    // Textfield needed to enter text
    textField = [[UITextField alloc] initWithFrame:CGRectMake(width/2, height/2, 0, 0)];
    [textField setDelegate:self];
    [textField setAutocapitalizationType:UITextAutocapitalizationTypeNone];
    [textField setAutocorrectionType:UITextAutocorrectionTypeNo];
    [self resetTextField];
    [self addSubview:textField];
    [textField addTarget:self action:@selector(handlefieldTextChanged:) forControlEvents:UIControlEventEditingChanged];
    composition = false;
    
    // Init toolbar
    toolbar = [[OVDFloatingToolbarViewController alloc] initWithNibName:@"OVDFloatingToolbarView" bundle:nil];
    [toolbar setDelegate:self];
    toolbarDisplayed = NO;
    currentViewWidth = width;
    [self addObserver:self forKeyPath:@"self.bounds" options:NSKeyValueObservingOptionNew context:nil];
    
    // Init keyboard notification to add a custom toolbar
    [[NSNotificationCenter defaultCenter] addObserver:self 
                                             selector:@selector(keyboardWillShow:)
                                                 name:UIKeyboardWillShowNotification 
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(keyboardWillHide:)
                                                 name:UIKeyboardWillHideNotification
                                               object:nil];
    [self createGestureRecognizer];
    
    // Launch connection as soon as we are ready
    [self performSelector:@selector(launchConnection) withObject:NULL afterDelay:0];    
	}
	return self;
}


- (void)launchConnection {
  [drawView performSelector:@selector(launchConnection) withObject:nil afterDelay:0];
  [toolbar setClientViewController:clientViewController];
}


- (void)createGestureRecognizer; {
  for (UIGestureRecognizer *gestureRecognizer in self.gestureRecognizers) {
    if ([gestureRecognizer  isKindOfClass:[UIPanGestureRecognizer class]]) {
      panGR = (UIPanGestureRecognizer *) gestureRecognizer;
//      [panGR setEnabled:NO];
      panGR.minimumNumberOfTouches = 2;               
      panGR.maximumNumberOfTouches = 2;

      // Allow overriding
//      [panGR setDelegate:self];
    }
    
    if ([gestureRecognizer isKindOfClass:[UIPinchGestureRecognizer class]]) {
      pinchGR = (UIPinchGestureRecognizer *)gestureRecognizer;
    }
  }

  
  UITapGestureRecognizer *tapGesture = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleSingleTap:)];
  [tapGesture setNumberOfTapsRequired:1];
  [tapGesture setNumberOfTouchesRequired:1];
  [self addGestureRecognizer:tapGesture];
  DESTROY(tapGesture);
  
  UITapGestureRecognizer *doubleTapGesture = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleDoubleTap:)];
  [doubleTapGesture setNumberOfTapsRequired:2];
  [self addGestureRecognizer:doubleTapGesture];
  DESTROY(doubleTapGesture);
  
  UILongPressGestureRecognizer *longPressGesture = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(handleLongTap:)];
  [self addGestureRecognizer:longPressGesture];
  DESTROY(longPressGesture);

  UITapGestureRecognizer *twoFingersTapGesture = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleTwoFingersTap:)];
  [twoFingersTapGesture setNumberOfTouchesRequired:2];
  [self addGestureRecognizer:twoFingersTapGesture];
  DESTROY(twoFingersTapGesture);
  
  UITapGestureRecognizer *threeFingersTapGesture = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleThreeFingersTap:)];
  [threeFingersTapGesture setNumberOfTouchesRequired:3];
  [self addGestureRecognizer:threeFingersTapGesture];
  DESTROY(threeFingersTapGesture);

  UIPanGestureRecognizer *panGesture = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(handlePanCust:)];
  [panGesture setMaximumNumberOfTouches:1];
  [panGesture setMinimumNumberOfTouches:1];
  [self addGestureRecognizer:panGesture];
  DESTROY(panGesture);
  
//  UIPanGestureRecognizer *scrollPanGesture = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(handleScrollPan:)];
//  [scrollPanGesture setMaximumNumberOfTouches:2];
//  [scrollPanGesture setMinimumNumberOfTouches:2];
//  [self addGestureRecognizer:scrollPanGesture];
//  panGesture = NO;
//  DESTROY(scrollPanGesture);
  
//  for (UIGestureRecognizer *gestureRecognizer in self.gestureRecognizers) {
//    DDLogVerbose(@"gestureRecognizer: %@", gestureRecognizer);
//  }
}

#pragma mark -
#pragma mark UIGestureRecognizerDelegate

// This method allows multiple gesture recognizer at the same time
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer 
shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer {
  if ([gestureRecognizer isKindOfClass:[UIPinchGestureRecognizer class]] ||
      [gestureRecognizer isKindOfClass:[UIPanGestureRecognizer class]]) {
    return NO;
  }
  return YES;
}


- (UIView *)viewForZoomingInScrollView:(UIScrollView *)scrollView {
	return drawView;
}


#pragma mark Pointer Processing


- (CGPoint)computeRealPoint:(CGPoint)zoomedPoint {
  CGFloat zoomScale = [self zoomScale];
  float scale = [[UIScreen mainScreen] scale] / zoomScale;
  //log_info("offset x:%f y:%f zoom:%f", offset.x, offset.y, zoomScale);
  return CGPointMake(zoomedPoint.x * scale, zoomedPoint.y * scale);
}


- (void)handleSingleTap:(UIGestureRecognizer *)sender {
  DDLogVerbose(@"---");
  CGPoint point = [self computeRealPoint:[sender locationInView:self]];
  [[drawView rdp] clickDownWithButton:1 andPoint:point];
  [[drawView rdp] clickUpWithButton:1 andPoint:point];
}


- (void)handlePanCust:(UIGestureRecognizer *)sender {
  CGPoint point = [self computeRealPoint:[sender locationInView:self]];

  UIGestureRecognizerState state = [sender state];
  
  if (state == UIGestureRecognizerStateBegan) {
    [[drawView rdp] clickDownWithButton:1 andPoint:point];
  } else if (state == UIGestureRecognizerStateEnded) {
    [[drawView rdp] clickUpWithButton:1 andPoint:point];
  } else if (state == UIGestureRecognizerStateChanged) {
    [[drawView rdp] clickMoveWithButton:1 andPoint:point];
  } else {
   }
}


- (void)handleLongTap:(UIGestureRecognizer *)sender {
  //	DLog(@"---");
  CGPoint point = [self computeRealPoint:[sender locationInView:self]];
  
  UIGestureRecognizerState state = [sender state];
  switch(state) {
    case UIGestureRecognizerStateBegan:
      isLongPressed = YES;
      [[drawView rdp] clickDownWithButton:3 andPoint:point];
      [[drawView rdp] clickUpWithButton:3 andPoint:point];
      break;
    default: return;
  }
}

- (void)handleDoubleTap:(UIGestureRecognizer *)sender;
{
  DDLogVerbose(@"---");
  CGPoint point = [self computeRealPoint:[sender locationInView:self]];
  [[drawView rdp] clickDownWithButton:1 andPoint:point];
  [[drawView rdp] clickUpWithButton:1 andPoint:point];
}

- (void)handleTwoFingersTap:(UIGestureRecognizer *)sender; 
{
  if ([textField isFirstResponder]) {
    [textField resignFirstResponder];
  } else {
    [textField becomeFirstResponder];
  }
}

- (void)handleThreeFingersTap:(UIGestureRecognizer *)sender;
{
  [self toggleToolbar];

}

- (void)handleTwoFingerSwipe:(UIGestureRecognizer *)sender; 
{
  //CGPoint point = [self computeRealPoint:[sender locationInView:self]];
  CGPoint point = [sender locationInView:drawView];
  switch ([sender state]) {
    case UIGestureRecognizerStateBegan:
      [[drawView rdp] clickDownWithButton:1 andPoint:point];
      break;
    case UIGestureRecognizerStateChanged:
      [[drawView rdp] clickDownWithButton:1 andPoint:point];
      break;
    case UIGestureRecognizerStateEnded:
      [[drawView rdp] clickUpWithButton:1 andPoint:point];
      break;
    default:
      return;
  }
}

#pragma mark Keyboard Delegate

- (BOOL)textField:(UITextField *)tf shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string {
  DDLogVerbose(@"---");

	if ([string isEqualToString:@""]) {
		// Backspace
		if(tf.markedTextRange) { return YES; }
		[[drawView rdp] sendScancode:0x0E withFlag:KBD_FLAGS_DOWN];
		[[drawView rdp] sendScancode:0x0E withFlag:KBD_FLAGS_RELEASE];
		[self resetTextField];
	} else if ([string isEqualToString:@" "]) {
		// Space
		[[drawView rdp] sendScancode:0x39 withFlag:KBD_FLAGS_DOWN];
		[[drawView rdp] sendScancode:0x39 withFlag:KBD_FLAGS_RELEASE];
	}

  return YES;
}


- (BOOL)textFieldShouldReturn:(UITextField *)tf {
  DDLogVerbose(@"---");
    [[drawView rdp] sendScancode:0x1C withFlag:KBD_FLAGS_DOWN];
    [[drawView rdp] sendScancode:0x1C withFlag:KBD_FLAGS_RELEASE];
  
  [self resetTextField];
  
  return YES;
}

- (IBAction)handlefieldTextChanged: (UITextField *)tf{
	UITextRange *range = textField.markedTextRange;

	if(range) {
			composition = true;

			NSUInteger location = [textField offsetFromPosition:textField.beginningOfDocument toPosition:range.start];
			NSUInteger length = [textField offsetFromPosition:range.start toPosition:range.end];
			NSRange selectedRange = NSMakeRange(location, length);
			NSString *str = [textField.text substringWithRange:selectedRange];
			NSData *data = [str dataUsingEncoding:NSUTF16LittleEndianStringEncoding];
			[[drawView rdp] sendImePreeditString:data.bytes withSize:data.length];
			previousString = [[textField text] retain];
	} else {
		if(composition) {
			composition = false;
			int commit = [tf.text isEqualToString:previousString];

			if(commit) {
				/* Server side composition string is valid. Commit it */
				[[drawView rdp] sendImePreeditStringStop];
			}	else {
				/* Cancel server side composition */
				NSString *str = @"";
				NSData *data = [str dataUsingEncoding:NSUTF16LittleEndianStringEncoding];
				[[drawView rdp] sendImePreeditString:data.bytes withSize:data.length];
				[[drawView rdp] sendImePreeditStringStop];
			}
			[self resetTextField];
		}
	}
}

- (void)keyPressed:(NSNotification*)notification
{
  DDLogVerbose(@"---");
  
//  DDLogVerbose(@"notification: '%@'", [[notification object] text]);
//  DDLogVerbose(@"previous: '%@'", previousString);

  if (preventNotification) {
    DDLogVerbose(@"preventNotification");
    return;
  }
  
  NSString *text = [[notification object] text];
  if ([text isEqualToString:@""]) {
    [self resetTextField];
    return;
  }

  int textLength = [text length];
  int previousStringLength = [previousString length];
  unichar new = 0, old = 0;
  //int pivot = -1;
  
  int max = (textLength > previousStringLength) ? textLength : previousStringLength;
  
  for (int i = 0; i < max; i++) {
//    DDLogVerbose(@"i: %d", i);
    if (i < textLength) new = [text characterAtIndex:i];
    if (i < previousStringLength) old = [previousString characterAtIndex:i];
//    DDLogVerbose(@"old: %d new: %d", old, new);

    if (old && new && old == new) {
//      DDLogVerbose(@"continue");
      old = new = 0;
      continue;
    } else if (old && new && old != new) {
      for (int y = 0; y < previousStringLength - i; y++) {
//        DDLogVerbose(@"delete: %d", y);
          [[drawView rdp] sendScancode:0x0E withFlag:KBD_FLAGS_DOWN];
          [[drawView rdp] sendScancode:0x0E withFlag:KBD_FLAGS_RELEASE];
      }
      previousString = [previousString substringToIndex:i];
      previousStringLength = [previousString length];
        if (new != 32) [[drawView rdp] sendUnicode:new];
    } else if (!old && new) {

      if (new != 32) { 
          [[drawView rdp] sendUnicode:new];
//        DDLogVerbose(@"send: %d", new);
      } else {
//        DDLogVerbose(@"nothing sent");
      }
    } else if (!new && old) {
//      DDLogVerbose(@"delete (new lost some char)");
        [[drawView rdp] sendScancode:0x0E withFlag:KBD_FLAGS_DOWN];
        [[drawView rdp] sendScancode:0x0E withFlag:KBD_FLAGS_RELEASE];
    }
    old = new = 0;
  }
  if (shouldChangedWasCalled) {
    shouldChangedWasCalled = NO;
  }
  previousString = [[textField text] retain];
}


- (void)resetTextField {
  preventNotification = YES;
  [textField setText:@" "];
  previousString = [[textField text] retain];
  preventNotification = NO;
}

/*
#pragma mark Zoom
- (CGRect)zoomRectForScrollView:(UIScrollView *)scrollView withScale:(float)scale withCenter:(CGPoint)center {
	
  CGRect zoomRect;
	
  // The zoom rect is in the content view's coordinates.
  // At a zoom scale of 1.0, it would be the size of the
  // imageScrollView's bounds.
  // As the zoom scale decreases, so more content is visible,
  // the size of the rect grows.
  zoomRect.size.height = scrollView.frame.size.height / scale;
  zoomRect.size.width  = scrollView.frame.size.width  / scale;
	
  // choose an origin so as to get the right center.
  zoomRect.origin.x = center.x - (zoomRect.size.width  / 2.0);
  zoomRect.origin.y = center.y - (zoomRect.size.height / 2.0);
	
  return zoomRect;
}
*/

#pragma mark Custom toolbar for the Keyboard
- (void)keyboardWillShow:(NSNotification *)note {
  
  DDLogVerbose(@"keyboard will show");
  NSDictionary *info = [note userInfo];
  
  NSValue *beginRectValue = [info objectForKey:UIKeyboardFrameBeginUserInfoKey];
  NSValue *endRectValue = [info objectForKey:UIKeyboardFrameEndUserInfoKey];
  CGRect beginRect, endRect;
  
  [beginRectValue getValue:&beginRect];
  [endRectValue getValue:&endRect];
  
  beginRect = [[self window] convertRect:beginRect toView:drawView];
  endRect = [[self window] convertRect:endRect toView:drawView];    
  
  NSValue *valueDuration = [info objectForKey:UIKeyboardAnimationDurationUserInfoKey];
  NSTimeInterval duration = 0;
  [valueDuration getValue:&duration];
  
  
  [UIView animateWithDuration:duration animations:^(void) {
    /*float zoom = [[NSUserDefaults standardUserDefaults] floatForKey:kZoom];
    DLog(@"width; %.0f height: %.0f height: %d", width * zoom, height * zoom + endRect.size.height, height);
    self.contentSize = CGSizeMake(width * zoom, height * zoom + endRect.size.height);*/

    [textField setFrame:CGRectMake(width / 2, (height + endRect.size.height) / 2, 0, 0)];
  }];
  
  
  DDLogVerbose(@"after set content size");
  
  //    CGRect keyboardToolbarFrame = CGRectMake(0, - 40, endRect.size.width, 40);
  //   DDLogVerbose(@"toolbar: %.0f %.0f %.0f %.0f", keyboardToolbarFrame.origin.x, keyboardToolbarFrame.origin.y, keyboardToolbarFrame.size.width, keyboardToolbarFrame.size.height);
  //
  //    [keyboardToolbar setFrame:keyboardToolbarFrame];
  
  
  
  //    for (UIWindow *keyboardWindow in [[UIApplication sharedApplication] windows]) {
  //		
  //        // Now iterating over each subview of the available windows
  //        for (UIView *keyboard in [keyboardWindow subviews]) {
  //            DDLogVerbose(@"keyboard: %@", keyboard);
  //
  //            // Check to see if the description of the view we have referenced is UIKeyboard.
  //            // If so then we found the keyboard view that we were looking for.
  //            if([[keyboard description] hasPrefix:@"<UIKeyboard"] == YES) {
  //				NSValue *v = [[note userInfo] valueForKey:UIKeyboardBoundsUserInfoKey];
  //				CGRect kbBounds = [v CGRectValue];
  //				
  //				if(keyboardToolbar == nil) {
  //					keyboardToolbar = [[UIToolbar alloc] initWithFrame:CGRectZero];
  //					keyboardToolbar.barStyle = UIBarStyleBlackTranslucent;
  //					
  //					UIBarButtonItem *barButtonItem = [[UIBarButtonItem alloc] initWithTitle:@"Dismiss" style:UIBarButtonItemStyleBordered target:self action:@selector(dismissKeyboard:)];
  //					NSArray *items = [[NSArray alloc] initWithObjects:barButtonItem, nil];
  //					[keyboardToolbar setItems:items];	
  //                    [barButtonItem release];
  //					[items release];	
  //				}				
  //				
  //				[keyboardToolbar removeFromSuperview];
  //				keyboardToolbar.frame = CGRectMake(0, 0, kbBounds.size.width, 30);
  //				[keyboard addSubview:keyboardToolbar];
  //				keyboard.bounds = CGRectMake(kbBounds.origin.x, kbBounds.origin.y, kbBounds.size.width, kbBounds.size.height + 60);
  //				
  //				for(UIView* subKeyboard in [keyboard subviews]) {
  //					if([[subKeyboard description] hasPrefix:@"<UIKeyboardImpl"] == YES) {
  //						subKeyboard.bounds = CGRectMake(kbBounds.origin.x, kbBounds.origin.y - 30, kbBounds.size.width, kbBounds.size.height);	
  //					}						
  //				}
  //            }
  //        }
  //    }    
  
  //    [self addSubview:keyboardToolbar];
}

- (void)keyboardWillHide:(NSNotification *)note;
{
  DDLogVerbose(@"keyboard will hide");
  NSDictionary *info = [note userInfo];
  
  NSValue *valueDuration = [info objectForKey:UIKeyboardAnimationDurationUserInfoKey];
  NSTimeInterval duration = 0;
  [valueDuration getValue:&duration];
  
  
  [UIView animateWithDuration:duration animations:^(void) {
    /*float zoom = [[NSUserDefaults standardUserDefaults] floatForKey:kZoom];
    self.contentSize = CGSizeMake(width * zoom, height * zoom);*/
    
    [textField setFrame:CGRectMake(width / 2, height/ 2, 0, 0)];
  }];
}


#pragma mark OVDFloatingToolbarDelegate


- (void)touchHelp {
  OVDHelpViewController *c = [[[OVDHelpViewController alloc] initWithNibName:@"OVDHelpView" bundle:nil] autorelease];
  [c setModalPresentationStyle:UIModalPresentationFormSheet];
  
  OVDClientViewController *viewcontroller = (OVDClientViewController *)[[self superview] nextResponder];
  [viewcontroller presentModalViewController:c animated:YES];
}


- (void)touchLogout {
	UIAlertView *alert = [[UIAlertView alloc]
												initWithTitle: NSLocalizedString(@"Logout",nil)
												message: NSLocalizedString(@"Do you want to disconnect the current session ?",nil)
												delegate: self
												cancelButtonTitle: NSLocalizedString(@"No",nil)
												otherButtonTitles: NSLocalizedString(@"Yes",nil), nil];
	[alert show];
	[alert release];
}

// Called when an alertview button is touched
- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
	if (buttonIndex == 1) {
		[self logout];
	}
}



- (void)logout {
  DDLogInfo(@"Logout was asked. Exiting...");
  [[drawView rdp] stopSession];
}


#pragma mark -
#pragma mark KVO
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
  if ([keyPath isEqualToString:@"self.bounds"]) {
//    DDLogInfo(@"keyPath: %@ width: %.0f", keyPath, self.bounds.size.width * self.zoomScale);
    /*currentViewWidth = self.bounds.size.width * self.zoomScale;
    [self updateToolbarWidth];*/
  }
}

#pragma mark -
#pragma mark UIScrollView Delegate
- (void)scrollViewDidZoom:(UIScrollView *)scrollView
{
  /*currentViewWidth = self.bounds.size.width * self.zoomScale;
  [self updateToolbarWidth];*/
}

- (void)scrollViewWillBeginZooming:(UIScrollView *)scrollView withView:(UIView *)view;
{
  [panGR setEnabled:NO];
}
- (void)scrollViewDidEndZooming:(UIScrollView *)scrollView withView:(UIView *)view atScale:(float)scale; 
{
  [panGR setEnabled:YES];
}

- (void)scrollViewWillBeginDragging:(UIScrollView *)scrollView
{
  [pinchGR setEnabled:NO];
}
- (void)scrollViewDidEndDragging:(UIScrollView *)scrollView willDecelerate:(BOOL)decelerate
{
  [pinchGR setEnabled:YES];
}

#pragma mark -
#pragma mark Toolbar
- (void) toggleToolbar {
  if (!toolbarDisplayed) {
    toolbarDisplayed = YES;
//    DDLogVerbose(@"--- currentViewWidth: %.0f", currentViewWidth);
		currentViewWidth = self.bounds.size.width;
    toolbar.view.frame = CGRectMake(0, -HEIGHT_TOOLBAR, currentViewWidth, HEIGHT_TOOLBAR);
    [[toolbar view] setAlpha:0.0];
    [[self superview] addSubview:[toolbar view]];
    
    [UIView animateWithDuration:0.3 animations:^{
      toolbar.view.frame = CGRectMake(0, 0, currentViewWidth, HEIGHT_TOOLBAR);            
      [[toolbar view] setAlpha:1.0];
    }];
  } else {
    toolbarDisplayed = NO;
    
    [UIView animateWithDuration:0.3 animations:^{
      toolbar.view.frame = CGRectMake(0, -HEIGHT_TOOLBAR, currentViewWidth, HEIGHT_TOOLBAR); 
      [[toolbar view] setAlpha:0.0];
    }];
    [[toolbar view] removeFromSuperview];
  }
}

/*
- (void)updateToolbarWidth
{
  if (toolbarDisplayed) {
    DDLogVerbose(@"currentViewWidth: %.0f", currentViewWidth);
    CGRect toolbarFrame = [[toolbar view] frame];
    toolbarFrame.size.width = currentViewWidth;
    [[toolbar view] setFrame:toolbarFrame];
  }
}
*/
 
@end

CGPoint midpointBetweenPoints(CGPoint a, CGPoint b) {
  CGFloat x = (a.x + b.x) / 2.0;
  CGFloat y = (a.y + b.y) / 2.0;
  return CGPointMake(x, y);
}
