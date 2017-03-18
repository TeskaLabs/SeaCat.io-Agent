# Optional include of machine-specific configuration
-include $(ROOTDIR)/rules.site

VERSION=$(shell git describe --abbrev=7 --tags --dirty --always)

ifndef RELEASE
CFLAGS+=-O0 -fno-strict-aliasing -ggdb -DDEBUG=1
else
CFLAGS+=-O2 -fno-strict-aliasing -DRELEASE=1
endif

ifndef NOSTRIP
CFLAGS+=-ggdb
endif 

CFLAGS+=-Wall -std=gnu99 -static -fpic
LDLIBS+=-lev -lm

ifdef RELEASE
CPPFLAGS+=-DSEACAT_VERSION=\"${VERSION}\"
else
CPPFLAGS+=-DSEACAT_VERSION=\"${VERSION}-debug\"
endif

# Detect OS

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    OS = lnx
endif
ifeq ($(UNAME_S),Darwin)
    OS = mac
endif

# OpenSSL
OPENSSLINCPATH?=/usr/local/include
OPENSSLLIBPATH?=/usr/local/lib

CPPFLAGS+=-I${OPENSSLINCPATH}

ifdef OPENSSLDYNAMIC
LDLIBS+=-L${OPENSSLLIBPATH} -lssl -lcrypto
else
LDLIBS+=${OPENSSLLIBPATH}/libssl.a ${OPENSSLLIBPATH}/libcrypto.a
endif

.PHONY: clean all subdirs


# Basic commands

subdirs:
	@$(foreach dir, $(SUBDIRS), echo " [CD]" $(dir) && $(MAKE) -C $(dir);)

clean:
	@echo " [RM] in" $(CURDIR)
	@$(RM) $(BIN) $(LIB) $(EXT) $(CLEAN) $(EXTRACLEAN) ${OBJS} *.o
	@$(foreach dir, $(SUBDIRS) $(CLEANSUBDIRS), $(MAKE) -C $(dir) clean;)


# Compile command

.c.o:
	@echo " [CC]" $@
	@$(COMPILE.c) $(OUTPUT_OPTION) $<


# Link commands

${BIN}: ${OBJS}
	@rm -rf ${BIN}
	@echo " [LD]" $@
	@$(LINK.o) -fPIE $^ ${UTOBJS} $(LOADLIBES) $(LDLIBS) -ldl -o $@
ifdef RELEASE
ifndef NOSTRIP
	@echo " [ST]" $@
	@strip $@
endif
	@bash -c "if [[ `nm $@ | grep _ev_ | wc -l` -gt 0 ]] ; then \
		 echo \"Produced binary contains unwanted symbols!\"; \
		 exit 1; \
	fi"
endif
ifdef LIBEXEC
	@echo " [CM]" $@
	@chmod 0500 ${BIN}
endif

${LIB}: ${OBJS}
	@echo " [AR]" $@
	@$(AR) -cr $@ $^ ${LIBEXTRAOBJS}

$(EXT): ${OBJS}
	@echo " [AR]" $@
	@$(AR) -cr $@ $^ ${LIBEXTRAOBJS}
