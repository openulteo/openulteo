#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright (c) 2011
# Author Harold LEBOULANGER <harold@ulteo.com> 2011

import os
import sys

def build_file_list(dir, ext="m"):
    """
    Return a files list build recursively from a directory and an extension.

    """
    list = []

    for root, dirs, files in os.walk(dir):
        for file in files:
            if file.endswith(ext):
                f = os.path.join(root, file)
                list.append(f)

    return list

if __name__ == "__main__":
    print "Not a script"
    sys.exit(1)
