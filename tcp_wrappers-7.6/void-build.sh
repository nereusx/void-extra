#!/bin/sh

list="info man doc lib64"
for f in $list; do
	if [ ! -L /usr/$f ]; then
		echo "/usr/$f is not set. Run slackify script."
		exit 1
	fi
done

make linux
list="tcpd safe_finger tcpdchk tcpdmatch try-from"
for f in $list; do
	install -m 755 -o root -g root $f /usr/sbin
done

ldir=/usr/lib
dl=libwrap.so.7.6.1
install -m 755 -o root -g root $dl $ldir
install -m 644 -o root -g root libwrap.a $ldir
ln -sf $ldir/$dl $ldir/libwrap.so.7
ln -sf $ldir/$dl $ldir/libwrap.so

ldconfig

