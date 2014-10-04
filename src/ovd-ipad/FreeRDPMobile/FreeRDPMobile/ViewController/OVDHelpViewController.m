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


#import "OVDHelpViewController.h"
#import "GRMustache.h"

@implementation OVDHelpViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
  self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
  if (self) {
    NSError *error;
    NSString *path = [[NSBundle mainBundle] pathForResource:@"help" ofType:@"mustache"];
    
    NSString *imagePath = [[NSBundle mainBundle] resourcePath];
    imagePath = [imagePath stringByReplacingOccurrencesOfString:@"/" withString:@"//"];
    imagePath = [imagePath stringByReplacingOccurrencesOfString:@" " withString:@"%20"];
    
    baseurl = [NSURL URLWithString: [NSString stringWithFormat:@"file:/%@//",imagePath]];
//    html = [NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:&error];
    
    strings = [NSDictionary dictionaryWithObjectsAndKeys:
               _(@"Gestures"), @"Gestures",
               _(@"Main gestures"), @"MainGestures", 
               _(@"Click"), @"Click",
               _(@"Right click"), @"RightClick",
               _(@"Show keyboard"), @"ShowKeyboard",
               _(@"Show toolbar"), @"ShowToolbar",
               _(@"View gestures"), @"ViewGestures",
               _(@"Zoom"), @"Zoom",
               _(@"Move view"), @"MoveView",
               _(@"Connection"), @"Connection",
               _(@"To log in, you need to enter your login, your password and the server name you wish to connect."), @"LogInText",
               _(@"Toolbar"), @"Toolbar",
               _(@"The question mark shows this help sheet. The cross exits the session."), @"ToolbarText",
               nil];
    html = [GRMustacheTemplate renderObject:strings fromContentsOfFile:path error:&error];
    if (!html) {
      DDLogError(@"Error during rendering: %@", error);
    }
  }
  return self;
}

- (void)dealloc
{
  DDLogWarn(@"---");
//  DESTROY(strings);
//  DESTROY(baseurl);
//  DESTROY(html);
  [super dealloc];
}

- (void)didReceiveMemoryWarning
{
  // Releases the view if it doesn't have a superview.
  [super didReceiveMemoryWarning];
  DDLogError(@"---");
  
  // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle
- (void)viewDidLoad
{
  DDLogVerbose(@"---");
  [super viewDidLoad];
  
  // Load HTML
  [webView loadHTMLString:html baseURL:baseurl];
}

- (void)viewDidUnload
{
  DDLogVerbose(@"---");
  [super viewDidUnload];
  [webView release];
  [bgImageView release];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
  return YES;
}

- (IBAction)dismissView:(id)sender
{
  DDLogVerbose(@"---");
  [self dismissModalViewControllerAnimated:YES];
}

@end
