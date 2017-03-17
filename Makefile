SUBDIRS=src
CLEANSUBDIRS=libft

all: .libft subdirs

.libft:
ifndef RELEASE
	@TOPDIR=$(shell pwd) ${MAKE} -C libft
else
	@TOPDIR=$(shell pwd) RELEASE=1 ${MAKE} -C libft
endif

ROOTDIR=.
include $(ROOTDIR)/rules.make
