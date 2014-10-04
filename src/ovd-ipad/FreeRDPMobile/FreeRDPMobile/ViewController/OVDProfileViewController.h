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


#import <Foundation/Foundation.h>
#import "OVDSessionManager.h"
#import <AVFoundation/AVFoundation.h>

@class OVDClientViewController;
@class OVDSessionManager;
@class OVDSplitViewController;
@class OVDLeftViewController;
@class Reachability;


@interface OVDProfileViewController : UITableViewController <UITableViewDataSource, UITableViewDelegate, UITextFieldDelegate, OVDSessionManagerDelegate, UIAlertViewDelegate> {
  IBOutlet UITableView *tableView;

  OVDSessionManager *sessionManager;
  NSString *currentTextFieldLabel;
  UITextField *currentTextField;
  IBOutlet OVDSplitViewController *splitViewController;
  IBOutlet UIViewController *rootViewController; // Change between iPad and iPhone UI
  OVDClientViewController *clientViewController;
  
  Reachability *internetConnection;
  
  NSString *currentLoginStatus;
  UIAlertView *lostNetworkAlert;
  BOOL preventLogin;
}

@property (retain, nonatomic) UITableView *tableView;

#pragma mark Called from the view controller
- (void)hideKeyboard;

#pragma mark Action
- (void)login;
- (void)displayHelp;

#pragma mark Network status
- (void)networkReachabilityEvent: (NSNotification *) notification;
- (void)resetErrorMessage;
- (void)displayErrorMessage:(NSString *)e;
- (void)enableLoginButton:(BOOL)b;

- (BOOL)checkRDPConnection;

@end
