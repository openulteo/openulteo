#!/bin/sh -e

apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 3ACC3965
cat > /etc/apt/sources.list.d/ferramroberto-java.list << EOF
# PPA for the latest version of JAVA
# https://launchpad.net/~ferramroberto/+archive/java
deb http://ppa.launchpad.net/ferramroberto/java/ubuntu lucid main
EOF

apt-key adv --keyserver keyserver.ubuntu.com --recv-keys CE49EC21
cat > /etc/apt/sources.list.d/thunderbird-stable.list << EOF
# Stable releases of Thunderbird
# https://launchpad.net/~mozillateam/+archive/thunderbird-stable
# deb http://ppa.launchpad.net/mozillateam/thunderbird-stable/ubuntu lucid main
EOF

apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1378B444
cat > /etc/apt/sources.list.d/libreoffice.list << EOF
# LibreOffice test builds and backports
# https://launchpad.net/~libreoffice/+archive/ppa
deb http://ppa.launchpad.net/libreoffice/libreoffice-4-0/ubuntu lucid main
EOF

exit 0
