#ifndef TLSCA_SVR__ALL_H_
#define TLSCA_SVR__ALL_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include <ft.h>
#include <tlsca.h>
#include <seacatcc.h>

struct sca_app;
extern struct sca_app sca_app;

#include "config.h"
#include "app.h"
#include "reactor.h"
#include "cntl.h"

enum sca_frame_type
{
	FT_FRAME_TYPE_SEACATCC_WRITE = 0xFFFFF001,
	FT_FRAME_TYPE_SEACATCC_READ  = 0xFFFFF002,
};


#endif //TLSCA_SVR__ALL_H_
