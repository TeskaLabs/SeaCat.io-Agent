ROOTDIR=../..
BIN=$(ROOTDIR)/bin/seacatiod

OBJS=	config.o \
		app.o \
		reactor.o \
		cntl.o \
		main.o

CPPFLAGS += -DOS_NAME=\"${OS}\"

# Include libtlsca
CPPFLAGS+=-I../libtlsca/
LDLIBS+=-L../libtlsca/ -ltlsca -lpthread

# Include libft
CPPFLAGS+=-I$(ROOTDIR)/libft/include/
LDLIBS+=-L$(ROOTDIR)/libft/src -lft

# Include SeaCat CCore
CPPFLAGS+=-I$(ROOTDIR)/seacatcc/$(TARGET_TRIPLET)/
LDLIBS+=-L$(ROOTDIR)/seacatcc/$(TARGET_TRIPLET)/ -lseacatcc

all: subdirs | $(BIN)

include $(ROOTDIR)/rules.make
