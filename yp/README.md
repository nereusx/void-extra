# yptools (NIS servers and clients)

NIS stands for Network Information Service. NIS is usually used to
provide `/etc/passwd` and `/etc/group` information throughout the network.
Most Sun-based networks run NIS, and Linux machines can take full
advantage of existing NIS service or provide NIS service themselves.

## Install

```
	xbps-install -Sy \
		rpcbind libnsl libnsl-devel libtirpc libtirpc-devel \
		gdbm gdbm-devel

	./void-build.sh
```

## Configure

1. Edit /etc/defaultdomain

1. Edit /var/yp files

1. Edit /etc/default/yp

1. Run rpcbind

1. Run it

	/etc/rc.d/rc.yp start

1. [SERVER - INIT]

//1.1. For the server you need at first certificate, the default filename for this is /etc/rpasswdd.pem.
//The file can be created with the following command:
//
//	openssl req -new -x509 -nodes -days 730 -out /etc/rpasswdd.pem -keyout /etc/rpasswdd.pem

1.1. Create `/etc/netgroup`, this is the file of NIS groups. It can be used in `/etc/exports` (NFS).
Try `man netgroup` you must already have it.
	
	touch /etc/netgroup

1.1. Create `/etc/publickey`.
The `etc/publickey` is a local public key database that is used for secure RPC. The `/etc/publickey` file can be used in conjunction with
or instead of other publickey databases, including the NIS publickey map. Each entry in the database consists of a network user
name (which may refer to either a user or a hostname), followed by the user's public key (in hex notation), a colon, and then the
user's secret key encrypted with a password (also in hex notation).

	[ ! -e /etc/publickey ] && touch /etc/publickey

1.1. If `/etc/mail` does not exists just link it to where the mail's `aliases` file exists

	ln -s /etc/postfix /etc/mail
	[ ! -e /etc/mail/aliases ] && touch /etc/mail/aliases

1.1. Create databases

	/usr/lib/yp/ypinit -m
	
1. [SERVER - UPDATE]

1.1. Update databases

	make -C /var/yp

1. if ypwhich works, make changes to /etc/nsswhitch.conf

1. [OPTIONAL] Edit /etc/pam.d/passwd and append on the line 'nisplus nis'

