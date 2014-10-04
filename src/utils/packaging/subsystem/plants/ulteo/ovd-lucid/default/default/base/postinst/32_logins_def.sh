#!/bin/sh -e

sed -i "s/^UID_MAX.*$/UID_MAX 6000000/" /etc/login.defs
sed -i "s/^GID_MAX.*$/GID_MAX 6000000/" /etc/login.defs

exit 0

