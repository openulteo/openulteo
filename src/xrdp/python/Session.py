# -*- coding: utf-8 -*-

# Copyright (C) 2009-2012 Ulteo SAS
# http://www.ulteo.com
# Author Julien LANGLOIS <julien@ulteo.com> 2009
# Author David LECHEVALIER <david@ulteo.com> 2012
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

from xml.dom import minidom

from Management import _ManagementProcessRequest

SESSION_STATUS_DISCONNECTED	= "DISCONNECT"
SESSION_STATUS_ACTIVE		= "ACTIVE"
SESSION_STATUS_UNKNOWN		= "UNKNOWN"
SESSION_STATUS_CLOSED		= "CLOSED"


def _SessionFormatRequest(type_, action_, id_ = None, username_ = None):
	doc = minidom.Document()
	rootNode = doc.createElement("request")
	rootNode.setAttribute("type", type_)
	rootNode.setAttribute("action", action_)
	if id_ is not None:
		rootNode.setAttribute("id", id_)
	if username_ is not None:
		rootNode.setAttribute("username", username_)
		
	doc.appendChild(rootNode)
	return doc




def SessionGetList():
	l = {}
	
	doc = _ManagementProcessRequest(_SessionFormatRequest("sessions", "list"))
	if doc is None:
		return l
	
	sessionNodes = doc.getElementsByTagName("session")
	for node in sessionNodes:
		session = {}
		
		for key in ["id", "username", "status"]:
			session[key] = node.getAttribute(key)
		
		l[session["id"]] = session
	
	return l


def SessionGetStatus(session_id):
	doc = _ManagementProcessRequest(_SessionFormatRequest("session", "status", session_id))
	
	if doc is None:
		return SESSION_STATUS_UNKNOWN
	
	sessions = doc.getElementsByTagName("session")
	if len(sessions) == 0:
		return SESSION_STATUS_UNKNOWN
	
	sessionNode = sessions[0]
	status = sessionNode.getAttribute("status")
	return status


def SessionGetId(username):
	doc = None
	try:
		doc = _ManagementProcessRequest(_SessionFormatRequest("session", "status", None, username))
	except Exception, e:
		return None
	
	if doc is None:
		return None
	
	sessions = doc.getElementsByTagName("session")
	if len(sessions) == 0:
		return None
	
	sessionNode = sessions[0]
	session_id = sessionNode.getAttribute("id")
	return session_id


def SessionLogoff(session_id):
	doc = _ManagementProcessRequest(_SessionFormatRequest("session", "logoff", session_id))
	if doc is None:
		return False
	
	sessions = doc.getElementsByTagName("session")
	if len(sessions) == 0:
		return False
	
	sessionNode = sessions[0]
	status = sessionNode.getAttribute("status")
	if status != SESSION_STATUS_CLOSED:
		return False
	
	return True
	
	
def SessionDisconnect(session_id):
	doc = _ManagementProcessRequest(_SessionFormatRequest("session", "disconnect", session_id))
	if doc is None:
		return False
	
	sessions = doc.getElementsByTagName("session")
	if len(sessions) == 0:
		return False
	
	sessionNode = sessions[0]
	status = sessionNode.getAttribute("status")
	if status != SESSION_STATUS_DISCONNECTED:
		return False
	
	return True

def main():
	list = SessionGetList()
	for session_id in list:
		try:
			status = SessionGetStatus(session_id)
			print session_id," -> ", status
		except:
			print session_id," -> Unknow"

if __name__ == "__main__":
	try:
		main()
	except KeyboardInterrupt:
		pass

