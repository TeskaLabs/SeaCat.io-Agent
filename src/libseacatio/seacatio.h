#ifndef SEACATIO__SC_H_
#define SEACATIO__SC_H_

#include <ft.h>

#define ASSERT_THIS() assert(this != NULL)

#define SEACATIO_CONFIG_FILE SEACATIO_PREFIX "/etc/seacatio.conf"
#define SEACATIO_CONFIG_DIR SEACATIO_PREFIX "/etc/seacatio.d/*.conf"
#define SEACATIO_CNTL_SOCKET_NAME SEACATIO_PREFIX "/var/seacatio.cntl"

extern const char * seacatio_ascii;

#include "proto_spdy.h"
#include "proto_spdy_ping.h"

#endif //SEACATIO__SC_H_
