# -*- mode: makefile; coding: utf-8 -*-
# Copyright © 2006 Martin Pitt <martin.pitt@ubuntu.com>
# Description: Rules for language pack support (POT file updating, and
# gettext domain key for .desktop/.directory/.server files)
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

ifndef _cdbs_rules_langpack
_cdbs_rules_langpack := 1

# try to build a POT file
common-post-build-arch:: langpack-mk-update-pot
common-post-build-indep:: langpack-mk-update-pot

langpack-mk-update-pot:
	if [ -d $(DEB_BUILDDIR)/po ]; then \
	    if grep -q intltool $(DEB_BUILDDIR)/po/Makefile*; then \
		if [ -x /usr/bin/intltool-update ]; then \
		    cd $(DEB_BUILDDIR)/po; /usr/bin/intltool-update -p --verbose || true; \
		elif [ -x $(DEB_BUILDDIR)/intltool-update ]; then \
		    cd $(DEB_BUILDDIR)/po; env XGETTEXT=/usr/bin/xgettext ../intltool-update -p --verbose || true; \
		else \
		    echo 'langpack.mk: po/Makefile* mentions intltool, but intltool-update is not available'; \
		    exit 1; \
		fi; \
	    elif [ -e $(DEB_BUILDDIR)/po/Makefile ]; then \
	        DOMAIN=$$(grep --max-count 1 '^GETTEXT_PACKAGE[[:space:]]*=' $(DEB_BUILDDIR)/po/Makefile | sed 's/^.*=[[:space:]]\([^[:space:]]\)/\1/'); \
	        if [ "$$DOMAIN" ]; then \
	            echo "langpack.mk: Generating $$DOMAIN.pot..."; \
	            make -C $(DEB_BUILDDIR)/po "$$DOMAIN.pot" || true; \
	        fi; \
	    fi; \
	fi

	if [ -d $(DEB_BUILDDIR)/help ]; then \
	    cd $(DEB_BUILDDIR)/help; make pot || true; \
	fi

# add translation domain to installed desktop/directory/schema files 
$(patsubst %,binary-predeb/%,$(DEB_PACKAGES)) :: binary-predeb/%:
	echo "langpack.mk: add translation domain to $(cdbs_curpkg)"; \
	if [ -e $(DEB_BUILDDIR)/po/Makefile ]; then \
	    DOMAIN=$$(grep --max-count 1 '^GETTEXT_PACKAGE[[:space:]]*=' $(DEB_BUILDDIR)/po/Makefile | sed 's/^.*=[[:space:]]*\([^[:space:]]\)/\1/'); \
	    if [ "$$DOMAIN" ]; then \
		for d in $$(find debian/$(cdbs_curpkg) -type f \( -name "*.desktop" -o -name "*.directory" \) ); do \
		    echo "langpack.mk: Replacing translations with domain $$DOMAIN in $$d..."; \
		    sed -ri '/^(Name|GenericName|Comment)\[/d' $$d; \
		    echo "X-Ubuntu-Gettext-Domain=$$DOMAIN" >> $$d; \
		done; \
                for d in $$(find debian/$(cdbs_curpkg) -type f -name "*.server" ); do \
                    echo "langpack.mk: Adding translation domain $$DOMAIN to $$d..."; \
                    sed -i "s/<oaf_server\>/<oaf_server ubuntu-gettext-domain=\"$$DOMAIN\"/" $$d; \
                done; \
                for d in $$(find debian/$(cdbs_curpkg) -type f -name "*.schemas" ); do \
                    echo "langpack.mk: Replacing translations with domain $$DOMAIN in $$d..."; \
                    sed -ri "s/^([[:space:]]*)(<locale name=\"C\">)/\1<gettext_domain>$$DOMAIN<\/gettext_domain>\n\1\2/; /^[[:space:]]*<locale name=\"[^C]/,/^[[:space:]]*<\/locale>[[:space:]]*\$$/ d; /^$$/d; s/<\/schema>$$/&\n/" $$d; \
                done; \
	    fi; \
	fi
endif
