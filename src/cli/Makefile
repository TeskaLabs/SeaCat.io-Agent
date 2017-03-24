ROOTDIR=../..
BIN=$(ROOTDIR)/bin/seacatioctl

OBJS=	main.o

# Include libseacatio
CPPFLAGS+=-I../libseacatio/
LDLIBS+=-L../libseacatio/ -lseacatio

# Include libft
CPPFLAGS+=-I$(ROOTDIR)/libft/include/
LDLIBS+=-L$(ROOTDIR)/libft/src -lft

all: subdirs | $(BIN)

include $(ROOTDIR)/rules.make
