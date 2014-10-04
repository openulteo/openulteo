# Copyright (C) 2010 Ulteo SAS
# http://www.ulteo.com
# Author Samuel BOVEE <samuel@ulteo.com> 2010
# Author David LECHEVALIER <david@ulteo.com> 2010
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; version 2
# of the License
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

AM_CFLAGS = \
  -DXRDP_CFG_PATH=\"${sysconfdir}/xrdp\" \
  -Wall -O2 -D_REENTRANT \
  `pkg-config fuse --cflags` \
  `xml2-config --cflags`

INCLUDES = \
  -I$(top_srcdir)/common \
  -I$(top_srcdir)/vchannel/lib

AM_LDFLAGS = \
  `pkg-config fuse --libs` \
  `xml2-config --libs`

sbin_PROGRAMS = \
  vchannel_rdpdr

vchannel_rdpdr_SOURCES = \
  main.c  rdpdr.h\
  shares.c shares.h \
  printers.c printers.h \
  fuse_dev.c fuse_dev.h \
  hashmap.c hashmap.h \
  rdpfs_cache.c rdpfs_cache.h \
  rdpfs.h rdpfs.c 

vchannel_rdpdr_LDADD = \
  -L/usr/X11R6/lib \
  $(top_builddir)/vchannel/lib/libxrdp_vchannel.la \
  $(top_builddir)/common/libxrdp_common.la 

rdpdrsysconfdir=$(sysconfdir)/xrdp

rdpdrsysconf_DATA = \
  rdpdr.conf  

EXTRA_DIST = \
  $(rdpdrsysconf_DATA)
