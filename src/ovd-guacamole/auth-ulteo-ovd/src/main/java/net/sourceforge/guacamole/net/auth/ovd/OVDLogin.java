package net.sourceforge.guacamole.net.auth.ovd;

/*
 *  Guacamole - Clientless Remote Desktop
 *  Copyright (C) 2012  Ulteo SAS
 *  http://www.ulteo.com
 *  Author Alexandre CONFIANT-LATOUR <a.confiant@ulteo.com> 2012
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import net.sourceforge.guacamole.net.auth.AuthenticationProvider;
import net.sourceforge.guacamole.net.auth.Credentials;

import java.io.IOException;
import java.util.Map;
import java.util.HashMap;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;
import net.sourceforge.guacamole.GuacamoleException;
import net.sourceforge.guacamole.protocol.GuacamoleConfiguration;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class OVDLogin extends HttpServlet {
	private static final String CONFIGURATIONS_ATTRIBUTE = "GUAC_CONFIGS";
	private static final String CREDENTIALS_ATTRIBUTE = "GUAC_CREDS";
	private Logger logger = LoggerFactory.getLogger(OVDLogin.class);

	@Override
	public void init() throws ServletException {
	}

	@Override
	protected void service(HttpServletRequest request, HttpServletResponse response) throws IOException {
		/* Get the user session */
		HttpSession httpSession = request.getSession(true);

		/* Guacamole configuration */
		Credentials credentials = new Credentials();
		GuacamoleConfiguration g_conf = new GuacamoleConfiguration();
		Map<String, GuacamoleConfiguration> configs = (Map<String, GuacamoleConfiguration>) httpSession.getAttribute(CONFIGURATIONS_ATTRIBUTE);

		if(configs == null) {
			/* No existing configs, creating a new one */
			configs = new HashMap<String, GuacamoleConfiguration>();
		}

		/* Configure the guacamole connection */
		g_conf.setProtocol("rdp");
		g_conf.setParameter("hostname", request.getParameter("server"));
		g_conf.setParameter("port", request.getParameter("port"));
		g_conf.setParameter("username", request.getParameter("username"));
		g_conf.setParameter("password", request.getParameter("password"));
		g_conf.setParameter("width", request.getParameter("width"));
		g_conf.setParameter("height", request.getParameter("height"));
		
		String mode = request.getParameter("mode");
		if (mode != null) {
			if (mode.equals("desktop"))
				g_conf.setParameter("initial-program", "OvdDesktop");
			else
				g_conf.setParameter("initial-program", "OvdRemoteApps");
		}
		configs.put(request.getParameter("id"), g_conf);

		// Associate configs with session
		httpSession.setAttribute(CONFIGURATIONS_ATTRIBUTE, configs);
		httpSession.setAttribute(CREDENTIALS_ATTRIBUTE, credentials);
	}
}
