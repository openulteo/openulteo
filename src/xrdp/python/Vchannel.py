# -*- coding: utf-8 -*-

# Copyright (C) 2010-2013 Ulteo SAS
# http://www.ulteo.com
# Author Laurent CLOUET <laurent@ulteo.com> 2010
# Author Julien LANGLOIS <julien@ulteo.com> 2010
# Author David LECHEVALIER <david@ulteo.com> 2010, 2013
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

import ctypes
from VchannelException import VchannelException

class VirtualChannel():
	def __init__(self, name):
		self.CHUNK_LENGTH = 1600
		self.STATUS_ERROR = -1
		self.STATUS_NORMAL = 0
		self.STATUS_DISCONNECTED = 1
		self.STATUS_CONNECTED = 2
		self._dll = ctypes.cdll.LoadLibrary("libxrdp_vchannel.so.0")
		self._dll.vchannel_init()
		self._handle = None
		self.name = name
		self.connected = False

	def isConnected(self) :
		return self.connected

	def Open(self) :
	        self._handle = self._dll.vchannel_open(self.name)
		if self._handle == self.STATUS_ERROR :
			self._handle = None
			self.connected = False
			return False
		self.connected = True
		return True

	def Close(self):
		if self._handle == None :
			return False

		res = self._dll.vchannel_close(self._handle)
		return res == self.STATUS_NORMAL

	def Read(self):
		if self._handle == None :
			return None

		buffer = ctypes.create_string_buffer(1600)
		buffer_len = ctypes.c_ulong(0)
		total_len = ctypes.c_ulong(0)

		ret = self._dll.vchannel_receive(self._handle, ctypes.byref(buffer), ctypes.byref(buffer_len), ctypes.byref(total_len))

		if ret == self.STATUS_NORMAL:
			return buffer.raw
		if ret == self.STATUS_ERROR:
			return None
		if ret == self.STATUS_DISCONNECTED:
			self.connected = False
			return None
		if ret == self.STATUS_CONNECTED:
			self.connected = True
			return None
		if buffer_len == 0:
			self.connected = False
			return None
                return None

	def Write(self, message):
		if self._handle == None :
			return False

		buffer = ctypes.create_string_buffer(len(message))
		buffer.raw = message
		buffer_len = ctypes.c_ulong(len(message))

		ret = self._dll.vchannel_send(self._handle, ctypes.byref(buffer), buffer_len)
		
		return ret == self.STATUS_NORMAL

