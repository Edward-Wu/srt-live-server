/*
 * This file is part of SLS Live Server.
 *
 * SLS Live Server is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * SLS Live Server is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with SLS Live Server;
 * if not, please contact with the author: Edward.Wu(edward_email@126.com)
 */

#ifndef _COMMON_INCLUDE_
#define _COMMON_INCLUDE_


#include <stddef.h>
#include <stdint.h>


using namespace std;

/**********************************************
 * function return type
 */
/* error handling */
#if EDOM > 0
#define SLSERROR(e) (-(e))   ///< Returns a negative error code from a POSIX error code, to return from library functions.
#define SLSUNERROR(e) (-(e)) ///< Returns a POSIX error code from a library function error return value.
#else
/* Some platforms have E* and errno already negated. */
#define SLSERROR(e) (e)
#define SLSUNERROR(e) (e)
#endif

#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define SLSERRTAG(a, b, c, d) (-(int)MKTAG(a, b, c, d))

#define SLS_OK                      SLSERRTAG(0x0,0x0,0x0,0x0 ) ///< OK
#define SLS_ERROR                   SLSERRTAG(0x0,0x0,0x0,0x1 ) ///<
#define SLSERROR_BSF_NOT_FOUND      SLSERRTAG(0xF8,'B','S','F') ///< Bitstream filter not found
#define SLSERROR_BUG                SLSERRTAG( 'B','U','G','!') ///< Internal bug, also see SLSERROR_BUG2
#define SLSERROR_BUFFER_TOO_SMALL   SLSERRTAG( 'B','U','F','S') ///< Buffer too small
#define SLSERROR_EOF                SLSERRTAG( 'E','O','F',' ') ///< End of file
#define SLSERROR_EXIT               SLSERRTAG( 'E','X','I','T') ///< Immediate exit was requested; the called function should not be restarted
#define SLSERROR_EXTERNAL           SLSERRTAG( 'E','X','T',' ') ///< Generic error in an external library
#define SLSERROR_INVALIDDATA        SLSERRTAG( 'I','N','D','A') ///< Invalid data found when processing input
#define SLSERROR_OPTION_NOT_FOUND   SLSERRTAG(0xF8,'O','P','T') ///< Option not found
#define SLSERROR_PROTOCOL_NOT_FOUND SLSERRTAG(0xF8,'P','R','O') ///< Protocol not found
#define SLSERROR_STREAM_NOT_FOUND   SLSERRTAG(0xF8,'S','T','R') ///< Stream of the StreamID not found
#define SLSERROR_UNKNOWN            SLSERRTAG( 'U','N','K','N') ///< Unknown error, typically from an external library


#define SLSERROR_INVALID_SOCK       SLSERRTAG( 'I','N','V','S') ///< Unknown error, typically from an external library
/**
 * end
 **********************************************/

//#define SAFE_CREATE(p, class_name) { if (!p) new class_name(); }
#define SAFE_DELETE(p) {if (p) { delete p; p = NULL; }}
#define msleep(ms) usleep(ms*1000)

#define TS_UDP_LEN 1316//7*188


int64_t sls_gettime(void);
int64_t sls_gettime_relative(void);
void    sls_gettime_fmt(char *dst, int64_t cur_time_sec, char *fmt);
char  * sls_strupper(char * str);

uint32_t sls_hash_key(const char *data, int len);

#endif
