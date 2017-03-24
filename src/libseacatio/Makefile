LIB=libseacatio.a
OBJS= \
	proto_spdy.o \
	proto_spdy_ping.o \
	asciiart.o

# Include libft
CPPFLAGS=-I$(ROOTDIR)/libft/include/
LDFLAGS=-L$(ROOTDIR)/libft/src -lft

###

all: | subdirs $(LIB) 

ROOTDIR=../..
include $(ROOTDIR)/rules.make
