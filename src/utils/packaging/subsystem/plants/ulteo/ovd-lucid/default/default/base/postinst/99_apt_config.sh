#!/bin/sh -e

cat > /etc/apt/sources.list << EOF
# Ubuntu repositories
deb http://archive.ubuntu.com/ubuntu lucid main restricted universe multiverse
deb http://archive.ubuntu.com/ubuntu lucid-updates main restricted universe multiverse
deb http://archive.ubuntu.com/ubuntu lucid-security main restricted universe multiverse
deb http://archive.canonical.com/ubuntu lucid partner
deb http://archive.ubuntu.com/ubuntu/ lucid-backports main restricted universe multiverse
deb http://archive.ubuntu.com/ubuntu/ lucid-proposed restricted main multiverse universe
EOF

cat > /etc/apt/sources.list.d/ulteo_ovd.list << EOF
# Ulteo repositories
deb $PUBLISH_URI/ubuntu lucid main
deb http://archive.ulteo.com/ovd/desktop hardy main universe ulteo
EOF

cat > /etc/apt/apt.conf.d/90_cache_limit <<EOF
APT::Cache-Limit 100000000;
EOF

exit 0
