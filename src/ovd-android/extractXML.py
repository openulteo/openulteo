#!/usr/bin/python
# -*- coding: utf-8 -*-

# Copyright (C) 2011-2014 Ulteo SAS
# http://www.ulteo.com
# Author Pierre Laine <plaine@ulteo.com> 2011
# Author Clément Bizeau <cbizeau@ulteo.com> 2011
# Author David PHAM-VAN <d.pham-van@ulteo.com> 2012, 2013, 2014
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

from xml.etree import ElementTree
import translate.storage.po
from xml.dom import minidom
import codecs
import re
import os
import glob
from optparse import OptionParser


def build_android_dir(dir, filename):
	"""
	Return a directory with format: "value-<language>"
	"""
	regex = re.compile('.*/([a-zA-Z\_]{2,5})\.po')
	locale = regex.match(filename)
	locale = locale.group(1)
	locale = locale.replace("_", "-r");
	dest = os.path.join(dir,"values-"+locale)
	if not os.path.exists(dest):
		os.makedirs(dest)

	return dest


def extract_strings_xml_en(filename):
	"""
	Extract name value and string from strings.xml
	ex: {'hello': "Hello, world!", ...}
	"""
	data = dict()
	with open(filename, "r") as f:
		tree = ElementTree.parse(f)
	
	for node in tree.findall(".//string"):
		data[node.attrib.get("name")] = node.text
	
	for array in tree.findall(".//string-array"):
		name = array.attrib.get("name")
		i = 0
		for node in array:
			data["%s[%d]" % (name, i)] = node.text
			i += 1
	
	return data

def extract_po(data_xml_android, filename_po):
	"""
	Extract po data. Return a dictionnary : key:msgid, value:msgstr
	"""
	po = translate.storage.po.pofile.parsefile(filename_po)
	
	data = dict()
	for unit in po.getunits():
		po_id = unit.getid()
		po_id = po_id.replace("\"", "\\\"")
		po_translation = unit.gettarget()
		for k, v in data_xml_android.iteritems():
			if v == po_id:
				data[k] = po_translation

	return data

def create_xml(data, filename_out):
	"""
	Save the Android XML language file.
	"""
	regex = re.compile('(.*)\[([\d]*)\]')
	root = ElementTree.Element("resources")
	arrays = {}

	for k,v in data.iteritems():
		m = regex.match(k)
		if m:
			k = m.group(1)
			i = m.group(2)
			if not arrays.has_key(k):
				arrays[k]={}
			
			arrays[k][i] = v
			continue
		string_node = ElementTree.SubElement(root, "string")
		string_node.set("name", k)
		v = v.replace("\'", "\\\'")
		string_node.text = v
	
	for array in arrays:
		string_node = ElementTree.SubElement(root, "string-array")
		string_node.set("name", array)
		for item in sorted(arrays[array].keys()):
			item_node = ElementTree.SubElement(string_node, "item")
			v = arrays[array][item].replace("\'", "\\\'")
			item_node.text = v

	xmlstring = ElementTree.tostring(root)
	
	parse_xmlstring = minidom.parseString(xmlstring)
	parse_xmlstring = parse_xmlstring.toprettyxml(indent="  ")

	with codecs.open(filename_out, "w", encoding="UTF-8") as f:
		f.write(parse_xmlstring)

def make_po_source(data_xml_android, filename_output):
	po = translate.storage.po.pofile()
	for k, v in data_xml_android.iteritems():
		po.addunit(translate.storage.po.pounit(source=v))
	if filename_output == "-":
		print str(po)
		return
	
	f = file(filename_output, "w")
	f.write(str(po))
	f.close()

if __name__ == "__main__":
	parser = OptionParser()
	parser.add_option("-x", "--xml", dest="xml", default="res/values/strings.xml", help="Set the Android xml source file")
	parser.add_option("-p", "--po-dir", dest="podir", default="uovdclient", help="Set the ovd translations folder")
	parser.add_option("-s", "--save", dest="save", help="Set the file where to save the .po output from Android strings")
	parser.add_option("-q", "--quiet", dest="quiet", action="store_true", default=False, help="Disable outputs")
	(options, args) = parser.parse_args()

	data_strings_xml = extract_strings_xml_en(options.xml)
	if options.save:
		make_po_source(data_strings_xml, options.save)
	
	#get res directory based on english xml path
	list_po_files = glob.glob(os.path.join(options.podir, "*.po"))
	xml_dir = os.path.dirname(os.path.dirname(options.xml))

	for po_file in list_po_files:
		if not options.quiet:
			print po_file

		translations = extract_po(data_strings_xml, po_file)
		dest_dir = build_android_dir(xml_dir, po_file)
		if not options.quiet:
			print dest_dir
		
		create_xml(translations, os.path.join(dest_dir, "strings.xml"))
