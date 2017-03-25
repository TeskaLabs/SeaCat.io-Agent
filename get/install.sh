#!/bin/bh -e 

ARCHIVE=seacatio-arm-linux-gnueabihf-vv1703-alpha.1.tar.gz
URL=https://getseacatiostoracc.blob.core.windows.net/getseacatio/releases/

curl ${URL}${ARCHIVE} | tar xz -C /opt/
chown -Rv root:root /opt/seacatio/