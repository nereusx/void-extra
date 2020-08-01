#!/bin/sh

list="info man doc lib64"
for f in $list; do
	if [ ! -L /usr/$f ]; then
		echo "/usr/$f is not set. Run slackify script."
		exit 1
	fi
done

TMP=${TMP:-/tmp}
PKG=$TMP/package-uucp
CFLAGS="-O2 -fPIC"
UUCPID=500

rm -rf $PKG
mkdir -p $TMP $PKG

./configure \
	--prefix=/usr \
	--with-oldconfigdir=/etc/uucp/oldconfig \
	--with-newconfigdir=/etc/uucp

make || exit 1
make install DESTDIR=$PKG || exit 1

strip --strip-unneeded $PKG/usr/bin/* $PKG/usr/sbin/*

gzip -9 $PKG/usr/man/man?/*
gzip -9 $PKG/usr/info/*
rm -f $PKG/usr/info/dir.gz

groupadd -f uucp -g $UUCPID
useradd uucp -g uucp -u $UUCPID
usermod uucp -g $UUCPID -s /sbin/nologin

# Setuid uucp binaries may only be run by members of the uucp group:
( cd $PKG/usr/bin
  chgrp uucp cu uucp uuname uustat uux
  chmod 4554 cu uucp uuname uustat uux
  cd ../sbin
  chgrp uucp uucico uuxqt
  chmod 4554 uucico uuxqt
)

# install:
#
cp -a $PKG/usr/bin/* /usr/bin/
cp -a $PKG/usr/sbin/* /usr/bin/
cp -a $PKG/usr/share/* /usr/share/
[ -e /install ] && rm -rf /install
rm -rf $PKG
#echo $PKG
