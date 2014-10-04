#!/bin/sh -e

mkdir -p /etc/apt/preferences.d

cat > /etc/apt/preferences.d/ovd-desktop << EOF
Package: *
Pin: origin archive.ulteo.com
Pin-Priority: 1001
EOF
