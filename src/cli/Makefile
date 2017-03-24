ROOTDIR=../..
BIN=$(ROOTDIR)/bin/seacatioctl

OBJS=	main.o

# Include libtlsca
CPPFLAGS+=-I../libtlsca/
LDLIBS+=-L../libtlsca/ -ltlsca

# Include libft
CPPFLAGS+=-I$(ROOTDIR)/libft/include/
LDLIBS+=-L$(ROOTDIR)/libft/src -lft

all: subdirs | $(BIN)

include $(ROOTDIR)/rules.make
