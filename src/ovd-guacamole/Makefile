PWD:=$(shell pwd)
BUILD_DIR?=$(PWD)/guac-out
LDCONFIG_FREERDP=$(shell pkg-config $(shell find $(BUILD_DIR) -name freerdp.pc) --libs-only-L)
CFLAGS_FREERDP=$(shell pkg-config $(shell find $(BUILD_DIR) -name freerdp.pc) --cflags)
MAKE_CPU?=8
MKTEMP=$(shell mktemp -d)
LIBDIR?=lib

all: libguac freerdp libguac-client-rdp guacd mvn conf


clean: libguac-clean freerdp-clean libguac-client-rdp-clean guacd-clean
	rm -rf $(BUILD_DIR)


$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)


libguac-prep:
	cd libguac; aclocal
	cd libguac; libtoolize
	cd libguac; autoconf
	cd libguac; automake -a
	cd libguac; ./configure --prefix=$(BUILD_DIR)/usr --libdir=$(BUILD_DIR)/usr/$(LIBDIR)

libguac-build: $(BUILD_DIR) libguac-prep
	make -C libguac -j $(MAKE_CPU)

libguac-install: libguac-build
	cd libguac; make install

libguac: libguac-install

libguac-clean:
	if [ -f libguac/Makefile ]; then make -C libguac distclean; fi


freerdp-prep:
	mkdir -p freerdp/.build
	sed -i 's@$${FREERDP_PLUGIN_PATH}@/usr/$${CMAKE_INSTALL_LIBDIR}/freerdp@g' freerdp/libfreerdp-utils/CMakeLists.txt
	cd freerdp/.build; cmake .. -DCMAKE_INSTALL_PREFIX=$(BUILD_DIR)/usr -DCMAKE_INSTALL_LIBDIR=$(LIBDIR) -DCMAKE_BUILD_TYPE=Release -DWITH_ULTEO_PDF_PRINTER=ON -DWITH_CUPS=OFF -DWITH_X11=OFF -DWITH_XCURSOR=OFF -DWITH_XEXT=OFF -DWITH_XINERAMA=OFF -DWITH_XV=OFF -DWITH_XKBFILE=OFF -DWITH_FFMPEG=OFF -DWITH_ALSA=OFF

freerdp-build: freerdp-prep
	make -C freerdp/.build -j $(MAKE_CPU)

freerdp-install: freerdp-build
	make -C freerdp/.build install

freerdp: freerdp-install

freerdp-clean:
	rm -rf freerdp/.build


libguac-client-rdp-prep: 
	cd libguac-client-rdp; aclocal
	cd libguac-client-rdp; libtoolize
	sed -i 's/AC_FUNC_MALLOC/#AC_FUNC_MALLOC/' libguac-client-rdp/configure.in 
	cd libguac-client-rdp; autoconf
	cd libguac-client-rdp; automake -a
	cd libguac-client-rdp; export LD_LIBRARY_PATH="$(BUILD_DIR)/usr/$(LIBDIR)"; export LDFLAGS="-L$(BUILD_DIR)/usr/$(LIBDIR) $(LDCONFIG_FREERDP)"; export CFLAGS="$(CFLAGS_FREERDP)"; ./configure --prefix=$(BUILD_DIR)/usr --libdir=$(BUILD_DIR)/usr/$(LIBDIR)

libguac-client-rdp-build: libguac-client-rdp-prep
	export LD_LIBRARY_PATH="$(BUILD_DIR)/usr/$(LIBDIR)"; make -C libguac-client-rdp -j $(MAKE_CPU)

libguac-client-rdp-install: libguac-client-rdp-build
	make -C libguac-client-rdp install

libguac-client-rdp: libguac-client-rdp-install

libguac-client-rdp-clean:
	if [ -f libguac-client/Makefile ]; then make -C libguac-client-rdp distclean; fi


guacd-prep:
	cd guacd; aclocal
	cd guacd; autoconf
	cd guacd; automake -a
	cd guacd; env LDFLAGS="-L$(BUILD_DIR)/usr/$(LIBDIR)" CFLAGS="-I$(BUILD_DIR)/usr/include" ./configure --with-init-dir=$(BUILD_DIR)/etc/init.d --with-default-dir=$(BUILD_DIR)/etc/default --prefix=$(BUILD_DIR)/usr --libdir=$(BUILD_DIR)/usr/$(LIBDIR)

guacd-build: guacd-prep
	make -C guacd

guacd-install: guacd-build
	make -C guacd install

guacd: guacd-install

guacd-clean:
	if [ -f guacd/Makefile ]; then make -C guacd distclean; fi


mvn:
	cd common; mvn -B package
	cd common; mvn -B install
	cd common-auth; mvn -B package
	cd common-auth; mvn -B install
	cd common-js; mvn -B package
	cd common-js; mvn -B install
	cd guacamole; mvn -B package
	cd guacamole; mvn -B install
	cd auth-ulteo-ovd; mvn package
	## règle un problème de classe introuvable
	sed -i 's/GuacamoleServerException/Exception/' printing/src/main/java/net/sourceforge/guacamole/net/printing/GuacamolePrinterServlet.java
	cd printing; mvn package
	mkdir -p $(BUILD_DIR)/var/lib/tomcat6/webapps/tmp
	cd $(BUILD_DIR)/var/lib/tomcat6/webapps/tmp; unzip $(PWD)/guacamole/target/guacamole-default-webapp-0.6.0.war
	tar xvzf auth-ulteo-ovd/target/guacamole-auth-ulteo-ovd-0.6.0.tar.gz -C $(BUILD_DIR)/var/lib/tomcat6/webapps/tmp/WEB-INF/ --strip 1
	tar xvzf printing/target/guacamole-printing-0.6.0.tar.gz -C $(BUILD_DIR)/var/lib/tomcat6/webapps/tmp/WEB-INF/ --strip 1
	cd $(BUILD_DIR)/var/lib/tomcat6/webapps/tmp; zip -r ../guacamole.war *
	rm -rf $(BUILD_DIR)/var/lib/tomcat6/webapps/tmp


conf:
	mkdir -p $(BUILD_DIR)/usr/share/tomcat6/lib
	cp guacamole/doc/example/guacamole.properties $(BUILD_DIR)/usr/share/tomcat6/lib/
	sed -i 's/auth-provider:.*$$/auth-provider: net.sourceforge.guacamole.net.auth.ovd.UlteoOVDAuthenticationProvider/' $(BUILD_DIR)/usr/share/tomcat6/lib/guacamole.properties
	sed -i 's/basic-user-mapping:.*$$//' $(BUILD_DIR)/usr/share/tomcat6/lib/guacamole.properties
	sed -i 's#$(BUILD_DIR)##' $(BUILD_DIR)/etc/init.d/guacd.debian
	sed -i 's#$(BUILD_DIR)##' $(BUILD_DIR)/etc/init.d/guacd.redhat
	mkdir -p $(BUILD_DIR)/var/spool/ulteo/pdf-printer
	mkdir -p $(BUILD_DIR)/etc/ulteo/webclient
	cp data/apache.conf $(BUILD_DIR)/etc/ulteo/webclient/apache2-html5.conf
	mkdir -p $(BUILD_DIR)/etc/apache2/conf.d

install:
	cp -r ${BUILD_DIR}/* ${DESTDIR}
