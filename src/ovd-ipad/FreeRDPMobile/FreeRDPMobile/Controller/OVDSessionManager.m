/**
 * Copyright (C) 2011-2014 Ulteo SAS
 * http://www.ulteo.com
 * Author Harold LEBOULANGER <harold@ulteo.com> 2011
 * Author Clement BIZEAU <cbizeau@ulteo.com> 2011
 * Author Guillaume SEMPE <guillaume.sempe@gmail.com> 2012
 * Author David PHAM-VAN <d.pham-van@ulteo.com> 2013, 2014
 * Author Alexandre CONFIANT-LATOUR <a.confiant@ulteo.com> 2014
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


#import "OVDSessionManager.h"
#import "RdpConfig.h"
#import "TBXML.h"


#define kParseStartSession 0
#define kParseStatus 1
#define kParseLogout 2
#define kParseLicensing 3

#define kStatusError @"error"
#define kStatusUnknown @"unknown"
#define kStatusCreating @"creating"
#define kStatusCreated @"created"
#define kStatusInit  @"init"
#define kStatusReady @"ready"
#define kStatusLogged @"logged"
#define kStatusDisconnected @"disconnected"
#define kStatusWaitDestroy @"wait_destroy"
#define kStatusDestroying @"destroying"
#define kStatusDestroyed @"destroyed"

#define kSessionPhaseInit 0
#define kSessionPhaseStarted 1
#define kSessionPhaseDeInit 2
#define kSessionPhaseStop 3

#define kStatusTimeout 40.0
#define kSessionStatusLongDelay 600.0 // 10 min
#define kSessionStatusShortDelay 3.0 // 3 sec


@interface OVDSessionManager () 
- (void)parseXMLStartSession;
- (void)parseXMLStatus;
@end

@implementation OVDSessionManager

@synthesize delegate;

- (void)dealloc
{
  DESTROY(xml);
  DESTROY(baseURL);
  [super dealloc];
}

- (void)startSessionWithLicensing
{
  activeCommand = kParseLicensing;
  NSString *sm       = [[RdpConfig sharedConfig] sm];
  baseURL = [[NSString alloc] initWithFormat:@"https://%@/ovd/client", sm];

	licensing = [OVDLicensing alloc];
	[licensing setDelegate:self];
	NSMutableURLRequest * request = [licensing check:baseURL];
	connection = [[NSURLConnection alloc] initWithRequest:request delegate:self];
  if (!connection) {
    [delegate error:_(@"Unable to reach the Session Manager")];
    DDLogError(@"error while connecting to the SM");
  }
}

- (void)licenseError
{
	[delegate error:[self humanReadableError:ERROR_SERVICE_NOT_AVAILABLE]];
}

- (void)startSession
{
  [self startSessionWithSuffix:@""];
}

- (void)startSessionWithSuffix:(NSString *)suffix
{
  service_suffix = suffix;
  DDLogVerbose(@"---");
  activeCommand = kParseStartSession;
  
  NSString *user     = [[RdpConfig sharedConfig] user];
  NSString *password = [[RdpConfig sharedConfig] password];
  NSString *sm       = [[RdpConfig sharedConfig] sm];
    
  NSString *locale   = [[NSLocale currentLocale] localeIdentifier];

  DDLogVerbose(@"current locale: %@", locale);
    
  NSArray *preferredLocale = [NSLocale preferredLanguages];
  DDLogVerbose(@"preffered locale %@", preferredLocale);
  
  if ([preferredLocale count]) {
    locale = [preferredLocale objectAtIndex:0];
  }
  
  locale = [locale stringByReplacingOccurrencesOfString:@"_" withString:@"-"];
  locale = [locale lowercaseString];
    
  baseURL = [[NSString alloc] initWithFormat:@"https://%@/ovd/client", sm];
  
  NSString *post = [NSString stringWithFormat:@"<session mode=\"desktop\" language=\"%@\"><user login=\"%@\" password=\"%@\"/></session>", locale, user, password];
  DDLogVerbose(@"post: %@", post);
  
  NSString *urlString  = [NSString stringWithFormat:@"%@/start%@", baseURL, service_suffix];
  NSURL    *url        = [NSURL URLWithString:urlString];
  
  NSData   *postData   = [post dataUsingEncoding:NSUTF8StringEncoding];
  NSString *postLength = [NSString stringWithFormat:@"%d", [postData length]];
  
  NSMutableURLRequest *request = [[[NSMutableURLRequest alloc] init] autorelease];
  [request setURL:url];
  [request setHTTPMethod:@"POST"];
  [request setValue:postLength forHTTPHeaderField:@"Content-Length"];
  [request setValue:@"text/xml" forHTTPHeaderField:@"Content-Type"];
  [request setValue:@"deflate" forHTTPHeaderField:@"Accept-Encoding"];
    
  [request setHTTPBody:postData];
  
  connection = [[NSURLConnection alloc] initWithRequest:request delegate:self];
  if (connection) {
    [self performSelector:@selector(checkServerStatus) withObject:nil afterDelay:kStatusTimeout];
  } else {
    [delegate error:_(@"Unable to reach the Session Manager")];
    DDLogError(@"error while connecting to the SM");
  }
  
}

- (void)checkStatus
{
  activeCommand = kParseStatus;
  NSString *urlString = [NSString stringWithFormat:@"%@/session_status%@", baseURL, service_suffix];
  NSURL    *url       = [NSURL URLWithString:urlString];
  
  
  
  NSMutableURLRequest *request = [[[NSMutableURLRequest alloc] init] autorelease];
  [request setURL:url];
  [request setHTTPMethod:@"GET"];
  NSDictionary * cookies = [NSHTTPCookie requestHeaderFieldsWithCookies:[[NSHTTPCookieStorage sharedHTTPCookieStorage] cookies]];
  DDLogVerbose(@"Add Cookie : %@", [cookies objectForKey: @"Cookie"]);
  [request setValue:[cookies objectForKey: @"Cookie"] forHTTPHeaderField:@"Cookie"];
  connection = [[NSURLConnection alloc] initWithRequest:request delegate:self];
  if (connection) {
    [self performSelector:@selector(checkServerStatus) withObject:nil afterDelay:kStatusTimeout];
  } else {
    [delegate error:_(@"Unable to reach the Session Manager")];
    DDLogError(@"error while connecting to the SM");
  }
}

- (void)logout {
  activeCommand = kParseLogout;
  
  NSString *urlString = [NSString stringWithFormat:@"%@/logout%@", baseURL, service_suffix];
  NSURL    *url       = [NSURL URLWithString:urlString];
  
  NSString *post = @"<logout mode=\"logout\"/>";
  NSData   *postData   = [post dataUsingEncoding:NSUTF8StringEncoding];
  NSString *postLength = [NSString stringWithFormat:@"%d", [postData length]];
  
  NSMutableURLRequest *request = [[[NSMutableURLRequest alloc] init] autorelease];
  [request setURL:url];
  [request setHTTPMethod:@"POST"];
  [request setValue:postLength forHTTPHeaderField:@"Content-Length"];
  [request setValue:@"text/xml" forHTTPHeaderField:@"Content-Type"];
  [request setHTTPBody:postData];
	
  NSDictionary * cookies = [NSHTTPCookie requestHeaderFieldsWithCookies:[[NSHTTPCookieStorage sharedHTTPCookieStorage] cookies]];
  DDLogVerbose(@"Add Cookie : %@", [cookies objectForKey: @"Cookie"]);
  [request setValue:[cookies objectForKey: @"Cookie"] forHTTPHeaderField:@"Cookie"];
  
  connection = [[NSURLConnection alloc] initWithRequest:request delegate:self];
  if (connection) {
    DDLogVerbose(@"connection: %@", connection);
    [self performSelector:@selector(checkServerStatus) withObject:nil afterDelay:kStatusTimeout];
  } else {
    //        [delegate error:@"Unable to reach the Session Manager"];
    DDLogError(@"error while connecting to the SM");
  }
}

- (void)checkServerStatus
{
  if (!receivedData) {
    [connection cancel];
    [delegate error:_(@"Unable to reach the Session Manager")];
  }
}

- (void)cancelCheckServerStatus
{
  [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(checkServerStatus) object:nil];
}

- (NSString *)humanReadableError:(NSString *)e
{
  if ([e isEqualToString:ERROR_AUTHENTICATION_FAILED]) {
    return _(@"Authentication failed: please double-check your password and try again");
  } else if ([e isEqualToString:ERROR_IN_MAINTENANCE]) {
    return _(@"The system is on maintenance mode, please contact your administrator for more information");
  } else if ([e isEqualToString:ERROR_INTERNAL]) {
    return _(@"An internal error occured, please contact your administrator");
  } else if ([e isEqualToString:ERROR_INVALID_USER]) {
    return _(@"You specified an invalid login, please double-check and try again");
  } else if ([e isEqualToString:ERROR_SERVICE_NOT_AVAILABLE]) {
    return _(@"The service is not available, please contact your administrator for more information");
  } else if ([e isEqualToString:ERROR_UNAUTHORIZED_SESSION_MODE]) {
    return _(@"You are not authorized to launch a session in this mode");
  } else if ([e isEqualToString:ERROR_ACTIVE_SESSION]) {
    return _(@"You already have an active session");
  }
  return _(@"An error occured, please contact your administrator");
}

#pragma mark -
#pragma mark Parse XML
- (void)parseXMLStartSession
{
  TBXMLElement *server = [TBXML childElementNamed:@"server" parentElement:[xml rootXMLElement]];
  
  if (!server) {
    NSString *errorString = [TBXML valueOfAttributeNamed:@"code" forElement:[xml rootXMLElement]];
    NSString *humanReadable = [self humanReadableError:errorString];
    
    [delegate error:humanReadable];
    return;
  }
  
  NSString *gatewayOn = [TBXML valueOfAttributeNamed:@"mode_gateway" forElement:[xml rootXMLElement]];
  
  if (gatewayOn && [gatewayOn isEqualToString:@"on"]) {
    DDLogVerbose(@"gateway on");
    [[RdpConfig sharedConfig] setGatewayOn:YES];
    NSString *token = [TBXML valueOfAttributeNamed:@"token" forElement:server];
    if (token) {
      [[RdpConfig sharedConfig] setToken:token];
      DDLogVerbose(@"token: %@", token); 
    }
  } else {
		[[RdpConfig sharedConfig] setGatewayOn:NO];
	}
  DLog(@"gateway: %@", gatewayOn);
  
  NSString *aps_user   = [TBXML valueOfAttributeNamed:@"login" forElement:server];
  NSString *aps_pass   = [TBXML valueOfAttributeNamed:@"password" forElement:server];  
  NSString *aps_server = [TBXML valueOfAttributeNamed:@"fqdn" forElement:server];
  
  [[RdpConfig sharedConfig] setApsUser:aps_user];
  [[RdpConfig sharedConfig] setApsPassword:aps_pass];
  
  if (!aps_server || [aps_server isEqualToString:@"127.0.0.1"]) {
    aps_server = [[RdpConfig sharedConfig] sm];
  } 
    
  [[RdpConfig sharedConfig] setApsHost:aps_server];
    
    DDLogVerbose(@"Config: %@", [RdpConfig sharedConfig]);
  
  [delegate startSessionDone];
}

- (void)parseXMLStatus
{
  NSString *session_id = [TBXML valueOfAttributeNamed:@"id" forElement:[xml rootXMLElement]];
  if (!session_id) {
    NSString *errorString = [TBXML valueOfAttributeNamed:@"message" forElement:[xml rootXMLElement]];
    NSString *humanReadable = [self humanReadableError:errorString];
    
    [delegate error:humanReadable];
    return;
  }
  
  NSString *session_status = [TBXML valueOfAttributeNamed:@"status" forElement:[xml rootXMLElement]];  
  
  [[RdpConfig sharedConfig] setSessionID:session_id];
  [[RdpConfig sharedConfig] setSessionStatus:session_status];
  
  DDLogVerbose(@"Check Status: session_id %@ session8status %@", session_id, session_status);
	if ([session_status isEqualToString:kStatusReady]) {
		[delegate checkStatusDone];
	}

	/* Handle session_status polling interval */
	int sessionPhase = -1;

	if([session_status isEqualToString:kStatusInit] ||
	   [session_status isEqualToString:kStatusCreating] ||
	   [session_status isEqualToString:kStatusCreated] ||
	   [session_status isEqualToString:kStatusReady] )
	{
		sessionPhase = kSessionPhaseInit;
	} else if([session_status isEqualToString:kStatusLogged]) {
		sessionPhase = kSessionPhaseStarted;
	} else if([session_status isEqualToString:kStatusWaitDestroy] ||
	          [session_status isEqualToString:kStatusDestroying]) {
		sessionPhase = kSessionPhaseDeInit;
	} else {
		sessionPhase = kSessionPhaseStop;
	}

	if (sessionPhase == kSessionPhaseInit ||
	    sessionPhase == kSessionPhaseDeInit) {
		/* Fast checking */
		[self performSelector:@selector(checkStatus) withObject:nil afterDelay:kSessionStatusShortDelay];
	} else if (sessionPhase == kSessionPhaseStarted) {
		/* Slow checking */
		[self performSelector:@selector(checkStatus) withObject:nil afterDelay:kSessionStatusLongDelay];
	} else {
		/* No check */
		DDLogVerbose(@"Check Status: session ended");
		[delegate logoutDone];
	}
}

#pragma mark -
#pragma mark NSURLConnection Delegate
- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
  [self cancelCheckServerStatus];
  receivedData = [[NSMutableData data] retain];
  [receivedData setLength:0];
  
  // Save cookie for futur request
  NSHTTPURLResponse * httpResponse = (NSHTTPURLResponse *)response;
	statusCode = [httpResponse statusCode];
	NSArray * cookies = [NSHTTPCookie cookiesWithResponseHeaderFields:[httpResponse allHeaderFields] forURL:[NSURL URLWithString:baseURL]];
	NSHTTPCookieStorage * cookiesStore = [NSHTTPCookieStorage sharedHTTPCookieStorage];
	for (NSHTTPCookie *cookie in cookies) {
    DDLogVerbose(@"Cookie: %@ = %@", cookie.name, cookie.value);
		[cookiesStore setCookie:cookie];
	}
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{    
  [receivedData appendData:data];
}

- (void)connectionDidFinishLoading:(NSURLConnection *)c
{    
  xml = [[TBXML alloc] initWithXMLData:receivedData]; // Data is copied by the initializer, safe to release
  NSString *str = [[NSString alloc] initWithData:receivedData encoding:NSUTF8StringEncoding];
  DDLogVerbose(@"xml: %@", str);
  DESTROY(str);
  
  switch (activeCommand) {
		case kParseStartSession:
			[self parseXMLStartSession];
			break;
		case kParseStatus:
			[self parseXMLStatus];
			break;
		case kParseLicensing:
			if (statusCode != 200) {
			  DDLogVerbose(@"The SM returned error");
			  [self startSessionWithSuffix:@".php"];
			} else {
			  [licensing parseXML:xml];
			}
			break;
  }
	
	DESTROY(xml);
  DESTROY(receivedData);
  DESTROY(connection);
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
  DDLogVerbose(@"---");
  [self cancelCheckServerStatus];
  [delegate error:_(@"Unable to reach the Session Manager")];
  DDLogError(@"Connection to the SM failed with error: %@", [error localizedDescription]);
}

- (BOOL)connection:(NSURLConnection *)connection 
canAuthenticateAgainstProtectionSpace:(NSURLProtectionSpace *)protectionSpace {
  return [protectionSpace.authenticationMethod isEqualToString:NSURLAuthenticationMethodServerTrust];
}

- (void)connection:(NSURLConnection *)connection 
didReceiveAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge {
  if ([challenge.protectionSpace.authenticationMethod isEqualToString:NSURLAuthenticationMethodServerTrust])
    [challenge.sender useCredential:[NSURLCredential credentialForTrust:challenge.protectionSpace.serverTrust] forAuthenticationChallenge:challenge];
  
  [challenge.sender continueWithoutCredentialForAuthenticationChallenge:challenge];
}

@end
