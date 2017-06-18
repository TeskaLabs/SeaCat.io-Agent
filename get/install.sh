#!/bin/sh -e 
set -e

## Detect CPU
CPU='unknown'
unamemstr=`uname -m`
case $unamemstr in
	'x86_64')
		CPU='x86_64'
		;;
	arm*)
		CPU='arm~'$unamemstr
		;;
	*)
		CPU='unknown'
		;;
esac

## Detect platform and OS
PLATFORM='unknown'
OS='unknown'
unamestr=`uname`
case $unamestr in
	'Linux')
		PLATFORM='linux'
		OS='gnu'
		;;
	'FreeBSD')
		PLATFORM='freebsd'
		;;
	'WindowsNT')
		PLATFORM='win'
		;;
	'Darwin')
		PLATFORM='apple'
		OS='darwin~'`uname -r`
		;;
	'SunOS')
		PLATFORM='solaris'
		;;
	'AIX')
		PLATFORM='aix'
		;;
	*)
		PLATFORM='unknown'
		;;
esac

echo ${CPU}-${PLATFORM}-${OS}
exit 0

ARCHIVE=seacatio-arm-linux-gnueabihf-v1703-alpha.3-debug.tar.gz
URL=https://getseacatiostoracc.blob.core.windows.net/getseacatio/releases/

curl ${URL}${ARCHIVE} | tar xz -C /opt/
chown -Rv root:root /opt/seacatio/
touch /opt/seacatio/etc/seacat.conf
