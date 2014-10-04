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


#import "OVDLeftViewController.h"
#import "FreeRDPMobileAppDelegate.h"
#import "OVDProfileViewController.h"

#define COPYRIGHT_NOTICE_HEIGHT 18

@implementation OVDLeftViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
  self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
  if (self) {
    
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
  [super didReceiveMemoryWarning];
  
  // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

/*
 // Implement loadView to create a view hierarchy programmatically, without using a nib.
 - (void)loadView
 {
 }
 */


// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad
{
  [super viewDidLoad];
  FreeRDPMobileAppDelegate *appDelegate = (FreeRDPMobileAppDelegate *)[[UIApplication sharedApplication] delegate];
  NSString *copyright = [appDelegate copyright];
  [copyrightNotice setTitle:copyright forState:UIControlStateNormal];
}


- (void)viewDidUnload
{
  [super viewDidUnload];
  // Release any retained subviews of the main view.
  // e.g. self.myOutlet = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
  // Return YES for supported orientations
  return YES;
}

- (void)viewDidAppear:(BOOL)animated {
	[self willRotateToInterfaceOrientation:[UIApplication sharedApplication].statusBarOrientation duration:0];
}


- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration {
  // Hide the keyboard
  [profileViewController hideKeyboard];
  
  // Hide the copyright notice in landscape mode for the iPhone UI
  UITableView *tableView = [profileViewController tableView];
  
  if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone) {
    CGRect tvFrame = tableView.frame;
    CGFloat copyrightNoticeHeight = CGRectGetHeight([copyrightNotice frame]);
    if (toInterfaceOrientation == UIInterfaceOrientationLandscapeLeft || 
        toInterfaceOrientation == UIInterfaceOrientationLandscapeRight) {
      copyrightNotice.hidden = YES;
      tvFrame.size.height += copyrightNoticeHeight;
    } else {
      copyrightNotice.hidden = NO;
      tvFrame.size.height -= copyrightNoticeHeight;
    }
    [tableView setFrame:tvFrame];
  }
  
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation;
{
  UITableView *tableView = [profileViewController tableView];
  [tableView reloadData];
}

- (IBAction)showWebsite:(id)sender
{
  NSString *website = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"WebSite"];
  [[UIApplication sharedApplication] openURL:[NSURL URLWithString: website]];
}

- (void)customPresentModalViewController:(UIViewController*)vc;
{
  
}

@end
