#! /usr/bin/env python

# Copyright (C) 2013 Ulteo SAS
# http://www.ulteo.com
# Author David PHAM-VAN <d.pham-van@ulteo.com> 2013
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

import os
import sys
import json
from glob import glob


def build_links():
	links = {}
	for link in glob("*/*"):
		if os.path.islink(link):
			links[link] = os.readlink(link)
	json.dump(links, open("links.json", "w"), sort_keys=True, indent=0, separators=(',', ': '))


def make_symlinks():
	links = json.load(open("links.json", "r"))
	if len(links) > 0:
		for src, dest in links.iteritems():
			os.symlink(dest, src)


def clean_symlinks():
	links = json.load(open("links.json", "r"))
	if len(links) > 0:
		for src, dest in links.iteritems():
			if os.path.islink(src):
				os.unlink(src)


if __name__ == "__main__":
	if len(sys.argv) > 0 and sys.argv[1] == "build":
		build_links()
	elif len(sys.argv) > 0 and sys.argv[1] == "clean":
		clean_symlinks()
	else:
		make_symlinks()
