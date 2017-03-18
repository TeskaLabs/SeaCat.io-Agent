ROOTDIR=../..
BIN=$(ROOTDIR)/bin/scad
LIBEXEC=yes

OBJS=	config.o \
		app.o \
		reactor.o \
		main.o

CPPFLAGS += -DOS_NAME=\"${OS}\"

# Include libtlsca
CPPFLAGS+=-I../libtlsca/
LDLIBS+=-L../libtlsca/ -ltlsca

# Include libft
CPPFLAGS+=-I$(ROOTDIR)/libft/include/
LDLIBS+=-L$(ROOTDIR)/libft/src -lft

# Include SeaCat CCore
CPPFLAGS+=-I$(ROOTDIR)/seacatcc/${OS}/
LDLIBS+=-L$(ROOTDIR)/seacatcc/${OS}/ -lseacatcc

all: subdirs | $(BIN)

include $(ROOTDIR)/rules.make
