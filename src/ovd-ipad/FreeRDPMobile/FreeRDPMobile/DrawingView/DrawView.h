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


//#import "../../FreeRDP/FreeRDP/config.h"
//#import <freerdp/freerdp.h>
//#import <freerdp/api.h>
//#import "chanman.h"
#import "IosRdp.h"
#import "Printer.h"

//@class Sound;
//@class Printer;

@interface DrawView : UIView  <IosRdpDelegate> {
  CGContextRef *draw;
  CGContextRef backstore;
  IosRdp *rdp;
  
  CGImageRef cursor;
  CGColorRef fg, bg;
  
  int cpt;
  
  // Objc helper class for rdpdr and rdpsnd channels
  //Sound   *sound;
  Printer *printer;
  
  BOOL restore;
//  BOOL connected;
  
  int width;
  int height;
	float scale; // 2.0 for retina
}

- (void)launchConnection;


- (void)session:(IosRdp *)session setupConnection:(rdpSettings*)settings;
- (void)session:(IosRdp *)session needsRedrawInRect:(CGRect)rect;
- (void)session:(IosRdp *)session endConnectionWithReason:(int)why;
- (void)session:(IosRdp*)session desktopResizedToSize:(CGSize)size;


@property (readonly) IosRdp *rdp;
@property CGImageRef cursor;
//@property BOOL connected;

@end
