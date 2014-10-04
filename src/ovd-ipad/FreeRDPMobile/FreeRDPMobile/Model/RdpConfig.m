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


#import "RdpConfig.h"


@implementation RdpConfig

@synthesize user, password, sm;
@synthesize apsUser, apsPassword, apsHost;
@synthesize sessionID, sessionStatus;
@synthesize gatewayOn, token;

static RdpConfig *_sharedConfig = nil;

+ (RdpConfig*)sharedConfig
{
	@synchronized([RdpConfig class])
	{
		if (!_sharedConfig)
			[[self alloc] init];

		return _sharedConfig;
	}
    
	return nil;
}

+ (id)alloc
{
	@synchronized([RdpConfig class])
	{
//		NSAssert(_sharedConfig == nil, @"Attempted to allocate a second instance of a singleton.");
		_sharedConfig = [super alloc];
		return _sharedConfig;
	}
    
	return nil;
}


- (BOOL)checkRDPConnection {
    return [[self sm] hasPrefix:@"rdp://"];
}


- (void)dealloc
{
    DESTROY(user);
    DESTROY(password);
    DESTROY(sm);

    DESTROY(apsHost);
    DESTROY(apsPassword);
    DESTROY(apsUser);
    
    DESTROY(sessionID);
    DESTROY(sessionStatus);
    
    DESTROY(token);
    
    [super dealloc];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<User: %@ Password: %@ SM: %@ | APSUser: %@ APSPass: %@ APSHost: %@ | SessionID: %@ SessionStatus: %@ GatewayOn: %d, Token: %@>", user, password, sm, apsUser, apsPassword, apsHost, sessionID, sessionStatus, gatewayOn, token];
}

- (void)reset {
    DESTROY(user);
    DESTROY(password);
    DESTROY(sm);
    
    DESTROY(apsHost);
    DESTROY(apsPassword);
    DESTROY(apsUser);
    
    DESTROY(sessionID);
    DESTROY(sessionStatus);
    
    DESTROY(token);
    gatewayOn = NO;
}


@end
