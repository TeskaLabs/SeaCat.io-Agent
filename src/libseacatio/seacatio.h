#ifndef SEACATIO__SC_H_
#define SEACATIO__SC_H_

#include <ft.h>

#define ASSERT_THIS() assert(this != NULL)
#define LIBTLSCA_CONFIG_FILE "./etc/seacat.conf"

extern const char * seacatio_ascii;

#include "proto_spdy.h"
#include "proto_spdy_ping.h"

#endif //SEACATIO__SC_H_
