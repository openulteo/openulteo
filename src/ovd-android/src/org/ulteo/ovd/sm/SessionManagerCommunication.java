/*
 * Copyright (C) 2009-2014 Ulteo SAS
 * http://www.ulteo.com
 * Author Thomas MOUTON <thomas@ulteo.com> 2010-2011
 * Author Jeremy DESVAGES <jeremy@ulteo.com> 2010
 * Author Julien LANGLOIS <julien@ulteo.com> 2010, 2011
 * Author David LECHEVALIER <david@ulteo.com> 2010 
 * Author Arnaud LEGRAND <arnaud@ulteo.com> 2010
 * Author David PHAM-VAN <d.pham-van@ulteo.com> 2012, 2014
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

package org.ulteo.ovd.sm;

import java.io.BufferedInputStream;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;
import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSession;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import org.ulteo.ovd.Config;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import android.util.Log;

public class SessionManagerCommunication implements HostnameVerifier, X509TrustManager {
    public static final String SESSION_MODE_REMOTEAPPS = "applications";
    public static final String SESSION_MODE_DESKTOP = "desktop";

    private static final String WEBSERVICE_START_SESSION = "start";
    private static final String WEBSERVICE_SESSION_STATUS = "session_status";
    private static final String WEBSERVICE_NEWS = "news";
    private static final String WEBSERVICE_LOGOUT = "logout";

    public static final String FIELD_LOGIN = "login";
    public static final String FIELD_PASSWORD = "password";
    public static final String FIELD_TOKEN = "token";
    public static final String FIELD_SESSION_MODE = "session_mode";
    public static final String FIELD_ICON_ID = "id";

    public static final String NODE_SETTINGS = "settings";
    public static final String NODE_SETTING = "setting";

    public static final String FIELD_NAME = "name";
    public static final String FIELD_VALUE = "value";

    public static final String VALUE_HIDDEN_PASSWORD = "****";

    public static final String CONTENT_TYPE_FORM = "application/x-www-form-urlencoded";
    private static final String CONTENT_TYPE_XML = "text/xml";

    private static final String REQUEST_METHOD_POST = "POST";
    public static final String REQUEST_METHOD_GET = "GET";

    public static final String SESSION_STATUS_UNKNOWN = "unknown";
    public static final String SESSION_STATUS_ERROR = "error";
    public static final String SESSION_STATUS_INIT = "init";
    public static final String SESSION_STATUS_INITED = "ready";
    public static final String SESSION_STATUS_ACTIVE = "logged";
    public static final String SESSION_STATUS_INACTIVE = "disconnected";
    public static final String SESSION_STATUS_WAIT_DESTROY = "wait_destroy";
    public static final String SESSION_STATUS_DESTROYED = "destroyed";

    private static final int TIMEOUT = 2000;
    private static final int MAX_REDIRECTION_TRY = 5;
    public static final int DEFAULT_PORT = 443;
    public static final int DEFAULT_RDP_PORT = 3389;

    private String host = null;
    private int port;
    private boolean use_https = false;

    private String base_url = null;
    private String service_suffix = Config.SERVICE_SUFFIX;
    private int lastResponseCode = 0;

    private Properties requestProperties = null;
    private Properties responseProperties = null;
    private List<ServerAccess> servers = null;

    private CopyOnWriteArrayList<Callback> callbacks = null;

    private List<String> cookies = null;

    public SessionManagerCommunication(String host_, int port_, boolean use_https_) {
	this.servers = new ArrayList<ServerAccess>();
	this.callbacks = new CopyOnWriteArrayList<Callback>();

	this.cookies = new ArrayList<String>();
	this.host = host_;
	this.port = port_;
	this.use_https = use_https_;

	this.base_url = this.makeUrl("");

    }
    
    public void setServiceSuffix(String value) {
	service_suffix = value;
    }
    
    public int getLastResponseCode() {
    	return lastResponseCode;
    }

    public String getHost() {
	return this.host;
    }

    private String makeUrl(String service) {
	return (this.use_https ? "https" : "http") + "://" + this.host
		+ (this.port == SessionManagerCommunication.DEFAULT_PORT ? "" : ":" + this.port) + "/ovd/client/"
		+ service;
    }

    public static Document getNewDocument() {
	DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
	DocumentBuilder builder;
	try {
	    builder = factory.newDocumentBuilder();
	} catch (ParserConfigurationException e) {
	    e.printStackTrace();
	    return null;
	}

	return builder.newDocument();
    }

    public static String getStringFromNode(Node root) throws IOException {

	StringBuilder result = new StringBuilder();

	if (root.getNodeType() == 3)
	    result.append(root.getNodeValue());
	else {
	    if (root.getNodeType() != 9) {
		StringBuffer attrs = new StringBuffer();
		for (int k = 0; k < root.getAttributes().getLength(); ++k) {
		    attrs.append(" ").append(root.getAttributes().item(k).getNodeName()).append("=\"")
			    .append(root.getAttributes().item(k).getNodeValue()).append("\" ");
		}
		result.append("<").append(root.getNodeName()).append(" ").append(attrs).append(">");
	    } else {
		result.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	    }

	    NodeList nodes = root.getChildNodes();
	    for (int i = 0, j = nodes.getLength(); i < j; i++) {
		Node node = nodes.item(i);
		result.append(getStringFromNode(node));
	    }

	    if (root.getNodeType() != 9) {
		result.append("</").append(root.getNodeName()).append(">");
	    }
	}
	return result.toString();
    }

    public static String Document2String(Document document) {
	try {
	    return getStringFromNode(document.getDocumentElement());
	} catch (IOException e) {
	    e.printStackTrace();
	    return null;
	}
    }

    public boolean askForSession(Properties request) throws SessionManagerException {
	if (request == null || this.requestProperties != null)
	    return false;

	this.requestProperties = request;

	Document doc = getNewDocument();
	if (doc == null)
	    return false;

	Element session = doc.createElement("session");
	doc.appendChild(session);

	if (request.getMode() == Properties.MODE_DESKTOP)
	    session.setAttribute("mode", SESSION_MODE_DESKTOP);
	else if (request.getMode() == Properties.MODE_REMOTEAPPS)
	    session.setAttribute("mode", SESSION_MODE_REMOTEAPPS);

	Element user = doc.createElementNS(null, "user");
	if (request.getToken() != null) {
		user.setAttribute("token", request.getToken());
	} else if (request.getLogin() != null && request.getPassword() != null) {
		user.setAttribute("login", request.getLogin());
		user.setAttribute("password", request.getPassword());
	}

	session.appendChild(user);

	session.setAttribute("language", request.getLang());
	session.setAttribute("timezone", request.getTimeZone());

	if (! request.showDesktop())
		session.setAttribute("no_desktop", "1");

	if (request.hasApplicationId()) {
		Element start = doc.createElementNS(null, "start");
		Element app = doc.createElementNS(null, "application");
		app.setAttribute("id", request.getApplicationId().toString());
		start.appendChild(app);
		session.appendChild(start);
	}

	String data = Document2String(doc);
	if (data == null)
	    return false;

	Object obj = this.askWebservice(WEBSERVICE_START_SESSION + service_suffix, CONTENT_TYPE_XML, REQUEST_METHOD_POST, data);
	if (!(obj instanceof Document))
	    return false;

	return this.parseStartSessionResponse((Document) obj);
    }

    public boolean askForLogout(boolean persistent) throws SessionManagerException {
	Document doc = getNewDocument();
	if (doc == null)
	    return false;

	Element logout = doc.createElement("logout");
	if (persistent)
		logout.setAttribute("mode", "suspend");
	else
		logout.setAttribute("mode", "logout");
	
	doc.appendChild(logout);

	String data = Document2String(doc);
	if (data == null)
	    return false;

	Object obj = this.askWebservice(WEBSERVICE_LOGOUT + service_suffix, CONTENT_TYPE_XML, REQUEST_METHOD_POST, data);
	return obj instanceof Document;
    }

    public String askForSessionStatus() throws SessionManagerException {
	Document doc = getNewDocument();
	if (doc == null)
	    return null;

	Object obj = this.askWebservice(WEBSERVICE_SESSION_STATUS + service_suffix, CONTENT_TYPE_FORM, REQUEST_METHOD_POST, null);
	if (!(obj instanceof Document))
	    return null;

	return this.parseSessionStatusResponse((Document) obj);
    }

    public List<News> askForNews() throws SessionManagerException {
	Object obj = this.askWebservice(WEBSERVICE_NEWS + service_suffix, CONTENT_TYPE_FORM, REQUEST_METHOD_POST, null);
	if (!(obj instanceof Document))
	    return null;

	return this.parseNewsResponse((Document) obj);
    }

    /**
     * send a customized request to the Session Manager
     * 
     * @param webservice
     *            the service path to join on the SM
     * @param content_type
     *            the content type expected to receive
     * @param method
     *            specify GET or POST for the HTTP request
     * @param data
     *            some optional data to send during the request
     * @param showLog
     *            verbosity of this function
     * @return generic {@link Object} result sent by the Session Manager
     * @throws SessionManagerException
     *             generic exception for all failure during the Session manager
     *             communication
     */
    public Object askWebservice(String webservice, String content_type, String method, String data)
	    throws SessionManagerException {
	try {
	    URL url = new URL(this.base_url + webservice);
	    return askWebservice(url, content_type, method, data, MAX_REDIRECTION_TRY);
	} catch (MalformedURLException e) {
	    throw new SessionManagerException(e.getMessage());
	}
    }

    /**
     * send a customized request to the Session Manager
     * 
     * @param url
     *            the complete {@link URL} to join on the SM
     * @param content_type
     *            the content type expected to receive
     * @param method
     *            specify GET or POST for the HTTP request
     * @param data
     *            some optional data to send during the request
     * @param showLog
     *            verbosity of this function
     * @param retry
     *            indicate the maximum of redirected request to make
     * @return generic {@link Object} result sent by the Session Manager
     * @throws SessionManagerException
     *             generic exception for all failure during the Session manager
     *             communication
     */
    private Object askWebservice(URL url, String content_type, String method, String data, int retry)
	    throws SessionManagerException {
		if (Config.DEBUG)
			Log.d(Config.TAG, "Connecting URL: " + url);
		
		lastResponseCode = 0;

	if (retry == 0)
	    throw new SessionManagerException(MAX_REDIRECTION_TRY + " redirections has been done without success");

	Object obj = null;
	HttpURLConnection connexion = null;

	try {
	    connexion = (HttpURLConnection) url.openConnection();
	    connexion.setAllowUserInteraction(true);
	    connexion.setConnectTimeout(TIMEOUT);
	    connexion.setDoInput(true);
	    connexion.setDoOutput(true);
	    connexion.setInstanceFollowRedirects(false);
	    connexion.setRequestMethod(method);
	    connexion.setRequestProperty("Content-type", content_type);
	    connexion.setRequestProperty("Accept-Encoding", "deflate");
	    for (String cookie : this.cookies) {
		connexion.setRequestProperty("Cookie", cookie);
	    }
	    if (this.use_https) {
		SSLContext sc = SSLContext.getInstance("SSL");
		sc.init(null, new TrustManager[] { this }, null);
		SSLSocketFactory factory = sc.getSocketFactory();
		((HttpsURLConnection) connexion).setSSLSocketFactory(factory);
		((HttpsURLConnection) connexion).setHostnameVerifier(this);
	    }
	    connexion.connect();

	    if (data != null) {
		OutputStreamWriter out = new OutputStreamWriter(connexion.getOutputStream());
		out.write(data);
		out.flush();
		out.close();

		try {
		    DocumentBuilder domBuilder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
		    Document xmlOut = domBuilder.parse(new ByteArrayInputStream(data.getBytes()));
		    if (Config.DEBUG)
		    	Log.d(Config.TAG, "Sending XML data: ");

		    dumpXML(xmlOut);
		} catch (Exception ex) {
			if (Config.DEBUG)
				Log.d(Config.TAG, "Send: " + data);
		}
	    }

	    lastResponseCode  = connexion.getResponseCode();
	    String res = connexion.getResponseMessage();
	    String contentType = connexion.getContentType();

	    if (Config.DEBUG)
	    	Log.d(Config.TAG, "Response " + lastResponseCode + " ==> " + res + " type: " + contentType);

	    String http_infos = "\tResponse code: " + lastResponseCode + "\n\tResponse message: " + res + "\n\tContent type: "
		    + contentType;

	    if (lastResponseCode == HttpURLConnection.HTTP_OK) {
		InputStream in = connexion.getInputStream();

		if (contentType.startsWith(CONTENT_TYPE_XML)) {
		    DocumentBuilder domBuilder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
		    Document doc = domBuilder.parse(new InputSource(in));

		    Element rootNode = doc.getDocumentElement();
		    if (rootNode.getNodeName().equalsIgnoreCase("error")) {
			for (Callback each : this.callbacks) {
			    each.reportError(Integer.parseInt(rootNode.getAttribute("id")),
				    rootNode.getAttribute("message"));
			}
		    } else {
			obj = doc;
		    }

		    if (Config.DEBUG) {
		    	Log.d(Config.TAG, "Receiving XML:");
			this.dumpXML((Document) doc);
		    }
		} else {
		    BufferedInputStream d = new BufferedInputStream(in);
		    String buffer = "";
		    for (int c = d.read(); c != -1; c = d.read())
			buffer += (char) c;

		    if (Config.DEBUG)
		    	Log.d(Config.TAG, "Unknown content-type: " + contentType + "buffer: \n" + buffer + "==\n");
		}
		in.close();

		String headerName = null;
		for (int i = 1; (headerName = connexion.getHeaderFieldKey(i)) != null; i++) {
		    if (headerName.equals("Set-Cookie")) {
			String cookie = connexion.getHeaderField(i);

			boolean cookieIsPresent = false;
			for (String value : this.cookies) {
			    if (value.equalsIgnoreCase(cookie))
				cookieIsPresent = true;
			}
			if (!cookieIsPresent)
			    this.cookies.add(cookie);
		    }
		}
	    } else if (lastResponseCode == HttpURLConnection.HTTP_MOVED_TEMP) {
		URL location = new URL(connexion.getHeaderField("Location"));
		if (Config.DEBUG)
			Log.d(Config.TAG, "Redirection: " + location);

		return askWebservice(location, content_type, method, data, retry - 1);
	    } else if (lastResponseCode == HttpURLConnection.HTTP_UNAUTHORIZED) {
		for (Callback c : this.callbacks)
		    c.reportUnauthorizedHTTPResponse(http_infos);
	    } else if (lastResponseCode == HttpURLConnection.HTTP_NOT_FOUND) {
		for (Callback c : this.callbacks)
		    c.reportNotFoundHTTPResponse(http_infos);
	    } else {
		for (Callback c : this.callbacks)
		    c.reportError(lastResponseCode, res);
	    }
	} catch (Exception e) {
	    throw new SessionManagerException(e.getMessage());
	} finally {
	    connexion.disconnect();
	}

	return obj;
    }

    private String parseSessionStatusResponse(Document in) throws SessionManagerException {
	Element rootNode = in.getDocumentElement();

	if (!rootNode.getNodeName().equals("session")) {
	    for (Callback c : this.callbacks)
		c.reportBadXml("");

	    throw new SessionManagerException("bad xml");
	}

	String status = null;
	try {
	    status = rootNode.getAttribute("status");
	} catch (Exception err) {
	    for (Callback c : this.callbacks)
		c.reportBadXml("");

	    throw new SessionManagerException("bad xml");
	}

	return status;
    }

    private List<News> parseNewsResponse(Document in) throws SessionManagerException {
	List<News> newsList = new ArrayList<News>();

	Element rootNode = in.getDocumentElement();

	if (!rootNode.getNodeName().equals("news")) {
	    for (Callback c : this.callbacks)
		c.reportBadXml("");

	    throw new SessionManagerException("bad xml");
	}

	NodeList newNodes = rootNode.getElementsByTagName("new");
	for (int j = 0; j < newNodes.getLength(); j++) {
	    Element newNode = (Element) newNodes.item(j);

	    News n = new News(Integer.parseInt(newNode.getAttribute("id")), newNode.getAttribute("title"), newNode
		    .getFirstChild().getTextContent(), Integer.parseInt(newNode.getAttribute("timestamp")));

	    newsList.add(n);
	}

	return newsList;
    }

    private boolean parseStartSessionResponse(Document document) {
	Element rootNode = document.getDocumentElement();

	if (!rootNode.getNodeName().equals("session")) {
	    if (rootNode.getNodeName().equals("response")) {
		try {
		    String code = rootNode.getAttribute("code");

		    for (Callback c : this.callbacks)
			c.reportErrorStartSession(code);

		    return false;
		} catch (Exception err) {
			if (Config.DEBUG)
				Log.d(Config.TAG, "Error: bad XML #1");
		}

		for (Callback c : this.callbacks)
		    c.reportBadXml("");

		return false;
	    }
	}

	try {
	    int mode = Properties.MODE_ANY;
	    boolean mode_gateway = false;

	    if (rootNode.getAttribute("mode").equals(SESSION_MODE_DESKTOP))
		mode = Properties.MODE_DESKTOP;
	    else if (rootNode.getAttribute("mode").equals(SESSION_MODE_REMOTEAPPS))
		mode = Properties.MODE_REMOTEAPPS;
	    if (mode == Properties.MODE_ANY)
		throw new Exception("bad xml: no valid session mode");

	    Properties response = new Properties(mode);

	    if (rootNode.hasAttribute("mode_gateway")) {
		if (rootNode.getAttribute("mode_gateway").equals("on"))
		    mode_gateway = true;
	    }

	    if (rootNode.hasAttribute("duration"))
		response.setDuration(Integer.parseInt(rootNode.getAttribute("duration")));

	    NodeList settingsNodeList = rootNode.getElementsByTagName(NODE_SETTINGS);
	    if (settingsNodeList.getLength() == 1) {
		Element settingsNode = (Element) settingsNodeList.item(0);

		settingsNodeList = settingsNode.getElementsByTagName(NODE_SETTING);
		for (int i = 0; i < settingsNodeList.getLength(); i++) {
		    Element setting = (Element) settingsNodeList.item(i);

		    try {
			String name = setting.getAttribute(FIELD_NAME);
			String value = setting.getAttribute(FIELD_VALUE);

			Protocol.parseSessionSettings(response, name, value);
		    } catch (org.w3c.dom.DOMException err) {
		    }
		}
	    }

	    NodeList usernameNodeList = rootNode.getElementsByTagName("user");
	    if (usernameNodeList.getLength() == 1) {
		response.setUsername(((Element) usernameNodeList.item(0)).getAttribute("displayName"));
	    }

	    this.responseProperties = response;

	    NodeList serverNodes = rootNode.getElementsByTagName("server");
	    if (serverNodes.getLength() == 0)
		throw new Exception("bad xml: no server node");

	    for (int i = 0; i < serverNodes.getLength(); i++) {
		Element serverNode = (Element) serverNodes.item(i);

		String server_host;
		if (mode_gateway)
		    server_host = this.host;
		else
		    server_host = serverNode.getAttribute("fqdn");

		int server_port = DEFAULT_RDP_PORT;
		if (mode_gateway)
			server_port = this.port;
		else if (serverNode.hasAttribute("port"))
			try {
				server_port = Integer.parseInt(serverNode.getAttribute("port"));
			}
			catch (NumberFormatException ex) {
				Log.w(Config.TAG, "Invalid protocol: server port attribute is not a digit ("+serverNode.getAttribute("port")+")");
			}
		ServerAccess server = new ServerAccess(server_host, server_port, serverNode.getAttribute("login"), serverNode.getAttribute("password"));

		if (mode_gateway)
		    server.token = serverNode.getAttribute("token");

		server.applications = parseApplications(serverNode);
		this.servers.add(server);
	    }
	} catch (Exception err) {
	    for (Callback c : this.callbacks)
		c.reportBadXml(err.toString());
	    return false;
	}

	return true;
    }

    /**
     * parse a DOM {@link Element} list of applications to a standard java
     * {@link ArrayList}
     * 
     * @param serverNode
     *            DOM {@link Element} to parse
     * @return iterable {@link ArrayList} of {@link Application}
     */
    public static ArrayList<Application> parseApplications(Element serverNode) {
	ArrayList<Application> apps = new ArrayList<Application>();

	NodeList applicationsNodes = serverNode.getElementsByTagName("application");
	for (int j = 0; j < applicationsNodes.getLength(); j++) {
	    Element applicationNode = (Element) applicationsNodes.item(j);

	    Application application = new Application(Integer.parseInt(applicationNode.getAttribute("id")),
		    applicationNode.getAttribute("name"));

	    NodeList mimeNodes = applicationNode.getElementsByTagName("mime");
	    for (int k = 0; k < mimeNodes.getLength(); k++) {
		Element mimeNode = (Element) mimeNodes.item(k);
		application.addMime(mimeNode.getAttribute("type"));
	    }
	    apps.add(application);
	}
	return apps;
    }

    /**
     * display all XML data in the standard logger output
     * 
     * @param doc
     *            XML {@link Document} to display
     */
    private void dumpXML(Document doc) {
    	if (Config.DEBUG) {
    		try {
    			Log.d(Config.TAG, getStringFromNode(doc.getDocumentElement()));
    		} catch (IOException e) {
    			Log.e(Config.TAG, e.getMessage());
    		}
    	}
    }

    public void addCallbackListener(Callback c) {
	this.callbacks.add(c);
    }

    public void removeCallbackListener(Callback c) {
	this.callbacks.remove(c);
    }

    public Properties getResponseProperties() {
	return this.responseProperties;
    }

    public List<ServerAccess> getServers() {
	return this.servers;
    }

    @Override
    public boolean verify(String hostname, SSLSession session) {
	return true;
    }

    @Override
    public void checkClientTrusted(X509Certificate[] arg0, String arg1) throws CertificateException {
	return;
    }

    @Override
    public void checkServerTrusted(X509Certificate[] arg0, String arg1) throws CertificateException {
	return;
    }

    @Override
    public X509Certificate[] getAcceptedIssuers() {
	return null;
    }
}
