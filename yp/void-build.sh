#!/bin/bash

setopt -e

cd yp-tools-4.2.2
./configure --prefix=/usr && make && make install
cd ..
cd ypbind-mt-2.4
./configure --prefix=/usr && make && make install
cd ..
cd ypserv-4.0
./configure --prefix=/usr && make && make install
cd ..

[ ! -f /etc/default/yp ] && cp yp.default /etc/default/yp
[ ! -f /etc/yp.conf ] && cp yp.conf /etc
if [ -d /etc/rc.d ]; then
	install -m 750 rc.yp -o root -g root /etc/rc.d
fi
echo
echo "done"
echo
echo "check: nsswitch.conf-nis"
