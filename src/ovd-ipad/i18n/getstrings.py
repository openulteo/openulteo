#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2011
# Author Harold LEBOULANGER <harold@ulteo.com> 2011

"""
Parse objc files to find strings used in the application.  Compare with a pot
file to find duplicate string and write untranslated strings in a destination
file.

"""

import sys
import argparse
import re
import translate.storage.po

from ftree import build_file_list

def extract_string(file, regex):
    """
    Extract string from objc file

    """
    result = []

    with open(file, "r") as f:
        lines = f.readlines()
        for line in lines:
            match = regex.search(line)
            if match:
                result.append(match.group(1))

    return result

def remove_duplicate(l):
    """
    Remove duplicate from a list
    
    """
    l = list(set(l))
    return l

def remove_already_in_pot(strings, pot):
    """
    Remove strings already in pot file

    """
    ret = []

    po = translate.storage.po.pofile.parsefile(pot)
    ids = [unit.getid() for unit in po.getunits()]

    for string in strings:
        if string not in ids:
            ret.append(string)

    return ret 
       

def write_pot(file, strings):
    """
    Write a pot file with strings

    """
    with open(file, "w") as f:
        for string in strings:
            f.write('msgid "{0}"\nmsgstr ""\n\n'.format(string))
        
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Extract strings from objective-c files")
    parser.add_argument("-d", "--directory", default=".", help="directory to scan")
    parser.add_argument("-o", "--output", default="untranslated.txt", help="destination filename")
    parser.add_argument("-p", "--pot", help="existing pot file")

    args = parser.parse_args()

    # get filenames
    files = build_file_list(args.directory)

    # get strings from source code
    regex = re.compile("_\(@\"(.+)\"\)")
    strings = []
    for file in files:
        result = extract_string(file, regex)
        if result:
            strings.extend(result)
    
    # remove duplicates string
    strings = remove_duplicate(strings)

    # remove already translated strings
    strings = remove_already_in_pot(strings, args.pot)

    # write output
    if len(strings) > 0:
        write_pot(args.output, strings)

