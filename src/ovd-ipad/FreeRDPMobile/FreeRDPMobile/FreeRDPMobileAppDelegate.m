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

#import "FreeRDPMobileAppDelegate.h"
#import "RdpConfig.h"

#import <UIKit/UIPasteboard.h>

@implementation FreeRDPMobileAppDelegate


@synthesize window=_window;
@synthesize splitViewController;
@synthesize leftViewController;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
  DDLogVerbose(@"---");
  // Override point for customization after application launch.
  [self.window makeKeyAndVisible];
  [self setupDefaultsSettings];
 
  
    
  [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(pasteboardChangedNotification:) name:UIPasteboardChangedNotification object:[UIPasteboard generalPasteboard]];
  [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(pasteboardChangedNotification:) name:UIPasteboardRemovedNotification object:[UIPasteboard generalPasteboard]];
    
  return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
  DDLogVerbose(@"---");
  /*
   Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
   Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
   */
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
  DDLogVerbose(@"---");
  /*
   Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
   If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
   */
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
  DDLogVerbose(@"---");
  /*
   Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
   */
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
  DDLogVerbose(@"---");
  /*
   Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
   */
    
    if (pasteboardChangeCount != [UIPasteboard generalPasteboard].changeCount) {
        [[NSNotificationCenter defaultCenter] postNotificationName:NEW_PASTBOARD_EVENT object:[UIPasteboard generalPasteboard]];
        pasteboardChangeCount = [UIPasteboard generalPasteboard].changeCount;
    }
}

- (void)applicationWillTerminate:(UIApplication *)application
{
  DDLogVerbose(@"---");
  /*
   Called when the application is about to terminate.
   Save data if appropriate.
   See also applicationDidEnterBackground:.
   */
}

- (void)dealloc
{
  DDLogVerbose(@"---");
  [_window release];
  [splitViewController release];
  [super dealloc];
}

- (NSString *)copyright;
{
  DDLogVerbose(@"---");
  NSString *name = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleDisplayName"];
  NSString *cn = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CompanyName"];
  NSString *version = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"];
  NSString *year = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CopyrightYear"];
  
  return [NSString stringWithFormat:@"%@ %@ - \u00A9 %@ %@", name, version, cn, year];
}

- (void)setupDefaultsSettings
{
  DDLogVerbose(@"---");
  NSUserDefaults *d = [NSUserDefaults standardUserDefaults];
  int width, height;
  float zoom;
  float scale = [[UIScreen mainScreen] scale];
  
  if ([[NSUserDefaults standardUserDefaults] boolForKey: @"native_resolution"] ) {
    zoom = 1.0;
    width = [[UIScreen mainScreen] bounds].size.height * scale * zoom;
    height = [[UIScreen mainScreen] bounds].size.width * scale * zoom;
  } else {
		zoom = 1.0; // scale;
		if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone) {
			if (scale < 2.0)
				zoom = 2.0 / scale;
		}
    
    width = [[UIScreen mainScreen] bounds].size.height * zoom;
    height = [[UIScreen mainScreen] bounds].size.width * zoom;
  }
  
  [d setInteger:width forKey:kWidth];
  [d setInteger:height  forKey:kHeight];
}


- (void)pasteboardChangedNotification:(NSNotification*)notification {
    pasteboardChangeCount = [UIPasteboard generalPasteboard].changeCount;
}

@end
