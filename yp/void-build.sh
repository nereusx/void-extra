#!/bin/bash

setopt -e

if [ "$USER" != "root" ]; then
	echo "run as root"
	exit 1
fi

cd yp-tools-4.2.2
./configure --prefix=/usr && make && make install
cd ..
cd ypbind-mt-2.4
./configure --prefix=/usr && make && make install
cd ..
cd ypserv-4.0
./configure --prefix=/usr && make && make install
cd ..

# configuration

# /etc/mail/aliases file
if [ ! -d /etc/mail ]; then
	if [ -d /etc/sendmail ]; then
		ln -s /etc/sendmail /etc/mail
	elif [ -d /etc/smtpd ]; then
		ln -s /etc/smtpd /etc/mail
	elif [ -d /etc/postfix ]; then
		ln -s /etc/postfix /etc/mail
	else
		mkdir /etc/mail
	fi
fi
[ ! -f /etc/mail/aliases ] && touch /etc/mail/aliases

# confs
[ ! -f /etc/default/yp ] && cp yp.default /etc/default/yp
[ ! -f /etc/yp.conf ] && cp yp.conf /etc

# rc.yp
if [ ! -d /etc/rc.d ]; then
	mkdir /etc/rc.d
	chmod 750 /etc/rc.d
fi
if [ -d /etc/rc.d ]; then
	install -m 750 rc.yp -o root -g root /etc/rc.d
fi

# other at /etc
[ ! -f /etc/netgroup  ] && touch /etc/netgroup
[ ! -f /etc/publickey ] && touch /etc/publickey
echo
echo "done"
echo
echo "now edit /etc/nsswitch, see example: nsswitch.conf-nis"

