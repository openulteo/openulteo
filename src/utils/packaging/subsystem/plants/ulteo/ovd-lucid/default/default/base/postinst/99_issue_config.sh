#!/bin/sh -e

cat > /etc/issue <<EOF
Ulteo OVD $OVD_VERSION\n \l

EOF

cat > /etc/issue.net <<EOF
Ulteo OVD $OVD_VERSION
EOF
