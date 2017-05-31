#!/bin/sh -e 

set -e

ARCHIVE=seacatio-arm-linux-gnueabihf-v1703-alpha.3-debug.tar.gz
URL=https://getseacatiostoracc.blob.core.windows.net/getseacatio/releases/

curl ${URL}${ARCHIVE} | tar xz -C /opt/
chown -Rv root:root /opt/seacatio/
touch /opt/seacatio/etc/seacat.conf
