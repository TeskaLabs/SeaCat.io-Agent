#!/bin/sh -e 
set -e

## Global configuration values
BASE_URL=http://get.seacat.io/releases/

## Produce an archive code 
function arch_code() {
	# OS / Kernel 
	UNAMESSTR=`uname -s | tr '[:upper:]' '[:lower:]'`

	# Machine
	UNAMEMSTR=`uname -m | tr '[:upper:]' '[:lower:]'`

	ARCHCODE=${UNAMESSTR}-${UNAMEMSTR}

	echo ${ARCHCODE}
}


while getopts "a" opt; do
	case $opt in
		a ) echo $(arch_code) && exit 0;;
		\?) echo "Invalid option: -"$OPTARG"" >&2
			exit 1;;
		: ) echo "Option -"$OPTARG" requires an argument." >&2
			exit 1;;
	esac
done

# ARCHIVE=seacatio-arm-linux-gnueabihf-v1703-alpha.3-debug.tar.gz

# curl ${URL}${ARCHIVE} | tar xz -C /opt/
# chown -Rv root:root /opt/seacatio/
# touch /opt/seacatio/etc/seacat.conf
