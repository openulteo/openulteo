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
#import "OVDProfileViewController.h"
#import "OVDClientViewController.h"
#import "OVDSessionManager.h"
#import "OVDSplitViewController.h"
#import "OVDHelpViewController.h"
#import "OVDLeftViewController.h"

#import "Reachability.h"

#define TABLEVIEW_IPHONE_HEIGHT      442
#define TABLEVIEW_IPHONE_REAL_HEIGHT 320
#define TABLEVIEW_IPHONE_WIDTH       320

#define TABLEVIEW_HEADER_HEIGHT 19

#import "RdpConfig.h"

#define ALERT(TITLE, MSG)     UIAlertView* alertView = [[UIAlertView alloc] initWithTitle:TITLE message:MSG delegate:self cancelButtonTitle:@"OK" otherButtonTitles:nil]; \
[alertView show]; \
[alertView release]; 

#define TRIM(STR) [STR stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]]

#define kSection 3

#define kLoginSection 0
#define kLoginCell 3
#define kLoginUserCell 0
#define kLoginPassCell 1
#define kLoginSMCell 2

#define kLoginButtonSection 1
#define kLoginButtonCell 1

#define kHelpSection 2
#define kHelpCell 1

#define kUsername 0
#define kPassword 1
#define kSM 2

#define TEXTLABEL ((UILabel *)[cell viewWithTag:100])
#define TEXTFIELD ((UITextField *)[cell viewWithTag:101])

#import <AVFoundation/AVFoundation.h>

@interface OVDProfileViewController (hidden)
- (void)updateConfig;
- (NSString *)textForCellAtIndexPath:(NSIndexPath *)i;
- (UITextField *)textFieldForCellAtIndexPath:(NSIndexPath *)i;
- (BOOL)checkConfig;
- (void)clearPasswordField;
@end

@implementation OVDProfileViewController

@synthesize tableView;

#pragma mark -
#pragma mark View lifecycle

- (void)viewDidLoad
{
  DDLogVerbose(@"---");
  [super viewDidLoad];
  
  sessionManager = [[OVDSessionManager alloc] init];
  [sessionManager setDelegate:self];
  
  // Setup reachability checking
  [[NSNotificationCenter defaultCenter] addObserver:self 
                                           selector:@selector(networkReachabilityEvent:) 
                                               name:kReachabilityChangedNotification 
                                             object:nil];
  
  internetConnection = [[Reachability reachabilityForInternetConnection] retain];
  [internetConnection startNotifier];
  
  // The iPhone screen is too small, so we adapt the table view size when the keyboard is shown
  if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone) {
    [[NSNotificationCenter defaultCenter] addObserver:self 
                                             selector:@selector(keyboardWillShow:)
                                                 name:UIKeyboardWillShowNotification 
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(keyboardWillHide:)
                                                 name:UIKeyboardWillHideNotification
                                               object:nil];
  }
  [tableView reloadData];
  [self performSelector:@selector(networkReachabilityEvent:) withObject:nil afterDelay:0.5];
}

- (void)viewWillAppear:(BOOL)animated {
  DDLogVerbose(@"---");
  [super viewWillAppear:animated];
}

- (void)viewDidUnload
{
  DDLogVerbose(@"---");
  [super viewDidUnload];
  DESTROY(sessionManager);
  DESTROY(internetConnection);
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation 
{
  return YES;
}

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration;
{
  DDLogVerbose(@"---");
  [tableView reloadData];
}

- (void)login
{
  DDLogInfo(@"---");
  [self updateConfig];
  BOOL ret = [self checkConfig];  
  
  if (!ret) {
    DDLogWarn(@"Error with config");
    return; 
  }
  
  [self enableLoginButton:NO];
  [self displayErrorMessage:_(@"Connecting to the Session Manager")];
  
  NSString *server;
  
  if ([self checkRDPConnection]) {
    DDLogInfo(@"RDP connection without SM");
    server = [[RdpConfig sharedConfig] sm];
    server = [server stringByReplacingOccurrencesOfString:@"rdp://" withString:@""];
    [[RdpConfig sharedConfig] setApsHost:server];
    [[RdpConfig sharedConfig] setApsUser:[[RdpConfig sharedConfig] user]];
    [[RdpConfig sharedConfig] setApsPassword:[[RdpConfig sharedConfig] password]];
    
    [self checkStatusDone];
  } else {
    DDLogInfo(@"Starting SM session");
    [sessionManager startSessionWithLicensing];
  }
}

- (void)checkStatus
{
  DDLogInfo(@"Checking session status");
  [sessionManager checkStatus];
}

#pragma mark -
#pragma mark Called from the view controller
- (void)hideKeyboard
{
  if (currentTextField) {
    [currentTextField resignFirstResponder];
    currentTextField = nil;
  }
}

#pragma mark -
#pragma mark OVDSessionManager Delegate
- (void)startSessionDone
{
  DDLogVerbose(@"---");
  [self checkStatus];
}

- (void)checkStatusDone
{
  DDLogVerbose(@"---");
  DDLogInfo(@"Start connection to the RDP server");
  DDLogVerbose(@"Config: %@", [RdpConfig sharedConfig]);
  [self displayErrorMessage:_(@"Start loading session")];
  
  clientViewController = [[[OVDClientViewController alloc] initWithNibName:@"OVDClientView" bundle:nil] autorelease];
  [clientViewController setModalPresentationStyle:UIModalPresentationFullScreen];
  [clientViewController setModalTransitionStyle:UIModalTransitionStyleFlipHorizontal];
  
  [rootViewController presentModalViewController:clientViewController animated:YES];
  
  // Cleanup
  [self clearPasswordField];
  [self enableLoginButton:YES];
  [self displayErrorMessage:@""];
}

- (void)logoutDone
{
	if (clientViewController) {
		[clientViewController logout];
		
	}
  // Cleanup
  [self clearPasswordField];
  [self enableLoginButton:YES];
  [self displayErrorMessage:_(@"You have been disconnected")];
}

- (void)error:(NSString *)e
{
  DDLogInfo(@"Error: %@", e);
  [self enableLoginButton:YES];
  [self displayErrorMessage:e];
}

- (void)resetErrorMessage
{
  [self networkReachabilityEvent:nil];
}

#pragma mark -
#pragma mark Table view data source methods

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tv {
  tableView = tv;
  return kSection;
  //    return (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) ? kSection - 1 : kSection;
}

- (NSInteger)tableView:(UITableView *)tv numberOfRowsInSection:(NSInteger)section {
  switch(section) {
    case kLoginSection:
      return kLoginCell;
    case kLoginButtonSection:
      return kLoginButtonCell;
    case kHelpSection:
      return kHelpCell;
    default:
      return 0;
  }
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)idx {
  return @"";
}

- (NSString *)tableView:(UITableView *)tableView titleForFooterInSection:(NSInteger)section;
{
  //    DDLogVerbose(@"section: %d", section);
  if (section == kLoginSection) {
    return currentLoginStatus;
  } else {
    return @"";
  }
}


- (UITableViewCell *)tableView:(UITableView *)tv
         cellForRowAtIndexPath:(NSIndexPath *)indexPath {
  
	UITableViewCell *cell;
  static NSString *cellID;
  if (indexPath.section == kLoginSection) {
    cellID = @"CellEditText";
    
    cell = [tv dequeueReusableCellWithIdentifier:cellID];
    
    if (cell == nil) {
      cell = [[[NSBundle mainBundle] loadNibNamed:cellID owner:self options:nil] lastObject];
      [cell setAccessoryType:UITableViewCellAccessoryNone];
      [cell setSelectionStyle:UITableViewCellSelectionStyleNone];
    }
  } else {
    cellID = @"ProfileCell";
    
    cell = [tv dequeueReusableCellWithIdentifier:cellID];
    if (cell == nil) {
      cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:cellID] autorelease];
      [cell setAccessoryType:UITableViewCellAccessoryNone];
      [cell setBackgroundView:nil];
    }
  }
  
	switch (indexPath.section) {
    case kLoginSection:
    {
      [TEXTFIELD setText:@""];
      [TEXTFIELD setDelegate:self];
      [TEXTLABEL setText:@""];
      
      NSString *string = @"";
      switch(indexPath.row) {
        case kUsername: {
          string = _(@"Login"); 
          TEXTFIELD.text = [[NSUserDefaults standardUserDefaults] stringForKey:@"username"];
          [[RdpConfig sharedConfig] setUser:TEXTFIELD.text];
          break;
        }
        case kPassword: {
          string =_(@"Password");
          if ([[NSUserDefaults standardUserDefaults] boolForKey: @"save_password"] ) {
            TEXTFIELD.text = [[NSUserDefaults standardUserDefaults] stringForKey:@"password"];
            [[RdpConfig sharedConfig] setSm:TEXTFIELD.text];
          }
          [TEXTFIELD setSecureTextEntry:YES];
          break;
        }
        case kSM:       {
          string = _(@"Session Manager"); 
          TEXTFIELD.text = [[NSUserDefaults standardUserDefaults] stringForKey:@"sm"];
          [[RdpConfig sharedConfig] setSm:TEXTFIELD.text];
          break;
        }
      }
      [TEXTLABEL setText:string];
      
      CGSize width = [string sizeWithFont:[TEXTLABEL font]];
      CGRect labelRect = CGRectMake(TEXTLABEL.frame.origin.x, 
                                    TEXTLABEL.frame.origin.y, 
                                    width.width, 
                                    TEXTLABEL.frame.size.height);
      [TEXTLABEL setFrame:labelRect];
      
      CGRect textFieldRect = CGRectMake(TEXTLABEL.frame.origin.x + TEXTLABEL.frame.size.width + 10, 
                                        TEXTFIELD.bounds.origin.y + 12, 
                                        cell.frame.size.width - TEXTLABEL.frame.size.width - 44.0, 
                                        TEXTLABEL.frame.size.height);
      [TEXTFIELD setFrame:textFieldRect];
      
      break;
    }
    case kLoginButtonSection:
    {
      if (indexPath.row == 0) {
        cell.textLabel.text = _(@"Start!");
        cell.textLabel.textAlignment = UITextAlignmentCenter;
        cell.textLabel.font = [UIFont boldSystemFontOfSize:16.0];
      } 
      break;
    }
    case kHelpSection:
    {
      if (indexPath.row == 0) {
        cell.textLabel.text = _(@"Help");
        cell.textLabel.textAlignment = UITextAlignmentCenter;
        cell.textLabel.font = [UIFont boldSystemFontOfSize:16.0];
      }
      break;
    }
  }
  return cell;
}

- (void)tableView:(UITableView *)tv didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
  [tv deselectRowAtIndexPath:indexPath animated:YES];
  
  switch (indexPath.section) {
    case kLoginSection: 
    {
      UITableViewCell *cell = [tv cellForRowAtIndexPath:indexPath];
      [TEXTFIELD becomeFirstResponder];
      break;
    }
    case kLoginButtonSection:
    {
      if (indexPath.row == 0) {
        [self login];
      }
      break;
    }
    case kHelpSection:
      if (indexPath.row == 0) {
        [self displayHelp];
      }
      break;
  }
}

#pragma mark -
#pragma mark Text Field Delegate
- (void)textFieldDidBeginEditing:(UITextField *)textField
{
  UITableViewCell *cell = (UITableViewCell *)[textField superview]; 
  currentTextField = TEXTFIELD;
}

- (void)textFieldDidEndEditing:(UITextField *)textField
{
  currentTextField = nil;
}

- (BOOL)textFieldShouldReturn:(UITextField *)tf {
  if(!preventLogin)
    [self login];
  return YES;
}

- (void)updateConfig
{
  [currentTextField resignFirstResponder];
  NSIndexPath *i;
  
  i = [NSIndexPath indexPathForRow:kLoginUserCell inSection:kLoginSection];
  [[RdpConfig sharedConfig] setUser:[self textForCellAtIndexPath:i]];
  [[NSUserDefaults standardUserDefaults] setValue:[self textForCellAtIndexPath:i] forKey:@"username"];
  
  i = [NSIndexPath indexPathForRow:kLoginPassCell inSection:kLoginSection];
  [[RdpConfig sharedConfig] setPassword:[self textForCellAtIndexPath:i]];
  if ([[NSUserDefaults standardUserDefaults] boolForKey: @"save_password"] ) {
    [[NSUserDefaults standardUserDefaults] setValue:[self textForCellAtIndexPath:i] forKey:@"password"];
  }
  
  i = [NSIndexPath indexPathForRow:kLoginSMCell inSection:kLoginSection];
  [[RdpConfig sharedConfig] setSm:[self textForCellAtIndexPath:i]];
  [[NSUserDefaults standardUserDefaults] setValue:[self textForCellAtIndexPath:i] forKey:@"sm"];
  
  DDLogVerbose(@"config: %@", [RdpConfig sharedConfig]);
}

- (BOOL)checkConfig
{
  NSString *user = [[RdpConfig sharedConfig] user];
  NSString *pass = [[RdpConfig sharedConfig] password];
  NSString *sm   = [[RdpConfig sharedConfig] sm];

  if (!user || [TRIM(user) isEqualToString:@""]) {[self displayErrorMessage:_(@"You must specify the host field!")]; return NO;}
  if (!pass || [TRIM(pass) isEqualToString:@""]) {[self displayErrorMessage:_(@"You must specify a password!")]; return NO;}
  if (!sm   || [TRIM(sm)   isEqualToString:@""]) {[self displayErrorMessage:_(@"You must specify the host field!")];       return NO;}
  
  return YES;
}

- (BOOL)checkRDPConnection
{
  return [[[RdpConfig sharedConfig] sm] hasPrefix:@"rdp://"];
}

#pragma mark -
#pragma mark Cell TextField Helper

- (NSString *)textForCellAtIndexPath:(NSIndexPath *)i
{
  //    UITableView * tv = (UITableView *)[self view];
  UITableViewCell *cell = [tableView cellForRowAtIndexPath:i];
  return [TEXTFIELD text];
}

- (UITextField *)textFieldForCellAtIndexPath:(NSIndexPath *)i
{
  //    UITableView * tv = (UITableView *)[self view];
  UITableViewCell *cell = [tableView cellForRowAtIndexPath:i];
  return TEXTFIELD;
}

- (void)clearPasswordField;
{
  if (! [[NSUserDefaults standardUserDefaults] boolForKey: @"save_password"] ) {
    NSIndexPath *i = [NSIndexPath indexPathForRow:kLoginPassCell inSection:kLoginSection];
    [[self textFieldForCellAtIndexPath:i] setText:@""]; // Cleaning the password field
  }
}

#pragma mark -
#pragma mark Help
- (void)displayHelp
{
  OVDHelpViewController *vc = [[[OVDHelpViewController alloc] initWithNibName:@"OVDHelpView" bundle:nil] autorelease];
  [vc setModalTransitionStyle:UIModalTransitionStyleCoverVertical];
  
  if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) {
    [vc setModalPresentationStyle:UIModalPresentationFormSheet];
  }
    
  [rootViewController presentModalViewController:vc animated:YES];
}

#pragma mark -
#pragma mark TableView Height with Keyboard
- (void)keyboardWillShow:(NSNotification *)note {
  DDLogVerbose(@"---");
  NSDictionary *info = [note userInfo];
  
  CGRect keyboardFrame;
  [[info objectForKey:UIKeyboardFrameEndUserInfoKey] getValue:&keyboardFrame];
  keyboardFrame = [[[self view] window] convertRect:keyboardFrame toView:[self view]];
  CGFloat keyboardHeight = keyboardFrame.size.height;
  
  CGRect windowFrame = [[[self view] window] frame];
  windowFrame = [[[self view] window] convertRect:windowFrame toView:[self view]];

  NSTimeInterval duration = 0;
  [[info objectForKey:UIKeyboardAnimationDurationUserInfoKey] getValue:&duration];
  
  CGRect tableViewFrame = [tableView frame];

  tableViewFrame.size.height = (windowFrame.size.height - 20) - keyboardHeight;

  
  [UIView animateWithDuration:duration animations:^(void) {
    [tableView setFrame:tableViewFrame];
  }];
}

- (void)keyboardWillHide:(NSNotification *)note;
{
  NSDictionary *info = [note userInfo];
  
  CGRect keyboardFrame;
  [[info objectForKey:UIKeyboardFrameEndUserInfoKey] getValue:&keyboardFrame];
  
  keyboardFrame = [[[self view] window] convertRect:keyboardFrame toView:[self view]];
  
  CGRect windowFrame = [[[self view] window] frame];
  windowFrame = [[[self view] window] convertRect:windowFrame toView:[self view]];
  
  NSTimeInterval duration = 0;
  [[info objectForKey:UIKeyboardAnimationDurationUserInfoKey] getValue:&duration];
  
  CGRect tableViewFrame = [tableView frame];
  tableViewFrame.size.height = (windowFrame.size.height - 20);
  
  [UIView animateWithDuration:duration animations:^(void) {
    [tableView setFrame:tableViewFrame];
  }];
}

#pragma mark -
#pragma mark Network status

- (void)networkReachabilityEvent:(NSNotification *) notification
{   
  DDLogVerbose(@"---");
  Reachability *r = internetConnection;
  if ([r isReachable]) {
    [self displayErrorMessage:@""];
    [self enableLoginButton:YES];
    preventLogin = NO;
    if (lostNetworkAlert) {
      [lostNetworkAlert dismissWithClickedButtonIndex:-1 animated:YES];
      DESTROY(lostNetworkAlert);
    }
  } else {
    [self displayErrorMessage:_(@"Network is down")];
    [self enableLoginButton:NO];
    preventLogin = YES;
    if (clientViewController) {
      if (!lostNetworkAlert) {
        lostNetworkAlert = [[UIAlertView alloc] initWithTitle:_(@"Network is down") 
                                                      message:_(@"A network issue prevents the dialog between the client and the server. You can wait until the connection is up again or logout now.") 
                                                     delegate:self 
                                            cancelButtonTitle:_(@"Logout") 
                                            otherButtonTitles:nil];
        [lostNetworkAlert show];
      }
    }
  }
}

- (void)displayErrorMessage:(NSString *)e
{
  currentLoginStatus = e;
  NSIndexSet *is = [NSIndexSet indexSetWithIndex:kLoginSection];
  [tableView reloadSections:is withRowAnimation:UITableViewRowAnimationFade];
}

- (void)enableLoginButton:(BOOL)b
{
  NSIndexPath *ip = [NSIndexPath indexPathForRow:0 inSection:kLoginButtonSection];
  UITableViewCell *cell = [tableView cellForRowAtIndexPath:ip];
  if (b) {
    [cell setUserInteractionEnabled:YES];
    [cell.textLabel setTextColor:[UIColor blackColor]];
  } else {
    [cell setUserInteractionEnabled:NO];
    [cell.textLabel setTextColor:[UIColor grayColor]];
  }
}
#pragma mark -
#pragma mark UIAlertView Delegate
- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex;
{
  DDLogVerbose(@"button idx: %d", buttonIndex);
  if (buttonIndex == [alertView cancelButtonIndex])
    [clientViewController logout];
}

@end
