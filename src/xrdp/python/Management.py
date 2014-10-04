# -*- coding: utf-8 -*-

# Copyright (C) 2009 Ulteo SAS
# http://www.ulteo.com
# Author Julien LANGLOIS <julien@ulteo.com> 2009
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
import socket
import struct
from xml.dom import minidom

from Exception import XrdpException

"""
Protocol:
	datagrame [ size_t , data ]
	  size_t: unsigned int 32 bits big indian
	  data: utf-8 (xml)
	
	request:
	  <request type="" action="" [id=""] />
	   type: sessions -> action: list
	   type: session  -> id=$ID action: status
	                            action: logoff
	
	response:
	 <error>Message</errror>
	 
	  <response>
	    <session id="" status="" username=""/>
	  </response>
	  
	  <response>
	    <sessions>
	      <session id="" status="" username=""/>
	      <session id="" status="" username=""/>
	    </sessions>
	  </response>
"""

def _ManagementGetSocket():
	filename = "/var/spool/xrdp/xrdp_management"
	
	if not os.path.exists(filename):
		raise XrdpException("Xrdp not available")
	
	# ToDo: check is rw access on the file
	
	return filename


def _ManagementProcessRequest(request):
	path = _ManagementGetSocket()
	if path is None:
		raise XrdpException("Xrdp not available")
	
	s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
	try:
		s.connect(path)
	except socket.error,err:
		#print "Unable to connect: ",err
		raise XrdpException("Xrdp not available (Unable to connect)")
	
	data = request.toxml()
		
	s.send(struct.pack('>I', len(data)))
	s.send(data)
	
	buffer = s.recv(4)
	try:
		packet_len = struct.unpack('>I', buffer)[0]
	except Exception, e:
		#print "packet recv syntax error"
		s.close()
		raise XrdpException("Xrdp not available (packet recv syntax error)")
	
	data = s.recv(packet_len)
	while len(data) < packet_len:
		data += s.recv(packet_len-len(data))
	
	s.close()
	
	try:
		document = minidom.parseString(data)
	except:
		#print "server didn't return XML"
		raise XrdpException("Xrdp not available (packet recv syntax error xml)")
	
	if document.documentElement.nodeName == "error":
		#print "server return error"
		raise XrdpException("Xrdp error")
	

	return document
