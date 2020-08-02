# yptools (NIS servers and clients)

NIS stands for Network Information Service. NIS is usually used to
provide `/etc/passwd` and `/etc/group` information throughout the network.
Most Sun-based networks run NIS, and Linux machines can take full
advantage of existing NIS service or provide NIS service themselves.

Configure

1. Edit /etc/defaultdomain

1. Set files at /var/yp (copy .new to .)

1. Set parameters to /etc/default/yp

1. Run it

	/etc/rc.d/rc.yp start

1. Make sure the portmapper (rpcbind) (portmap(8)) is running, and start the server ypserv. The command

    % rpcinfo -u localhost ypserv
	
1. [SERVER - INIT]

//1.1. For the server you need at first certificate, the default filename for this is /etc/rpasswdd.pem.
//The file can be created with the following command:
//
//	openssl req -new -x509 -nodes -days 730 -out /etc/rpasswdd.pem -keyout /etc/rpasswdd.pem

1.1. create /etc/netgroup
	https://www.ibm.com/support/knowledgecenter/ssw_aix_72/filesreference/netgroup.html
	
1.1. if /etc/mail does not exists just link it to where the mail aliases file exists

	ln -s /etc/postfix /etc/mail

1.1. create databases

	/usr/lib/yp/ypinit -m

1. [CLIENT] On a slave make sure that ypwhich -m works.
This means, that your slave must be configured as NIS client before you could run 

1. [CLIENT - ADD HOST]

	/usr/lib/yp/ypinit -s masterhost
