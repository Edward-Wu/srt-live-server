
/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2019-2020 Edward.Wu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _COMMON_INCLUDE_
#define _COMMON_INCLUDE_


#include <stddef.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <unistd.h>

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

#define TS_PACK_LEN 188
#define TS_UDP_LEN 1316 //7*188
#define STR_MAX_LEN 1024
#define URL_MAX_LEN STR_MAX_LEN
#define STR_DATE_TIME_LEN 32
#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46
#define IP_MAX_LEN INET6_ADDRSTRLEN



int64_t sls_gettime_ms(void);//rturn millisecond
int64_t sls_gettime(void);//rturn microsecond
void    sls_gettime_fmt(char *dst, int64_t cur_time_sec, char *fmt);
void    sls_gettime_default_string(char *cur_time);
char  * sls_strupper(char * str);
void    sls_remove_marks(char *s);

uint32_t sls_hash_key(const char *data, int len);
int      sls_gethostbyname(const char *hostname, char *ip);

int sls_read_pid();
int sls_write_pid(int pid);
int sls_remove_pid();
int sls_send_cmd(const char *cmd);

void sls_split_string(std::string str, std::string separator, std::vector<std::string> &result, int count=-1);
std::string sls_find_string(std::vector<std::string> &src, std::string &dst);


/*
 * parse ts packet
 */

#define TS_SYNC_BYTE 0x47
#define TS_PACK_LEN 188
#define INVALID_PID -1
#define PAT_PID 0
#define INVALID_DTS_PTS -1
#define MAX_PES_PAYLOAD 200 * 1024

typedef struct ts_info {
    int      es_pid;
    int64_t  dts;
    int64_t  pts;
    bool     need_spspps;
    int      sps_len;
    uint8_t  sps[TS_PACK_LEN];
    int      pps_len;
    uint8_t  pps[TS_PACK_LEN];
    uint8_t  ts_data[TS_UDP_LEN];
    uint8_t  pat[TS_PACK_LEN];
    int      pat_len;
    int      pmt_pid;
    uint8_t  pmt[TS_PACK_LEN];
    int      pmt_len;

};
void sls_init_ts_info(ts_info *ti);
int  sls_parse_ts_info(const uint8_t *packet, ts_info *ti);


#endif
