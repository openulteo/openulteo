#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2011
# Author Harold LEBOULANGER <harold@ulteo.com> 2011

"""
Convert po files to Localizable.strings needed for Xcode internationalization
"""

import argparse
import os

import po2strings
import ftree

def convert(files, dst):
    for file in files:
        basename = os.path.basename(file)
        root, ext = os.path.splitext(basename)

        dst_dir = os.path.join(dst, root + ".lproj")
        if not os.path.exists(dst_dir):
            os.makedirs(dst_dir)
            print "Creating directory {0}".format(dst_dir)

        print "Converting {0} to {1}".format(file, os.path.join(dst_dir, "Localizable.strings"))
        po2strings.convert_file(file, os.path.join(dst_dir, "Localizable.strings"))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Convert all po files to Xcode localized version")
    parser.add_argument("-s", "--source", help="directory containing po files",
            required=True)
    parser.add_argument("-o", "--output",  help="Xcode destination directory",
            required=True)

    args = parser.parse_args()

    files = ftree.build_file_list(args.source, ".po")
    convert(files, args.output)
    files = ftree.build_file_list(args.source, ".pot")
    convert(files, args.output)


