#!/bin/bash
SCRIPTEXEC_PATH=$(cd `dirname $0`; pwd)
SDK_HOME=$(cd ${SCRIPTEXEC_PATH}/../; pwd)
SDK_PATH_OLD=$(cat ${SCRIPTEXEC_PATH}/ScriptServer | grep "^SCRIPT_PATH=" | awk -F '=' '{print $2}')
FIND_PATH=${SDK_PATH_OLD//\//\\\/}
FIND_PATH=${FIND_PATH//\#/\\\#}
REPLACE_PATH=${SCRIPTEXEC_PATH//\//\\\/}
USER_ID=`id -u`

# Check required priviledge
if [ "$USER_ID" != "0" ]; then
echo "ScriptServer can only be installed by root user or sudoer"
exit 1
fi

if [ -f /etc/debian_version ]; then
    OS=Debian  # XXX or Ubuntu??
    VER=$(cat /etc/debian_version)
elif [ -f /etc/redhat-release ]; then
    # TODO add code for Red Hat and CentOS here
    OS=Redhat  # XXX or CentOs??
    VER=$(uname -r)
else
    OS=$(uname -s)
    VER=$(uname -r)
fi

# check parameter
if [ "1" = "$1" ]; then

sed -i "/^SCRIPT_PATH=*/s/${FIND_PATH}/${REPLACE_PATH}/" ${SCRIPTEXEC_PATH}/ScriptServer
cp -f ${SCRIPTEXEC_PATH}/ScriptServer /etc/init.d/

chmod 777 /etc/init.d/ScriptServer


if [ "Debian" = "$OS" ]; then
	update-rc.d -f ScriptServer defaults 1> /dev/null
elif [ "Redhat" = "$OS" ]; then
	chkconfig --add ScriptServer
	chkconfig ScriptServer on
else
	echo "Can not recognize system"
fi

/etc/init.d/ScriptServer start

elif [ "0" = "$1" ]; then

if [ "Debian" = "$OS" ]; then
        update-rc.d -f ScriptServer remove 1> /dev/null
elif [ "Redhat" = "$OS" ]; then
        chkconfig ScriptServer off
    	chkconfig --del ScriptServer 
else
        echo "Can not recognize system"
fi

if [ -f /etc/init.d/ScriptServer ]; then
rm /etc/init.d/ScriptServer
fi

else
 	echo "wrong parameter"
fi

