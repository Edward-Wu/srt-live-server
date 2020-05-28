
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


#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <cstdarg>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <string>
#include <vector>


#include "common.hpp"


/**
* sls_format
 */
std::string sls_format(const char *pszFmt, ...)
{
    std::string str;
    /*
    va_list args;
    va_start(args, pszFmt);
    {
        int nLength = vscprintf(pszFmt, args);
        nLength += 1;  //include \0
        std::vector<char> vectorChars(nLength);
        vsnprintf(vectorChars.data(), nLength, pszFmt, args);
        str.assign(vectorChars.data());
    }
    va_end(args);
    */
    return str;
}

#define HAVE_GETTIMEOFDAY 1

int64_t sls_gettime_ms(void)//rturn millisecond
{
	return sls_gettime()/1000;
}

int64_t sls_gettime(void)//rturn micro-second
{
#if HAVE_GETTIMEOFDAY
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
#elif HAVE_GETSYSTEMTIMEASFILETIME
    FILETIME ft;
    int64_t t;
    GetSystemTimeAsFileTime(&ft);
    t = (int64_t)ft.dwHighDateTime << 32 | ft.dwLowDateTime;
    return t / 10 - 11644473600000000; /* Jan 1, 1601 */
#else
    return -1;
#endif
}

void sls_gettime_default_string(char *cur_time)
{
	if (NULL == cur_time) {
		return ;
	}
    int64_t cur_time_sec = sls_gettime()/1000000;
    sls_gettime_fmt(cur_time, cur_time_sec, "%Y-%m-%d %H:%M:%S");
}

void sls_gettime_fmt(char *dst, int64_t cur_time_sec, char *fmt)
{
    time_t rawtime;
    struct tm * timeinfo;
    char timef[32] = {0};

    time (&rawtime);
    rawtime = (time_t)cur_time_sec;
    timeinfo = localtime (&rawtime);
    strftime(timef, sizeof(timef), fmt, timeinfo);
    strcpy(dst, timef);
    return ;
}

char * sls_strupper(char * str)
{
    char *orign=str;
    for (; *str!='\0'; str++)
        *str = toupper(*str);
    return orign;
}

#define sls_hash(key, c)   ((uint32_t) key * 31 + c)
uint32_t sls_hash_key(const char *data, int len)
{
	//copy form ngx
    uint32_t  i, key;

    key = 0;

    for (i = 0; i < len; i++) {
        key = sls_hash(key, data[i]);
    }
    return key;
}

int sls_gethostbyname(const char *hostname, char *ip)
{
    char   *ptr, **pptr;
    struct hostent *hptr;
    char   str[32];
    ptr = (char *)hostname;

    if((hptr = gethostbyname(ptr)) == NULL)
     {
         printf("sls_gethostbyname: gethostbyname error for host:%s\n", ptr);
         return 0;
     }

/*
    printf("official hostname:%s\n",hptr->h_name);
     for(pptr = hptr->h_aliases; *pptr != NULL; pptr++)
         printf(" alias:%s\n",*pptr);
*/

     switch(hptr->h_addrtype)
     {
         case AF_INET:
         case AF_INET6:
             //pptr=hptr->h_addr_list;
             //for(; *pptr!=NULL; pptr++)
             //    printf(" address:%s\n",
             //           inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));

        	 //copy the 1st ip
             strcpy(ip, inet_ntop(hptr->h_addrtype, hptr->h_addr, str, sizeof(str)));
         break;
         default:
             printf("sls_gethostbyname: unknown address type\n");
         break;
     }

     return 0;
 }

static void av_str_replace(char *buf, const char dst, const char ch)
{
    char *p = NULL;
    while(1) {
        p = strchr(buf, dst);
        if (p == NULL)
            break;
        *p++ = ch;
    }
}
static size_t max_alloc_size= 1024000;// max 1M

static void *av_malloc(size_t size)
{
    void *ptr = NULL;

    /* let's disallow possibly ambiguous cases */
    if (size > (max_alloc_size - 32))
        return NULL;

#if HAVE_POSIX_MEMALIGN
    if (size) //OS X on SDK 10.6 has a broken posix_memalign implementation
    if (posix_memalign(&ptr, ALIGN, size))
        ptr = NULL;
#elif HAVE_ALIGNED_MALLOC
    ptr = _aligned_malloc(size, ALIGN);
#elif HAVE_MEMALIGN
#ifndef __DJGPP__
    ptr = memalign(ALIGN, size);
#else
    ptr = memalign(size, ALIGN);
#endif
    /* Why 64?
     * Indeed, we should align it:
     *   on  4 for 386
     *   on 16 for 486
     *   on 32 for 586, PPro - K6-III
     *   on 64 for K7 (maybe for P3 too).
     * Because L1 and L2 caches are aligned on those values.
     * But I don't want to code such logic here!
     */
    /* Why 32?
     * For AVX ASM. SSE / NEON needs only 16.
     * Why not larger? Because I did not see a difference in benchmarks ...
     */
    /* benchmarks with P3
     * memalign(64) + 1          3071, 3051, 3032
     * memalign(64) + 2          3051, 3032, 3041
     * memalign(64) + 4          2911, 2896, 2915
     * memalign(64) + 8          2545, 2554, 2550
     * memalign(64) + 16         2543, 2572, 2563
     * memalign(64) + 32         2546, 2545, 2571
     * memalign(64) + 64         2570, 2533, 2558
     *
     * BTW, malloc seems to do 8-byte alignment by default here.
     */
#else
    ptr = malloc(size);
#endif

    if (ptr)
        memset(ptr, 0, size);
    return ptr;
}
static void av_free(void *arg)
{
    //memcpy(arg, &(void *){ NULL }, sizeof(arg));
    free(arg);
}

static char *av_strdup(const char *s)
{
    char *ptr = NULL;
    if (s) {
        size_t len = strlen(s) + 1;
        ptr = (char *)av_malloc(len);
        if (ptr)
            memcpy(ptr, s, len);
    }
    return ptr;
}

/**
 * Locale-independent conversion of ASCII characters to lowercase.
 */
static inline int av_tolower(int c)
{
    if (c >= 'A' && c <= 'Z')
        c ^= 0x20;
    return c;
}

static int av_strncasecmp(const char *a, const char *b, size_t n)
{
    uint8_t c1, c2;
    if (n <= 0)
        return 0;
    do {
        c1 = av_tolower(*a++);
        c2 = av_tolower(*b++);
    } while (--n && c1 && c1 == c2);
    return c1 - c2;
}

int sls_mkdir_p(const char *path)
{

    int ret = 0;
    char *temp = av_strdup(path);
    char *pos = temp;
    char tmp_ch = '\0';

    if (!path || !temp) {
        return -1;
    }

    if (!av_strncasecmp(temp, "/", 1) || !av_strncasecmp(temp, "\\", 1)) {
        pos++;
    } else if (!av_strncasecmp(temp, "./", 2) || !av_strncasecmp(temp, ".\\", 2)) {
        pos += 2;
    }

    for ( ; *pos != '\0'; ++pos) {
        if (*pos == '/' || *pos == '\\') {
            tmp_ch = *pos;
            *pos = '\0';
            ret = mkdir(temp, 0755);
            *pos = tmp_ch;
        }
    }

    if ((*(pos - 1) != '/') || (*(pos - 1) != '\\')) {
        ret = mkdir(temp, 0755);
    }

    av_free(temp);
    return ret;
}

void sls_remove_marks(char * s) {
    int len = strlen(s);
    if (len < 2)//pair
        return;

    if ((s[0] == '\'' && s[len-1] == '\'')
     || (s[0] == '"' && s[len-1] == '"')) {
        for(int i=0; i < len-2; i ++) {
            s[i] = s[i+1];
        }
        s[len-2] = 0x0;
    }
}

static char pid_path_name[] = "/opt/soft/sls";
static char pid_file_name[] = "/opt/soft/sls/pid.txt";
int sls_read_pid()
{
	struct stat stat_file;
	int ret = stat(pid_file_name, &stat_file);
    if (0 != ret) {
    	printf("no pid file='%s'.\n", pid_file_name);
    	return 0;
    }

    int fd = open(pid_file_name, O_RDONLY);
    if (0 == fd)  {
    	printf("open file='%s' failed.\n", pid_file_name);
    	return 0;
    }
    char pid[128] = {0};
    int n = read(fd, pid, sizeof(pid));
    ret = atoi(pid);
    close(fd);
    return ret;
}

int sls_write_pid(int pid)
{
	struct stat stat_file;
	int fd = 0;

	if (sls_mkdir_p(pid_path_name) == -1 && errno != EEXIST) {
	    printf( "mkdir '%s' failed.\n", pid_path_name);
	    return -1;
	}
    fd = open(pid_file_name, O_WRONLY|O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IXOTH);

    if (0 == fd) {
    	printf("open file='%s' failed, '%s'.\n", pid_file_name, strerror(errno));
    	return -1;
    }
    char buf[128] = {0};
    sprintf(buf, "%d", pid);
    write(fd, buf, strlen(buf));
    close(fd);
	printf("write pid ok, file='%s', pid=%s.\n", pid_file_name, buf);
    return 0;

}

int sls_remove_pid()
{
	struct stat stat_file;
    if (0 == stat(pid_file_name, &stat_file)) {
    	FILE *fd = fopen(pid_file_name, "w");
        fclose(fd);
    }
    return 0;
}

int sls_send_cmd(const char *cmd)
{
	if (NULL == cmd) {
    	printf("sls_send_cmd failed, cmd is null.\n");
		return SLS_ERROR;
	}
	int pid = sls_read_pid();
	if (0 >= pid) {
    	printf("sls_send_cmd failed, pid is invalid.\n", pid);
        return SLS_OK;
	}

	//reload?
    if (strcmp(cmd, "reload") == 0) {
    	//reload the existed sls
     	printf("sls_send_cmd ok, reload, sls pid = %d, send SIGUP to it.\n", pid);
    	kill(pid, SIGHUP);
        return SLS_OK;
    }

	//ctrl + c
    if (strcmp(cmd, "stop") == 0) {
    	//
     	printf("sls_send_cmd ok, stop, sls pid = %d, send SIGINT to it.\n", pid);
    	kill(pid, SIGINT);
        return SLS_OK;
    }
    return SLS_OK;
}

#define ADD_VECTOR_END(v,i) (v).push_back((i))

void sls_split_string(std::string str, std::string separator, std::vector<std::string> &result, int count)
{
	result.clear();
	string::size_type position = str.find(separator);
	string::size_type lastPosition = 0;
	uint32_t separatorLength = (uint32_t) separator.length();

	int i = 0;
	while (position != str.npos) {
		ADD_VECTOR_END(result, str.substr(lastPosition, position - lastPosition));
		lastPosition = position + separatorLength;
		position = str.find(separator, lastPosition);
		i ++;
		if (i == count)
			break;
	}
	ADD_VECTOR_END(result, str.substr(lastPosition, string::npos));
}

std::string sls_find_string(std::vector<std::string> &src, std::string &dst)
{
	std::string ret = std::string("");
	std::vector<std::string>::iterator it;
    for(it=src.begin(); it!=src.end();) {
    	std::string str = *it;
    	it ++;
    	string::size_type pos = str.find(dst);
    	if (pos != std::string::npos)
    	{
    		ret = str;
    	    break;
    	}
    }
    return ret;
}

/**
 * parse ts
 */
enum {
    H264_NAL_UNSPECIFIED     = 0,
    H264_NAL_SLICE           = 1,
    H264_NAL_DPA             = 2,
    H264_NAL_DPB             = 3,
    H264_NAL_DPC             = 4,
    H264_NAL_IDR_SLICE       = 5,
    H264_NAL_SEI             = 6,
    H264_NAL_SPS             = 7,
    H264_NAL_PPS             = 8,
    H264_NAL_AUD             = 9,
};

static int64_t ff_parse_pes_pts(const uint8_t  *buf)
{

    int64_t pts = 0;
    int64_t tmp = (int64_t)((buf[0] & 0x0e) << 29) ;
    pts = pts | tmp;
    tmp = (int64_t)((((int64_t)(buf[1]&0xFF)<<8)|(buf[2]) >> 1) << 15);
    pts = pts | tmp;
    tmp = (int64_t)((((int64_t)(buf[3]&0xFF)<<8)|(buf[4])) >> 1);
    pts = pts | tmp;
    return pts;
}

static int sls_parse_spspps(const uint8_t *es, int es_len, ts_info *ti)
{
    int ret = SLS_ERROR;
    int pos = 0;
    uint8_t *p = NULL;
    uint8_t *p_end = NULL;
    uint8_t nal_type = 0;
    while (pos < es_len-4) {
        //avc nal
        bool b_nal = false;
        if (0x0 == es[pos] &&
            0x0 == es[pos+1] &&
            0x0 == es[pos+2] &&
           (0x1 == es[pos+3] || (0x0 == es[pos+3] && 0x1 == es[pos+4]))) {
            if (p != NULL) {
                p_end= (uint8_t *)es + pos;
                if (H264_NAL_SPS == nal_type) {
                    ti->sps_len = p_end-p;
                    memcpy(ti->sps, p, ti->sps_len);
                } else if (H264_NAL_PPS == nal_type) {
                    ti->pps_len = p_end-p;
                    memcpy(ti->pps, p, ti->pps_len);
                } else {
                   printf("parse_spspps, wrong nal type=%d.\n", nal_type);
                }

                if (ti->sps_len > 0 && ti->pps_len > 0) {
                    p = NULL;
                    ret = SLS_OK;
                    break;
                }
            }
            int nal_pos = pos + (es[pos+3]?4:5);
            nal_type = es[nal_pos]&0x1f;
            if (H264_NAL_SPS == nal_type || H264_NAL_PPS == nal_type) {
                p = (uint8_t *)es + pos;
            }
            pos = nal_pos;
        }else{
            pos ++;
        }
    }

    //last nal
    if (p != NULL) {

        p_end= (uint8_t *)es + es_len;
        if (H264_NAL_SPS == nal_type) {
            ti->sps_len = p_end-p;
            memcpy(ti->sps, p, ti->sps_len);
        } else if (H264_NAL_PPS == nal_type) {
            ti->pps_len = p_end-p;
            memcpy(ti->pps, p, ti->pps_len);
        } else {
           printf("parse_spspps, wrong nal type=%d.\n", nal_type);
        }
        if (ti->sps_len > 0 && ti->pps_len > 0) {
            ret = SLS_OK;
        }
    }
    return ret;
}


static int sls_pes2es(const uint8_t *pes_frame, int pes_len, ts_info *ti, int pid)
{
    if (!pes_frame) {
        printf("pes2es: pes_frame is null.\n");
        return SLS_ERROR;
    }
    uint8_t *pes     = (uint8_t *)pes_frame;
    uint8_t *pes_end = (uint8_t *)pes_frame + pes_len;

    if (pes[0] != 0x00 ||
            pes[1] != 0x00 ||
            pes[2] != 0x01) {
        //printf("pes2es: pid=%d, wrong pes header, pes=0x%x-0x%x-0x%x.\n",
        //        ti->es_pid, pes[0], pes[1], pes[2]);
        return SLS_ERROR;
    }
    pes += 3;

    /* it must be an MPEG-2 PES stream */
    int stream_id = (pes[0] & 0xFF);
    if (stream_id != 0xE0 && stream_id != 0xC0) {
        printf("pes2es: pid=%d, wrong pes stream_id=0x%x.", pid, stream_id);
        return SLS_ERROR;
    }
    pes ++;

    int total_size = ((int)(pes[0]<<8)) | pes[1];
    pes += 2;
    /* NOTE: a zero total size means the PES size is
     * unbounded */
    if (0 == total_size)
        total_size = MAX_PES_PAYLOAD;
    int flags = 0;
    /*
    '10'                        :2,
    PES_scrambling_control      :2,
    PES_priority                :1,
    data_alignment_indicator    :1,
    copyright                   :1,
    original_or_copy            :1
     */
    flags = (pes[0] & 0x7F);
    pes ++;

    /*
    PTS_DTS_flags               :2,
    ESCR_flag                   :1,
    ES_rate_flag:1,
    DSM_trick_mode_flag:1,
    additional_copy_info_flag:1,
    PES_CRC_flag:1,
    PES_extension_flag:1,
     */
    flags = (pes[0] & 0xFF);
    pes ++;

    int header_len = (pes[0] & 0xFF);
    pes ++;
    ti->dts = INVALID_DTS_PTS;
    ti->pts = INVALID_DTS_PTS;
    if ((flags & 0xc0) == 0x80) {
        ti->dts = ti->pts = ff_parse_pes_pts(pes);
        pes += 5;
    } else if ((flags & 0xc0) == 0xc0) {
        ti->pts = ff_parse_pes_pts(pes);
        pes += 5;
        ti->dts = ff_parse_pes_pts(pes);
        pes += 5;
    }

    int ret = SLS_OK;
    //parse sps and pps
    if (ti->need_spspps){
        ret = sls_parse_spspps(pes, pes_end - pes, ti);
        if (ti->sps_len > 0 && ti->pps_len > 0 && ti->pat_len > 0 && ti->pat_len > 0) {
            uint8_t * p = ti->ts_data ;
            int pos = 0;
            uint8_t tmp;

            //pat, pmt
            memcpy(p+pos, ti->pat, TS_PACK_LEN);
            pos += TS_PACK_LEN;
            memcpy(p+pos, ti->pmt, TS_PACK_LEN);
            pos += TS_PACK_LEN;

            //sps pps
            int len = ti->sps_len + ti->pps_len;
            len = len + 9 + 5;//pes len
            if (len > TS_PACK_LEN-4) {
                printf("pid=%d, pes size=%d is abnormal!!!!\n", pid, len);
                return ret;
            }
            pos ++;
            //pid
            ti->es_pid = pid;
            tmp = ti->es_pid >> 8;
            p[pos++] = 0x40 | tmp;
            tmp = ti->es_pid;
            p[pos++] = tmp;
            p[pos] = 0x10;
            int ad_len = TS_PACK_LEN - 4 - len - 1;
            if (ad_len > 0) {
                p[pos++] = 0x30;
                p[pos++] = ad_len;//adaptation length
                p[pos++] = 0x00;//
                memset(p + pos, 0xFF, ad_len-1);
                pos += ad_len - 1;
            }else{
                pos ++;
            }

            //pes
            p[pos++] = 0;
            p[pos++] = 0;
            p[pos++] = 1;
            p[pos++] = stream_id;
            p[pos++] = 0;//total size
            p[pos++] = 0;//total size
            p[pos++] = 0x80;//flag
            p[pos++] = 0x80;//flag
            p[pos++] = 5;//header_len
            p[pos++] = 0;//pts
            p[pos++] = 0;
            p[pos++] = 0;
            p[pos++] = 0;
            p[pos++] = 0;
            memcpy(p+pos, ti->sps, ti->sps_len);
            pos += ti->sps_len;
            memcpy(p+pos, ti->pps, ti->pps_len);
            pos += ti->pps_len;
        }
    }
    return ret;
}

static int sls_parse_pat(const uint8_t *pat_data, int len, ts_info *ti)
{
    uint8_t *buffer = (uint8_t *)pat_data;
    int table_id                    = buffer[0];
    int section_syntax_indicator    = buffer[1] >> 7;
    int zero                        = buffer[1] >> 6 & 0x1;
    int reserved_1                  = buffer[1] >> 4 & 0x3;
    int section_length              = (buffer[1] & 0x0F) << 8 | buffer[2];
    int transport_stream_id         = buffer[3] << 8 | buffer[4];
    int reserved_2                  = buffer[5] >> 6;
    int version_number              = buffer[5] >> 1 &  0x1F;
    int current_next_indicator      = (buffer[5] << 7) >> 7;
    int section_number              = buffer[6];
    int last_section_number         = buffer[7];

    int CRC_32                      = (buffer[len-4] & 0x000000FF) << 24
       | (buffer[len-3] & 0x000000FF) << 16
       | (buffer[len-2] & 0x000000FF) << 8
       | (buffer[len-1] & 0x000000FF);

    int n = 0;
    for ( n = 0; n < section_length - 12; n += 4 )
    {
        unsigned  program_num = buffer[8 + n ] << 8 | buffer[9 + n ];
        int reserved_3           = buffer[10 + n ] >> 5;
        int network_PID = 0x00;
        if ( program_num == 0x00)
        {
             network_PID = (buffer[10 + n ] & 0x1F) << 8 | buffer[11 + n ];
        }else{
            ti->pmt_pid = (buffer[10 + n] & 0x1F) << 8 | buffer[11 + n];
        }
     }
    return SLS_OK;

}

int sls_parse_ts_info(const uint8_t *packet, ts_info *ti){


    if (packet[0] != TS_SYNC_BYTE) {
        printf( "ts2es: packet[0]=0x%x not 0x47.\n", packet[0]);
        return SLS_ERROR;
    }

    int is_start = packet[1] & 0x40;
    if (0 == is_start) {
        // no start indicatore
        return SLS_ERROR;
    }

    int pid = (int)((packet[1] & 0x1F) << 8) | (packet[2]&0xFF);
    if (PAT_PID == pid) {
        //save pat table
        memcpy(ti->pat, packet, TS_PACK_LEN);
        ti->pat_len = TS_PACK_LEN;
    }else {
        if (ti->pmt_pid == pid) {
            memcpy(ti->pmt, packet, TS_PACK_LEN);
            ti->pmt_len = TS_PACK_LEN;
            return SLS_OK;
        }
        if (INVALID_PID != ti->es_pid) {
            if (pid != ti->es_pid) {
                //not available pid
                return SLS_ERROR;
            }
        }
    }

    //start to parse the dts
    int afc = (packet[3] >> 4) & 3;
    if (afc == 0) /* reserved value */
        return SLS_ERROR;
    int has_adaptation   = afc & 2;
    int has_payload      = afc & 1;
    bool is_discontinuity = (has_adaptation == 1) &&
            (packet[4] != 0) && /* with length > 0 */
            ((packet[5] & 0x80) != 0); /* and discontinuity indicated */


    if ((packet[1] & 0x80) != 0) {
        //Log.i(TAG, "SrsTSToES, Packet had TEI flag set; marking as corrupt ");
    }

    int pos = 4;
    int p = (packet[pos] & 0xFF);
    if (has_adaptation != 0) {
        int64_t pcr_h;
        int pcr_l;
        //if (parse_pcr(&pcr_h, &pcr_l, packet) == 0)
        //ts->last_pcr = pcr_h * 300 + pcr_l;
        /* skip adaptation field */
        pos += p + 1;
        //printf("ts2es: adaptation, pos=%d.", pos);
    }
    /* if past the end of packet, ignore */
    if (pos >= TS_PACK_LEN || 1 != has_payload) {
        printf("ts2es: pid=%d, payload len=%d, >188.\n", pid, pos);
        return SLS_ERROR;
    }

    if (pid == PAT_PID) {
        if (is_start)
            pos ++;
        return sls_parse_pat(packet+pos, TS_PACK_LEN - pos, ti);
    }

    int ret = sls_pes2es(packet+pos, TS_PACK_LEN - pos, ti, pid);
    if (ti->dts != INVALID_DTS_PTS) {
        ti->es_pid = pid;
    }
    if (ti->sps_len > 0 && ti->pps_len > 0) {
        ti->es_pid = pid;
    }
    return ret;
}

void sls_init_ts_info(ts_info *ti)
{
    if (NULL != ti) {
        ti->es_pid  = INVALID_PID;
        ti->dts     = INVALID_DTS_PTS;
        ti->pts     = INVALID_DTS_PTS;
        ti->sps_len = 0;
        ti->pps_len = 0;
        ti->pat_len = 0;
        ti->pmt_len = 0;
        ti->pmt_pid = INVALID_PID;
        ti->need_spspps    = false;

        memset(ti->ts_data, 0, TS_UDP_LEN);

        for (int i = 0; i < TS_UDP_LEN; ) {
            ti->ts_data[i] = 0x47;
            ti->ts_data[i+1] = 0x1F;
            ti->ts_data[i+2] = 0xFF;
            ti->ts_data[i+3] = 0x00;
            i+=TS_PACK_LEN;
        }
    }
}

