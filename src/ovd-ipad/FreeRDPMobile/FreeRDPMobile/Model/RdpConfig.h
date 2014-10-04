/**
 * Copyright (C) 2011-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author Harold LEBOULANGER <harold@ulteo.com> 2011
 * Author Clement BIZEAU <cbizeau@ulteo.com> 2011
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


@interface RdpConfig : NSObject {
  NSString *user;
  NSString *password;
  NSString *sm;
  
  NSString *apsUser;
  NSString *apsPassword;
  NSString *apsHost;
  
  NSString *sessionID;
  NSString *sessionStatus;
  
  BOOL gatewayOn;
  NSString *token;
}

//- (id)initWithUser:(NSString *)u password:(NSString *)p sm:(NSString *)sm;

+ (RdpConfig *)sharedConfig;
- (void)reset;
- (BOOL)checkRDPConnection;

@property (retain) NSString *user;
@property (retain) NSString *password;
@property (retain) NSString *sm;

@property (retain) NSString *apsUser;
@property (retain) NSString *apsPassword;
@property (retain) NSString *apsHost;

@property (retain) NSString *sessionID;
@property (retain) NSString *sessionStatus;

@property BOOL gatewayOn;
@property (retain) NSString *token;
@end
