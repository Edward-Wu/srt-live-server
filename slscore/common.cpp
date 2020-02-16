
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

static int sls_mkdir_p(const char *path)
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


static char pid_path_name[] = "~/sls";
static char pid_file_name[] = "~/sls/pid.txt";
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
