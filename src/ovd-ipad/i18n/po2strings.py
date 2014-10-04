#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Author: Harold Leboulanger <harold@leboulanger.org>
# Date:   26/05/11 16:00:30
# Company: Ulteo
import argparse
import re
import codecs
import string

import translate.storage.po

OUTPUT_ENC = "UTF-16"

def convert_file(src, dst):
    po = translate.storage.po.pofile.parsefile(src)

    with codecs.open(dst, "w", encoding=OUTPUT_ENC) as d:
        for unit in po.getunits():
            id = unit.getid().replace('"', '\\"')
            #print id
            target = unit.gettarget().replace('"', '\\"')

            if not target:
                target = id
            string = u'"{0}" = "{1}";\n\n'.format(id, target)
            #print string
            d.write(string)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Convert po file to Xcode strings")
    parser.add_argument("-f", "--file",  help="file to convert", required=True)
    parser.add_argument("-o", "--output", default="Localized.strings", help="destination filename")

    args = parser.parse_args()
    convert_file(args.file, args.output)

