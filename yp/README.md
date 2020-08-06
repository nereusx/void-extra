# yptools (NIS servers and clients)

NIS stands for Network Information Service. NIS is usually used to
provide `/etc/passwd` and `/etc/group` information throughout the network.
Most Sun-based networks run NIS, and Linux machines can take full
advantage of existing NIS service or provide NIS service themselves.

## Install

```
	xbps-install -Sy psmisc rpcbind \
		libnsl libnsl-devel \
		libtirpc libtirpc-devel \
		gdbm gdbm-devel

	./void-build.sh
```

## Configure

1. Edit /etc/defaultdomain

2. [OPTIONAL] Edit /var/yp files

3. [OPTIONAL] Edit /etc/default/yp

4. Run rpcbind

5. Run it

	/etc/rc.d/rc.yp start

6. [SERVER - INIT]

//1.1. For the server you need at first certificate, the default filename for this is /etc/rpasswdd.pem.
//The file can be created with the following command:
//
//	openssl req -new -x509 -nodes -days 730 -out /etc/rpasswdd.pem -keyout /etc/rpasswdd.pem

1.1. [OPTIONAL] Edit `/etc/netgroup`, this is the file of NIS groups. It can be used in `/etc/exports` (NFS).
Try `man netgroup` you must already have it.
	
1.2. [OPTIONAL] Edit `/etc/publickey`.
The `etc/publickey` is a local public key database that is used for secure RPC. The `/etc/publickey` file can be used in conjunction with
or instead of other publickey databases, including the NIS publickey map. Each entry in the database consists of a network user
name (which may refer to either a user or a hostname), followed by the user's public key (in hex notation), a colon, and then the
user's secret key encrypted with a password (also in hex notation).

1.3. [OPTIONAL] If `/etc/mail` does not exists just link it to where the mail's `aliases` file exists

1.4. Create databases

	/usr/lib/yp/ypinit -m
	
7. [SERVER - UPDATE]

1.1. Update databases

	make -C /var/yp

8. [CLIENT/SERVER] if `ypwhich` works, make changes to `/etc/nsswhitch.conf`

9. [OPTIONAL] Edit /etc/pam.d/passwd and append on the line 'nisplus nis'

