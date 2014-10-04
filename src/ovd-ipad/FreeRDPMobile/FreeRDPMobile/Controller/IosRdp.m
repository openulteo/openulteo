/**
 * Copyright (C) 2013-2014 Ulteo SAS
 * http://www.ulteo.com
 * Author David PHAM-VAN <d.pham-van@ulteo.com> 2013
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

#import "IosRdp.h"
#import "FreeRDPMobileAppDelegate.h"
#import <UIKit/UIPasteboard.h>
#import <MobileCoreServices/UTCoreTypes.h>

@implementation IosRdp

@synthesize delegate;
@synthesize backstore;


- (void)thread_func {
	AND_EXIT_CODE why = urdp_connect((urdp_context*)context);
	log_info("run_rdp : end %d", why);
  [self performSelectorOnMainThread:@selector(endConnectionWithReason:) withObject:[NSNumber numberWithInt:why] waitUntilDone:NO];
}


- (void)needsRedrawInRect:(NSValue*)rect {
   [[self delegate] session:self needsRedrawInRect:[rect CGRectValue]];
}


- (void)desktopResizedToSize:(NSValue*)size {
   [[self delegate] session:self desktopResizedToSize:[size CGSizeValue]];
}


- (void)endConnectionWithReason:(NSNumber*)why {
   [[self delegate] session:self endConnectionWithReason:[why intValue]];
   [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)processPrintJob:(NSValue*)filename {
	[[self delegate] session:self processPrintJob:[NSString stringWithUTF8String:[filename pointerValue]]];
}


AND_EXIT_CODE ios_begin_paint(urdp_context* context, uint8** data) {
	return SUCCESS;
}


AND_EXIT_CODE ios_end_paint(urdp_context* context, int x, int y, int w, int h) {
    ios_data_struct* ios_context = (ios_data_struct*)context;
	CGRect dirty_rect = CGRectMake(x, y, w, h);
    [ios_context->ios performSelectorOnMainThread:@selector(needsRedrawInRect:) withObject:[NSValue valueWithCGRect:dirty_rect] waitUntilDone:NO];    
	return SUCCESS;
}


AND_EXIT_CODE ios_desktop_resized(urdp_context* context, int width, int height, int bpp, void* buffer) {
	log_debug("desktop_resized w=%d h=%d bpp=%d", width, height, bpp);
    
    ios_data_struct* ios_context = (ios_data_struct*)context;
    //[ios_context->ios->backstore release];
    
    // Release the old context in a thread-safe manner
    if (ios_context->ios->backstore != NULL)
        CGContextRelease(ios_context->ios->backstore);
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    if (bpp == 16)
		ios_context->ios->backstore = CGBitmapContextCreate(buffer, width, height, 5, width * 2, colorSpace, kCGBitmapByteOrder16Little | kCGImageAlphaNoneSkipFirst);
	else
		ios_context->ios->backstore = CGBitmapContextCreate(buffer, width, height, 8, width * 4, colorSpace, kCGBitmapByteOrder32Little | kCGImageAlphaNoneSkipFirst);
    
    CGColorSpaceRelease(colorSpace);
    
    CGSize dirty_size = CGSizeMake(width, height);
    [ios_context->ios performSelectorOnMainThread:@selector(desktopResizedToSize:) withObject:[NSValue valueWithCGSize:dirty_size] waitUntilDone:YES];
    
	return SUCCESS;
}


AND_EXIT_CODE ios_pre_connect(urdp_context* context) {
	// load rdpsnd plugin
	RDP_PLUGIN_DATA *rdpsnd_data = NULL;
	
	rdpsnd_data = (RDP_PLUGIN_DATA *) xmalloc(2 * sizeof(RDP_PLUGIN_DATA));
	rdpsnd_data[0].size = sizeof(RDP_PLUGIN_DATA);
	rdpsnd_data[0].data[0] = "iosaudio";
	rdpsnd_data[0].data[1] = NULL;
	rdpsnd_data[0].data[2] = NULL;
	rdpsnd_data[0].data[3] = NULL;
	rdpsnd_data[1].size = 0;
	
	int ret = freerdp_channels_load_plugin(context->context.channels, context->context.instance->settings, "rdpsnd", rdpsnd_data);
	if (ret != 0) {
		log_error("failed loading rdpsnd plugin");
	} else {
		context->context.instance->settings->audio_playback = true;
	}
	
	return SUCCESS;
}


char* ios_get_client_name(urdp_context* context) {
	return strdup([[[UIDevice currentDevice] name] UTF8String]);
}

urdp_rdpdr_disk* ios_get_disks(urdp_context* context) {
	urdp_rdpdr_disk* disks;
	int stringCount = 1;
    int nb;
    
	disks = malloc(sizeof(urdp_rdpdr_disk) * (stringCount+1));
	
  for (nb=0; nb<stringCount; nb++) {
		disks[nb].name = "Documents";
		disks[nb].path = strdup([[NSHomeDirectory() stringByAppendingString:@"/Documents"] UTF8String]);
	}
    
	disks[nb].name = NULL;
	disks[nb].path = NULL;
    
	return disks;
}


void ios_process_clipboard(urdp_context* context, uint32 format, const uint8* data, uint32 size) {
	switch (format) {
        case CB_FORMAT_UNICODETEXT:
        case CB_FORMAT_TEXT:
            [[UIPasteboard generalPasteboard] setValue:[NSString stringWithUTF8String:(const char*)data] forPasteboardType:(NSString*)kUTTypeUTF8PlainText];
            break;
	}
}

void ios_process_printjob(urdp_context* context, const char* filename) {
	[((ios_data_struct*)context)->ios performSelectorOnMainThread:@selector(processPrintJob:) withObject:[NSValue valueWithPointer:filename] waitUntilDone:YES];
	unlink(filename);
}


AND_EXIT_CODE ios_context_free(urdp_context* context) {
	return SUCCESS;
}


AND_EXIT_CODE ios_context_new(urdp_context* context) {
	context->urdp_begin_draw = ios_begin_paint;
	context->urdp_draw = ios_end_paint;
	context->urdp_desktop_resized = ios_desktop_resized;
	context->urdp_pre_connect = ios_pre_connect;
	context->urdp_process_clipboard = ios_process_clipboard;
	context->urdp_get_client_name = ios_get_client_name;
  context->urdp_get_disks = ios_get_disks;
	context->urdp_context_free = ios_context_free;
	context->urdp_process_printjob = ios_process_printjob;
	
	context->gdi_flags = CLRCONV_RGB555;
	return SUCCESS;
}


- (id)init {
    delegate = nil;
    return self;
}


-(void)startSession {
    context = (ios_data_struct*)urdp_init(sizeof(ios_data_struct), ios_context_new);
    context->ios = self;
    backstore = nil;
    rdpSettings* settings = context->context.context.instance->settings;
    
    [[self delegate] session:self setupConnection:settings];
    settings->compression = false; /* bug with xrdp */
  
	log_debug("Start RDP : %dx%d %s %s %s:%d", settings->width, settings->height, settings->username, settings->password, settings->hostname, settings->port);
    [NSThread detachNewThreadSelector:@selector(thread_func) toTarget:self withObject:nil];
	log_info("main : end rdp init");
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(pasteboardNotificationReceived:) name:NEW_PASTBOARD_EVENT object:[UIPasteboard generalPasteboard]];
}

-(void)stopSession {
    context->context.rdp_run = false;
}

- (void)pasteboardNotificationReceived:(NSNotification*)notification {
    log_debug("pasteboard changed : %s", [[[UIPasteboard generalPasteboard] string] UTF8String]);
    urdp_cliprdr_send_unicode((urdp_context*)context, [[[UIPasteboard generalPasteboard] string] UTF8String]);
}


- (void)clickDownWithButton:(int)button andPoint:(CGPoint)point {
    if (context->context.rdp_run)
      urdp_click_down((urdp_context*)context, (int)point.x, (int)point.y, button);
}


- (void)clickUpWithButton:(int)button andPoint:(CGPoint)point {
    if (context->context.rdp_run)
      urdp_click_up((urdp_context*)context, (int)point.x, (int)point.y, button);
}


- (void)clickMoveWithButton:(int)button andPoint:(CGPoint)point {
    if (context->context.rdp_run)
      urdp_click_move((urdp_context*)context, (int)point.x, (int)point.y);
}


- (void)sendScancode:(int)scancode withFlag:(int)flag {
    if (context->context.rdp_run)
      urdp_send_scancode((urdp_context*)context, flag, scancode);
}


- (void)sendUnicode:(int)unicode {
    if (context->context.rdp_run)
      urdp_send_unicode((urdp_context*)context, unicode);
}

- (void)sendImePreeditString:(char*)data withSize:(long)size {
	if (context->context.rdp_run)
		urdp_ukbrdr_send_ime_preedit_string((urdp_context*)context, data, size);
}

- (void)sendImePreeditStringStop {
	if (context->context.rdp_run)
		urdp_ukbrdr_send_ime_preedit_string_stop((urdp_context*)context);
}
@end
