/**
 * Copyright (C) 2011-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author Harold LEBOULANGER <harold@ulteo.com> 2011
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
 **/



#import "OVDClientViewController.h"
#import "ScrollView.h"

@implementation OVDClientViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
  DDLogVerbose(@"---");
  self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
  if (self) {
    landscapeLocked = YES;
  }
  return self;
}

- (void)dealloc
{
  [super dealloc];
}

- (void)didReceiveMemoryWarning
{
  // Releases the view if it doesn't have a superview.
  [super didReceiveMemoryWarning]; DDLogError(@"---");
  
  // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

- (void)viewDidLoad
{
  DDLogVerbose(@"---");
  [super viewDidLoad];
  [scrollView setClientViewController:self];
  
  // It's the only place where we can set the zoom scale. Otherwize the view is not totally loaded
  //float zoom = [[NSUserDefaults standardUserDefaults] floatForKey:kZoom];
  //[scrollView setZoomScale:scrollView.minimumZoomScale];
}

- (void)viewDidUnload
{
  [super viewDidUnload];
  
  DESTROY(scrollView);
}

- (void)viewWillDisappear:(BOOL)animated {
  DDLogVerbose(@"---");
  [super viewWillDisappear:animated];
  [[NSNotificationCenter defaultCenter] removeObserver:scrollView name:UITextFieldTextDidChangeNotification object:nil];
}

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];
  [[NSNotificationCenter defaultCenter] addObserver:scrollView
                                           selector:@selector(keyPressed:) 
                                               name:UITextFieldTextDidChangeNotification 
                                             object:nil];
  [self performSelector:@selector(removeLandscapeLock) withObject:nil afterDelay:TIMEOUT_LANDSCAPE_LOCK];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
  if (landscapeLocked) {
    return UIDeviceOrientationIsLandscape(interfaceOrientation);
  } else {
    return YES;
  }
}

- (void)logout
{
  [scrollView logout];
}

- (void)removeLandscapeLock
{
  DDLogVerbose(@"---");
  landscapeLocked = NO;
}

@end
