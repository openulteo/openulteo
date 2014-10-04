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


#import "Printer.h"

#import "DrawView.h"
#import "ScrollView.h"
#import "OVDFloatingToolbarViewController.h"
#import "OVDClientViewController.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "rdpdr_constants.h"
#include "rdpdr_types.h"
#include "devman.h"


const char* getPdfSpoolDir() {
	return [NSTemporaryDirectory() UTF8String];
}

@implementation Printer

#pragma mark Class management
- (id)initWithSize:(CGSize)s {
    DDLogVerbose(@"+++");
    
    if ((self = [super init])) {
        //printer = self; // Reference used to let C code access the Obj-C object
        width  = s.width;
        height = s.height;
        
        DDLogVerbose(@"size: %d %d", width, height);
    }
    return self;
}

- (void)dealloc {
    DDLogVerbose(@"---");
    [super dealloc];
}

#pragma mark Printer methods
- (void)printWithData:(NSData *)d andView:(UIView*)aview{
    DDLogVerbose(@"---");
    BOOL printing = [UIPrintInteractionController isPrintingAvailable];
		view = aview;
    if (!printing) DDLogVerbose(@"printing not available"); // FIXME Need to present a popup
    
    BOOL dataOK = [UIPrintInteractionController canPrintData:d];
    if (!dataOK) DDLogVerbose(@"data not ok to be printed"); // FIXME Need to present a popup
    
    [self displayFadeview];
    
    UIPrintInteractionController *controller = [UIPrintInteractionController sharedPrintController];
    [controller setDelegate:self];
    [controller setPrintingItem:d];
    
    [controller presentFromRect:CGRectMake(width/2, height/2, 0, 0) inView:view animated:NO completionHandler:nil];
}

#pragma mark UIPrintInteractionControllerDelegate methods
- (void)printInteractionControllerWillPresentPrinterOptions:(UIPrintInteractionController *)printInteractionController {
    DDLogVerbose(@"---");
}

- (void)printInteractionControllerDidPresentPrinterOptions:(UIPrintInteractionController *)printInteractionController {
    DDLogVerbose(@"---");
}

- (void)printInteractionControllerWillDismissPrinterOptions:(UIPrintInteractionController *)printInteractionController {
    [self removeFadeview];
}

- (void)printInteractionControllerDidDismissPrinterOptions:(UIPrintInteractionController *)printInteractionController {
    DDLogVerbose(@"---");
}

- (void)printInteractionControllerWillStartJob:(UIPrintInteractionController *)printInteractionController {
    DDLogVerbose(@"---");
}

- (void)printInteractionControllerDidFinishJob:(UIPrintInteractionController *)printInteractionController {
    DDLogVerbose(@"---");
}

#pragma mark Fade View

- (void)displayFadeview {
    UIView *dv = view;
    fadeView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, width, height)];
    [fadeView setBackgroundColor:[UIColor blackColor]];
    [fadeView setAlpha:0.0];
    [dv addSubview:fadeView];
    
    [UIView animateWithDuration:FADEVIEW_ANIMATION_DURATION animations:^(void) {
        [fadeView setAlpha:FADEVIEW_ALPHA];
    }];
}


- (void)removeFadeview {
    [UIView animateWithDuration:FADEVIEW_ANIMATION_DURATION animations:^(void) {
        [fadeView removeFromSuperview];
    }];
    
}
@end
