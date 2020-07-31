#!/bin/sh

list="info man doc lib64"
for f in $list; do
	if [ ! -L /usr/$f ]; then
		echo "/usr/$f is not set. Run slackify script."
		exit 1
	fi
done

./configure && make && make install
