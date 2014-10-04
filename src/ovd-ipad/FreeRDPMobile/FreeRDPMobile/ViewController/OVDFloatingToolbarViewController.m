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
#import "OVDClientViewController.h"
#import "RdpConfig.h"

@implementation OVDFloatingToolbarViewController

@synthesize clientViewController;
@synthesize delegate;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
  self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
  
  return self;
}

- (void)dealloc
{
    [ApplicationTitle release];
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
  self.view.backgroundColor = [[UIColor alloc] initWithPatternImage:[UIImage imageNamed: @"FloatingToolbarBG"]];
	NSString *title = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"ApplicationTitle"];
	[ApplicationTitle setText:title];

}

- (void)viewDidUnload
{
  DDLogVerbose(@"---");
    [ApplicationTitle release];
    ApplicationTitle = nil;
  [super viewDidUnload];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
  // Return YES for supported orientations
  return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

- (IBAction)logout:(id)sender;
{
  DDLogVerbose(@"---");
  [delegate touchLogout];
}

- (IBAction)help:(id)sender;
{
  DDLogVerbose(@"---");
  [delegate touchHelp];
}

- (IBAction)website:(id)sender;
{
  
}

@end
