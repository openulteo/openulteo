#!/bin/sh

INITCTL=/sbin/initctl
dpkg-divert --local --rename --add $INITCTL
cp /.garden/preinst/initctl_hook $INITCTL
chmod +x $INITCTL
