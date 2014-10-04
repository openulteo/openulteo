/**
 * Copyright (C) 2011-2014 Ulteo SAS
 * http://www.ulteo.com
 * Author Harold LEBOULANGER <harold@ulteo.com> 2011
 * Author Guillaume SEMPE <guillaume.sempe@gmail.com> 2012
 * Author David PHAM-VAN <d.pham-van@ulteo.com> 2013, 2014
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

#import "OVDLicensing.h"


/**
 * Definition of the error code sent back by the SM
 */
#define ERROR_AUTHENTICATION_FAILED      @"auth_failed"
#define ERROR_IN_MAINTENANCE             @"in_maintenance"
#define ERROR_INTERNAL                   @"internal_error"
#define ERROR_INVALID_USER               @"invalid_user"
#define ERROR_SERVICE_NOT_AVAILABLE      @"service_not_available"
#define ERROR_UNAUTHORIZED_SESSION_MODE  @"unauthorized_session_mode"
#define ERROR_ACTIVE_SESSION             @"user_with_active_session"
#define ERROR_DEFAULT                    @"default"

@class TBXML;

/**
 * Let the delegate knows the connection progress
 */
@protocol OVDSessionManagerDelegate <NSObject>
@required

/**
 * Acknowledge the start session order
 */
- (void)startSessionDone;

/**
 * Acknowledge the check status order
 */
- (void)checkStatusDone;

/**
 * Acknowledge the logout order
 */
- (void)logoutDone;

/**
 * Warn that an error has occured
 * @param e the error string
 */
- (void)error:(NSString *)e;
@end

/**
 * Represent the session manager.
 *
 * Connect to the SM, authenticate the user, get session informations 
 * such as the server, the username and the password.
 */
@interface OVDSessionManager : NSObject <OVDLicensingDelegate> {
  NSString *baseURL;            /**< The SM base URL */
  
  NSMutableData *receivedData;  /**< The temporary object that contains the received data */
	int statusCode;
  NSURLConnection *connection;  /**< The connection object */
  
  int activeCommand;
  TBXML *xml;
  
  id <OVDSessionManagerDelegate> delegate;
	
	OVDLicensing *licensing;
	NSString *service_suffix;
}


- (void)startSessionWithLicensing;
- (void)startSession;
- (void)checkStatus;
- (void)logout;

// This method is used to detect if the server is up or down.
// An event is fire after a timeout
- (void)checkServerStatus;
- (void)cancelCheckServerStatus;
- (NSString *)humanReadableError:(NSString *)e;

@property (nonatomic, assign) id <OVDSessionManagerDelegate> delegate;

@end
