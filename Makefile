SUBDIRS=src
CLEANSUBDIRS=libft
EXTRACLEANDIR=distdir

all: .libft subdirs

.libft:
ifndef RELEASE
	@TOPDIR=$(shell pwd) ${MAKE} -C libft
else
	@TOPDIR=$(shell pwd) RELEASE=1 ${MAKE} -C libft
endif

ROOTDIR=.
include $(ROOTDIR)/rules.make


DISTDIR:=$(ROOTDIR)/distdir/seacatio

dist: all
	@echo " [DI]" seacatio-$(VERSION)-$(ARCHCODE).tar.gz
	@mkdir -p $(DISTDIR) $(DISTDIR)/bin $(DISTDIR)/etc
	@cp $(ROOTDIR)/bin/seacatiod $(ROOTDIR)/bin/seacatioctl $(DISTDIR)/bin/
	@tar czf $(ROOTDIR)/seacatio-$(VERSION)-$(ARCHCODE).tar.gz --owner=0 --group=0 -C $(DISTDIR)/.. seacatio
	@rm -rf $(DISTDIR)
