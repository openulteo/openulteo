/**
 * Copyright (C) 2011-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author Harold LEBOULANGER <harold@ulteo.com> 2011
 * Author Clement BIZEAU <cbizeau@ulteo.com> 2011
 * Author Guillaume SEMPE <guillaume.sempe@gmail.com> 2012
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


#import "OVDClientViewController.h"
#import "DrawView.h"
#import "RdpConfig.h"
#import "locales.h"

// Virtual channel
//#import "Printer.h"

//void *refToSelf; // Let the C access the DrawView object

#define DEBUG_GRAPHICS  0
#define MAX_PLUGIN_DATA 20

#pragma mark - Private interface
@interface DrawView()
- (BOOL)_supportsTaskCompletion;
@end

@implementation DrawView

# pragma mark Class management
- (void)dealloc {
    if (backstore != NULL) CGContextRelease(backstore);
    [super dealloc];
}

- (id)initWithFrame:(CGRect)frame {
    DDLogVerbose(@"+++");
    rdp = nil;
    
    self = [super initWithFrame:frame];
    if (self) {
        //refToSelf = self;
        
        width = frame.size.width;
        height = frame.size.height;
			  scale = [[UIScreen mainScreen] scale];
				self.contentScaleFactor = 1.0;
				self.layer.contentsScale = 1.0;
			
        //sound   = [[Sound alloc] init];
        printer = [[Printer alloc] initWithSize:CGSizeMake(width, height)];
        
        cpt = 0;
        
    }
    return self;
}

/*
- (NSString *)documentsDirectory {
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES); 
	NSString *documentsDirectoryPath = [paths objectAtIndex:0];
	return documentsDirectoryPath;
}*/

#pragma mark IosRdpDelegate


char* allocAndCopy(NSString *src) {
    char* dst = malloc([src length]+1);
    strcpy(dst, [src UTF8String]);
    return dst;
}


- (void)session:(IosRdp *)session setupConnection:(rdpSettings*)settings {
    RdpConfig *config = [RdpConfig sharedConfig];
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    
    settings->width = width;
	  settings->height = height;
	  settings->color_depth = 16;
    
    settings->username = allocAndCopy([config apsUser]);
    settings->password = allocAndCopy([config apsPassword]);
    settings->hostname = allocAndCopy([config apsHost]);
    
    if([config gatewayOn]) {
        settings->port = 443;
        settings->gateway = true;
        strncpy(settings->gateway_token, [[config token] UTF8String], sizeof(settings->gateway_token));
        log_debug("Using gateway %s", settings->gateway_token);
        NSArray *sma = [[config sm] componentsSeparatedByString: @":"];
        if (sma.count == 2) {
            NSString *host = [sma objectAtIndex: 0];
            NSString *port = [sma objectAtIndex: 1];
            settings->hostname = allocAndCopy(host);
            settings->port = [port intValue];
        }
    } else {
        settings->port = 3389;
    }

    if (![config checkRDPConnection])
  	    settings->shell = allocAndCopy(@"OvdDesktop");

	settings->jpeg = true;
	settings->connection_type = [defaults integerForKey: @"connection_type"];
	settings->compression = [defaults boolForKey: @"bulk_compression"];
	settings->compression = false;
	if (settings->compression)
		log_debug("Bulk compression enabled");
	settings->tls_security = false;
    
    /*
    NSInteger keyboardLayoutID = 0x00000409; // default en_US
    {   // Find the best keyboard layout to use
        NSArray *activeInputModesArray = [UITextInputMode activeInputModes];
        if (0 < [activeInputModesArray count]) {
            UITextInputMode *firstActiveInputMode = [activeInputModesArray objectAtIndex:0];
            NSString *keyboardLayoutName = firstActiveInputMode.primaryLanguage;
            NSInteger currentKeyboardID = 0;
            NSString *currentKeyboardName = nil;
            NSInteger currentIndex = 0;
            do {
                currentKeyboardID = (_win_locales[currentIndex]).keyboardID;
                currentKeyboardName = (_win_locales[currentIndex]).keyboardName;
                if ([currentKeyboardName isEqualToString:keyboardLayoutName]) {
                    keyboardLayoutID = currentKeyboardID;
                }
                currentIndex++;
            } while (currentKeyboardID!=0);
        }
    }*/
    //settings->keyboard_layout = keyboardLayoutID;
}


- (void)session:(IosRdp *)session endConnectionWithReason:(int)why {
    // The super view is the ScrollView
    // The super super view is a View Controller, needed to dismiss the modal view
		DDLogInfo(@"Rdp Connection closed (%d)", why);
    [[UIApplication sharedApplication] setStatusBarHidden:NO withAnimation:YES];
    OVDClientViewController *viewcontroller = (OVDClientViewController *)[[[self superview] superview] nextResponder];
    [viewcontroller dismissModalViewControllerAnimated:YES];
}


- (void)session:(IosRdp *)session needsRedrawInRect:(CGRect)rect {
    [self setNeedsDisplay];
}


- (void)session:(IosRdp*)session desktopResizedToSize:(CGSize)size {
    CGRect r = [self bounds];
    r.size.width = size.width / scale;
    r.size.height = size.height / scale;
    [self setBounds:r];
    [self setFrame:r];
    [(UIScrollView*)[self superview] setZoomScale:1.0];
    [(UIScrollView*)[self superview] setContentSize:size];
  	float minimumZoom = [[UIScreen mainScreen] bounds].size.height / r.size.width;
	  ((UIScrollView*)[self superview]).minimumZoomScale = minimumZoom;
  	((UIScrollView*)[self superview]).maximumZoomScale = minimumZoom * 3.0;
  	[(UIScrollView*)[self superview] setZoomScale:minimumZoom];
}

- (void)session:(IosRdp *)session processPrintJob:(NSString*)filename {
	NSData *data = [NSData dataWithContentsOfFile:filename];
  [printer printWithData:data andView:self];
}


# pragma mark Connection management
- (void)launchConnection {
    rdp = [[IosRdp alloc] init];
    [rdp setDelegate:self];
    [rdp startSession];
}

/*
- (void)setupConnection {    
    
    
    IosRdp *rdp = [[IosRdp alloc] init];
    
 
    [[RdpConfig sharedConfig] reset];
}

- (void)launchConnection
{

    __block UIBackgroundTaskIdentifier backgroundTaskID = UIBackgroundTaskInvalid;

    void (^completionBlock)() = ^{
        [[UIApplication sharedApplication] endBackgroundTask:backgroundTaskID];
        backgroundTaskID = UIBackgroundTaskInvalid;
    };

    if ([self _supportsTaskCompletion]) {
        // according to docs this is safe to be called from background threads
        backgroundTaskID = [[UIApplication sharedApplication] beginBackgroundTaskWithExpirationHandler:completionBlock];
    }

    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    connected = YES;
    
    void   *read_fds[32];
    void   *write_fds[32];
    int     read_count;
    int     write_count;
    int     index;
    int     sck;
    int     max_sck;
    fd_set  rfds;
    fd_set  wfds;
    
    // Custom select
    int retSelect;
    struct timeval wait; // select timeout
    wait.tv_sec = 0;
    wait.tv_usec = 200000;
    
    signal(SIGPIPE, SIG_IGN);
    
    while (connected)
	{
		read_count  = 0;
		write_count = 0;
	}
     DESTROY(pool);
    
    if (backgroundTaskID != UIBackgroundTaskInvalid) {
        completionBlock();
    }

    [self performSelectorOnMainThread:@selector(endConnection) withObject:nil
                        waitUntilDone:NO];
}
*/


# pragma mark View Drawing
/*
CGContextRef CreateBitmapContext (int pixelsWide, int pixelsHigh) {
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	
	// create the bitmap context
	CGContextRef bitmapContext = CGBitmapContextCreate (NULL, pixelsWide, pixelsHigh, 8, 0, colorSpace, (kCGBitmapByteOrder32Little |kCGImageAlphaPremultipliedFirst));
	CGColorSpaceRelease(colorSpace);
	
    return bitmapContext;
}*/

/**
 * Main drawing method
 * Draw the backbuffer on the view when needed
 */
- (void)drawRect:(CGRect)rect {
    //	DLog(@"Draw the backbuffer in the view");
	if (rdp != nil) {
        CGContextRef context = UIGraphicsGetCurrentContext();
		    CGImageRef cgImage = CGBitmapContextCreateImage([rdp backstore]);
        
        CGContextTranslateCTM(context, 0, [self bounds].size.height);
        CGContextScaleCTM(context, 1.0, -1.0);
        //CGContextClipToRect(context, CGRectMake(rect.origin.x, [self bounds].size.height - rect.origin.y - rect.size.height, rect.size.width, rect.size.height));
        CGContextDrawImage(context, CGRectMake(0, 0, [self bounds].size.width, [self bounds].size.height), cgImage);
        //CGContextDrawImage(context, rect, cgImage);
        CGImageRelease(cgImage);
        
        //backstore = CreateBitmapContext(width, height);
        //draw = &backstore;
        //		DLog(@"Backstore created: %@", backstore);
	}
	//DLog(@"g_server_depth: %d", g_server_depth);
    /*
	CGContextRef context = UIGraphicsGetCurrentContext();
    CGImageRef cgImage = CGBitmapContextCreateImage(*draw);
	CGContextDrawImage(context, rect, cgImage);
    CGImageRelease(cgImage);
     */
}

- (void)drawRect:(CGRect)rect rdColor:(int)col; {
    //CGColorRef color = Color16toCGColor(col);
    
	//CGContextSetFillColorWithColor(*draw, color);
	//CGContextFillRect(*draw, rect);
    
#if DEBUG_GRAPHICS
    [self drawDebugRect:rect color:[[UIColor blueColor] CGColor]];
#endif
}


#pragma mark Needs display
/*
- (void)performSetNeedsDrawOnMainThread; {
	[self performSelectorOnMainThread:@selector(setNeedsDisplay) withObject:self waitUntilDone:NO];
}
*/
/*
- (CGContextRef *)draw {
    DDLogVerbose(@"---");
    return draw;
}
*/
/*
- (void)setDraw:(CGContextRef *)d {
    DDLogVerbose(@"---");
    draw = d;
}
*/
#pragma mark - Background support methods
- (BOOL)_supportsTaskCompletion {

	UIDevice *device = [UIDevice currentDevice];

	if ([device respondsToSelector:@selector(isMultitaskingSupported)]) {
		if (device.multitaskingSupported) {
			return YES;
		}
		else {
			return NO;
		}
	}
	return NO;
}

/*
#pragma mark -
#pragma mark Debug Methods
- (void)saveImage:(CGImageRef)image prefix:(NSString *)prefix count:(int)c; {
    UIImage *img  = [UIImage imageWithCGImage:image];
    NSData  *imgData = UIImagePNGRepresentation(img);
    [imgData writeToFile:[NSString stringWithFormat:@"%@/%@-%d.png",[self documentsDirectory], prefix, c] atomically:YES];
}

- (void)drawDebugRect:(CGRect) rect color:(CGColorRef)c;
{
    CGContextSetStrokeColorWithColor(*draw, c);
    CGContextSetLineWidth(*draw, 1);
    CGContextMoveToPoint(*draw, rect.origin.x, rect.origin.y);
    CGContextAddLineToPoint(*draw, rect.origin.x + rect.size.width, rect.origin.y);
    CGContextAddLineToPoint(*draw, rect.origin.x + rect.size.width, rect.origin.y + rect.size.height);
    CGContextAddLineToPoint(*draw, rect.origin.x, rect.origin.y + rect.size.height);
    CGContextAddLineToPoint(*draw, rect.origin.x, rect.origin.y);
    
    CGContextStrokePath(*draw);
}
*/
@synthesize rdp;
@synthesize cursor;
//@synthesize connected;

@end
