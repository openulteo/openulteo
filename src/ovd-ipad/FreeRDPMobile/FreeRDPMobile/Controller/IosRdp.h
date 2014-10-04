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

#import <Foundation/Foundation.h>

#include "../../FreeRDP/FreeRDP/config.h"
#include <freerdp/freerdp.h>
#include <freerdp/channels/channels.h>
#include <freerdp/gdi/gdi.h>
#include <freerdp/gdi/dc.h>
#include <freerdp/gdi/region.h>
#include <freerdp/rail/rail.h>
#include <freerdp/cache/cache.h>
#include <freerdp/utils/memory.h>
#include <freerdp/utils/semaphore.h>
#include <freerdp/utils/event.h>
#include <freerdp/constants.h>

#include "log.h"
#include "urdp.h"
#include "urdp_event.h"

typedef struct ios_data_struct ios_data_struct;
@class IosRdp;


@protocol IosRdpDelegate <NSObject>
- (void)session:(IosRdp*)session needsRedrawInRect:(CGRect)rect;
- (void)session:(IosRdp*)session desktopResizedToSize:(CGSize)size;
- (void)session:(IosRdp *)session endConnectionWithReason:(int)why;
- (void)session:(IosRdp *)session setupConnection:(rdpSettings*)settings;
- (void)session:(IosRdp *)session processPrintJob:(NSString*)filename;
@end


@interface IosRdp : NSObject {
    CGContextRef backstore;
    ios_data_struct* context;
    NSObject<IosRdpDelegate>* delegate;
}

@property (assign) id <IosRdpDelegate> delegate;
@property (readonly) CGContextRef backstore;

- (id) init;
- (void) startSession;
- (void) stopSession;
- (void)clickDownWithButton:(int)button andPoint:(CGPoint)point;
- (void)clickUpWithButton:(int)button andPoint:(CGPoint)point;
- (void)clickMoveWithButton:(int)button andPoint:(CGPoint)point;
- (void)sendScancode:(int)scancode withFlag:(int)flag;
- (void)sendUnicode:(int)unicode;
- (void)sendImePreeditString:(char*)data withSize:(long)size;
- (void)sendImePreeditStringStop;

@end


typedef struct ios_data_struct {
	urdp_context context;
    IosRdp* ios;
} t_ios_data_struct;
