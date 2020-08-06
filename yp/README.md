# yptools (NIS servers and clients)

NIS stands for Network Information Service. NIS is usually used to
provide `/etc/passwd` and `/etc/group` information throughout the network.
Most Sun-based networks run NIS, and Linux machines can take full
advantage of existing NIS service or provide NIS service themselves.

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

1.1. Create `/etc/netgroup`, this is the file of NIS groups. It can be used in /etc/exports (NFS)

	man netgroup

1.1. create /etc/publickey

	touch /etc/publickey

1.1. if /etc/mail does not exists just link it to where the mail aliases file exists

	ln -s /etc/postfix /etc/mail

1.1. create databases

	/usr/lib/yp/ypinit -m
	
1. [SERVER - UPDATE]

1.1. Update databases

	make -C /var/yp

1. if ypwhich works, make changes to /etc/nsswhitch.conf

1. [OPTIONAL] Edit /etc/pam.d/passwd and append on the line 'nisplus nis'

