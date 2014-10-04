package net.sourceforge.guacamole.printing;

/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is guacamole-common.
 *
 * The Initial Developer of the Original Code is
 * Jocelyn Delalande <j.delalande@ulteo.com>
 * Portions created by the Initial Developer are Copyright (C) 2012
 * Ulteo SAS <http://www.ulteo.com>
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */


import java.io.IOException;
import java.io.Reader;
import java.io.Writer;
import java.io.File;
import java.io.FileInputStream;
import java.io.OutputStream;
import java.util.Map;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;
import net.sourceforge.guacamole.*;
import net.sourceforge.guacamole.io.GuacamoleReader;
import net.sourceforge.guacamole.io.GuacamoleWriter;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import net.sourceforge.guacamole.protocol.GuacamoleConfiguration;  
import net.sourceforge.guacamole.servlet.GuacamoleHTTPTunnelServlet;
/**
 * A HttpServlet to fetch printed jobs in PDF format and handle printing in general
 *
 * @author Jocelyn Delalande
 */
public class GuacamolePrinterServlet extends HttpServlet {

	private Logger logger = LoggerFactory.getLogger(GuacamoleHTTPTunnelServlet.class);

	public static final String FILE_PATH = "/var/spool/ulteo/pdf-printer";

	/**
	 * Get the PDF via HTTP GET
	 * Format : URL : /opcode/jobId
	 * Code 500 on bad URL, 404 for non-existent job.
	 */
	@Override
	protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException {
		/* Get context */
		HttpSession httpSession = request.getSession(false);

		if(httpSession == null) {
			sendError(response, response.SC_FORBIDDEN , "Printer - no session");
			return;
		}

		/* Get path */
		String path = request.getPathInfo();

		if(path == null) {
			sendError(response, response.SC_FORBIDDEN , "Printer - invalid request : no request");
		}

		String path_split[] = path.toLowerCase().split("/"); /* Match "/" or "//" or "///" or ... */
		String opcode = null;
		String job_id = null;

		try {
			if(path_split.length != 3) {
				throw new Exception("Path length error");
			}

			opcode = path_split[1];
			job_id = path_split[2];

			if(opcode == null || job_id == null) {
				throw new Exception("Path content error");
			}
		} catch(Exception e) {
			sendError(response, response.SC_FORBIDDEN , "Printer - invalid request : path format error : "+e.getMessage());
			return;
		}

		if(opcode.equals("get")) {
			this.opcode_get(request, response, httpSession, job_id);
		} else if(opcode.equals("clear")) {
			this.opcode_clear(request, response, httpSession, job_id);
		} else {
			sendError(response, response.SC_FORBIDDEN , "Printer - invalid operation : " + opcode);
			return;
		}
	}

	protected void sendError(HttpServletResponse response, int code, String message) throws ServletException {
		try {
			response.sendError(code, message);
		} catch(java.io.IOException e) {
			throw new ServletException(message);
		} catch(java.lang.IllegalStateException e) {
			/* Ok, no problem */
			/* See http://docs.oracle.com/javaee/6/api/javax/servlet/http/HttpServletResponse.html#sendError%28int,%20java.lang.String%29 */
		}
	}

	/** 
	 * Sends a PDF file (created by guacd) to the client.
	 */
	protected void opcode_get(HttpServletRequest request, HttpServletResponse response, HttpSession httpSession, String job_id) throws ServletException {
		/* Get config */
		GuacamoleConfiguration config = ((Map<String, GuacamoleConfiguration>)(httpSession.getAttribute("GUAC_CONFIGS"))).get("0");

		/* Get username */
		String username = config.getParameter("username");

		/* Build pdf file path */
		String path = GuacamolePrinterServlet.FILE_PATH + "/" + username + "/" + job_id + ".pdf";
		
		/* Open as file */
		File file = new File(path);

		if(! file.exists()) {
			sendError(response, response.SC_NOT_FOUND, "Printer - no such file : "+ username + "/" + job_id + ".pdf");
			return;
		}

		/* Send the file over HTTP */
		response.setContentType("application/pdf");
		response.setContentLength((int) file.length()); /* file.length() is a long */
		
		FileInputStream in = null;
		OutputStream out = null;

		try {
			in = new FileInputStream(file);
			out = response.getOutputStream();
		
			byte[] buf = new byte[1024];
			int count = 0;
			while ((count = in.read(buf)) >= 0) {
				out.write(buf, 0, count);
			}
			in.close();
			out.close();
		} catch(Exception e) {
			sendError(response, response.SC_FORBIDDEN , "Printer - IOException : "+e.getMessage());
			return;
		}
	}

	protected void opcode_clear(HttpServletRequest request, HttpServletResponse response, HttpSession httpSession, String job_id) throws ServletException {
		sendError(response, response.SC_FORBIDDEN , "Printer - Not implemented");
		return;
	}
}
