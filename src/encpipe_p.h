#ifndef encpipe_p_H
#define encpipe_p_H 1

#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <hydrogen.h>

#include "common.h"
#include "safe_rw.h"

#define MIN_BUFFER_SIZE 512
#define MAX_BUFFER_SIZE 0x7fffffff
#define DEFAULT_BUFFER_SIZE (1 * 1024 * 1024)
#define HYDRO_CONTEXT "EncPipe"
#define PWHASH_OPSLIMIT 1000000
#define PWHASH_MEMLIMIT 0
#define PWHASH_THREADS 1

typedef struct Context_ {
    char *         in;
    char *         out;
    unsigned char  key[hydro_secretbox_KEYBYTES];
    unsigned char *buf;
    size_t         sizeof_buf;
    int            fd_in;
    int            fd_out;
    int            encrypt;
    int            has_key;
} Context;

#endif
