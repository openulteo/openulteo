# -*- mode: makefile; coding: utf-8 -*-
# Description: A class for XFCE packages
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA.


_cdbs_scripts_path ?= /usr/lib/cdbs
_cdbs_rules_path ?= /usr/share/cdbs/1/rules
_cdbs_class_path ?= /usr/share/cdbs/1/class

ifndef _cdbs_class_xfce_
_cdbs_class_xfce := 1

# for dh_icons
CDBS_BUILD_DEPENDS   := $(CDBS_BUILD_DEPENDS), debhelper (>= 5.0.7ubuntu4)

include $(_cdbs_rules_path)/debhelper.mk$(_cdbs_makefile_suffix)
include $(_cdbs_rules_path)/simple-patchsys.mk$(_cdbs_makefile_suffix)
include $(_cdbs_rules_path)/langpack.mk$(_cdbs_makefile_suffix)
include $(_cdbs_class_path)/autotools.mk$(_cdbs_makefile_suffix)

$(patsubst %,binary-install/%,$(DEB_PACKAGES)) :: binary-install/%:
	if test -x /usr/bin/dh_desktop; then dh_desktop -p$(cdbs_curpkg) $(DEB_DH_DESKTOP_ARGS); fi
	if test -x /usr/bin/dh_icons; then dh_icons -p$(cdbs_curpkg) $(DEB_DH_ICONCACHE_ARGS); fi

endif
