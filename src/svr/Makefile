ROOTDIR=../..
BIN=$(ROOTDIR)/bin/seacatiod

OBJS=	config.o \
		app.o \
		reactor.o \
		cntl.o \
		connectivity.o \
		main.o

CPPFLAGS += -DOS_NAME=\"${OS}\"
LDLIBS+=-lpthread

# Include libseacatio
CPPFLAGS+=-I../libseacatio/
LDLIBS+=-L../libseacatio/ -lseacatio

# Include libft
CPPFLAGS+=-I$(ROOTDIR)/libft/include/
LDLIBS+=-L$(ROOTDIR)/libft/src -lft

# Include SeaCat CCore
CPPFLAGS+=-I$(ROOTDIR)/seacatcc/$(ARCHCODE)/
LDLIBS+=-L$(ROOTDIR)/seacatcc/$(ARCHCODE)/ -lseacatcc

all: subdirs | $(BIN)

include $(ROOTDIR)/rules.make

# Add CoreFoundation and IOKit on Macos (used for capabilties)
ifeq ($(OS), mac)
LDLIBS+=-framework CoreFoundation -framework IOKit
endif
