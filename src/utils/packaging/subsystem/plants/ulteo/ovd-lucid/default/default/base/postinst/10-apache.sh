#!/bin/sh -e

a2dissite 000-default

mv /etc/apache2/ports.conf /etc/apache2/ports.conf.bak
touch /etc/apache2/ports.conf

exit 0
