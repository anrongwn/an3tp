#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "nng/nng.h"
#include "nng/protocol/reqrep0/rep.h"
#include "nng/protocol/reqrep0/req.h"
#include <nng/supplemental/util/platform.h>

#define CLIENT "client"
#define SERVER "server"
#define DATECMD 1

#define PUT64(ptr, u)                                                                                                  \
	do {                                                                                                               \
		(ptr)[0] = (uint8_t)(((uint64_t)(u)) >> 56);                                                                   \
		(ptr)[1] = (uint8_t)(((uint64_t)(u)) >> 48);                                                                   \
		(ptr)[2] = (uint8_t)(((uint64_t)(u)) >> 40);                                                                   \
		(ptr)[3] = (uint8_t)(((uint64_t)(u)) >> 32);                                                                   \
		(ptr)[4] = (uint8_t)(((uint64_t)(u)) >> 24);                                                                   \
		(ptr)[5] = (uint8_t)(((uint64_t)(u)) >> 16);                                                                   \
		(ptr)[6] = (uint8_t)(((uint64_t)(u)) >> 8);                                                                    \
		(ptr)[7] = (uint8_t)((uint64_t)(u));                                                                           \
	} while (0)

#define GET64(ptr, v)                                                                                                  \
	v = (((uint64_t)((uint8_t)(ptr)[0])) << 56) + (((uint64_t)((uint8_t)(ptr)[1])) << 48) +                            \
		(((uint64_t)((uint8_t)(ptr)[2])) << 40) + (((uint64_t)((uint8_t)(ptr)[3])) << 32) +                            \
		(((uint64_t)((uint8_t)(ptr)[4])) << 24) + (((uint64_t)((uint8_t)(ptr)[5])) << 16) +                            \
		(((uint64_t)((uint8_t)(ptr)[6])) << 8) + (((uint64_t)(uint8_t)(ptr)[7]))

// server handle
int server(const char *url, const char *name);

// client handle
int client(const char *url, const char *name);

#ifdef __cplusplus
}
#endif