#!/bin/sh

if [ "$(id -u)" != "0" ];then
	echo "This script must be run as root" 1>&2
	exit 1
fi

JUDGERNAME="judger";
JUDGERUID=1537;

if id -u $JUDGERNAME > /dev/null;then
	echo "User Exists";
else
	echo "Creating user and directory"
	mkdir -p /home/$JUDGERNAME
	mkdir -p /home/$JUDGERNAME/etc
	mkdir -p /home/$JUDGERNAME/sim
	cp ./doc/config.json.example /home/$JUDGERNAME/etc/config.json

	useradd -u $JUDGERUID -d /home/$JUDGERNAME -s /bin/bash $JUDGERNAME

	chown -R $JUDGERNAME:$JUDGERNAME /home/$JUDGERNAME
fi
