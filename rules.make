# Optional include of machine-specific configuration
-include $(ROOTDIR)/rules.site

ifndef RELEASE
VERSION=$(shell git describe --abbrev=7 --tags --dirty --always)-debug
CFLAGS+=-O0 -fno-strict-aliasing -ggdb -DDEBUG=1
else
VERSION=$(shell git describe --abbrev=7 --tags --dirty --always)
CFLAGS+=-O2 -fno-strict-aliasing -DRELEASE=1
endif

CFLAGS+=-Wall -std=gnu99 -static -fpic -fPIC
LDLIBS+=-lm
CPPFLAGS+=-DSEACAT_VERSION=\"${VERSION}\"

SEACATIO_PREFIX=/opt/seacatio

# Detect OS - must be a three letter, SeaCat compatible OS platform code
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    OS = lnx
endif
ifeq ($(UNAME_S),Darwin)
    OS = mac
endif


# Obtain target triplet
# http://wiki.osdev.org/Target_Triplet
TARGET_TRIPLET := $(shell $(CC) -dumpmachine)
CPPFLAGS+=-DSEACAT_TARGET_TRIPLET=\"${TARGET_TRIPLET}\" -DSEACATIO_PREFIX=\"${SEACATIO_PREFIX}\"


# OpenSSL
OPENSSLINCPATH?=/usr/openssl/include
OPENSSLLIBPATH?=/usr/openssl/lib

CPPFLAGS+=-I${OPENSSLINCPATH}

ifdef OPENSSLDYNAMIC
LDLIBS+=-L${OPENSSLLIBPATH} -lssl -lcrypto
else
LDLIBS+=${OPENSSLLIBPATH}/libssl.a ${OPENSSLLIBPATH}/libcrypto.a
endif


# libev
LIBEVINCPATH?=/usr/libev/include
LIBEVLIBPATH?=/usr/libev/lib

CPPFLAGS+=-I${LIBEVINCPATH}

ifdef LIBEVDYNAMIC
LDLIBS+=-L${LIBEVLIBPATH} -lev
else
LDLIBS+=${LIBEVLIBPATH}/libev.a
endif


.PHONY: clean all subdirs dist


# Basic commands

subdirs:
	@$(foreach dir, $(SUBDIRS), echo " [CD]" $(dir) && $(MAKE) -C $(dir);)

clean:
	@echo " [RM] in" $(CURDIR)
	@$(RM) $(BIN) $(LIB) $(EXT) $(CLEAN) $(EXTRACLEAN) ${OBJS} *.o
	@$(RM) -rf $(EXTRACLEANDIR)
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
