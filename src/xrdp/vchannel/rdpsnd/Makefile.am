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
  `pkg-config --cflags libpulse-simple` \
  -DXRDP_CFG_PATH=\"${sysconfdir}/xrdp\"

INCLUDES = \
  -I$(top_srcdir)/common \
  -I$(top_srcdir)/vchannel/lib

sbin_PROGRAMS = \
  vchannel_rdpsnd

vchannel_rdpsnd_SOURCES = \
  config.h main.c \
  sound_channel.h sound_channel.c

vchannel_rdpsnd_LDADD = \
  $(top_builddir)/vchannel/lib/libxrdp_vchannel.la \
  $(top_builddir)/common/libxrdp_common.la

vchannel_rdpsnd_LDFLAGS = \
  `pkg-config --libs libpulse-simple`

alsaconfdir=$(sysconfdir)
alsaconf_DATA = \
  asound.conf

sndsysconfdir=$(sysconfdir)/xrdp
sndsysconf_DATA = \
  rdpsnd.conf rdpsnd.pa

EXTRA_DIST = \
  $(sndsysconf_DATA) \
  $(alsaconf_DATA)
