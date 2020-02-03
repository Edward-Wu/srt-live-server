
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


#ifndef _SLSLOG_INCLUDE_
#define _SLSLOG_INCLUDE_

#include <cstdarg>
#include <stdio.h>

#include "common.hpp"
#include "SLSLock.hpp"

#define SLS_LOG_FATAL     0
#define SLS_LOG_ERROR     1
#define SLS_LOG_WARNING   2
#define SLS_LOG_INFO      3
#define SLS_LOG_DEBUG     4
#define SLS_LOG_TRACE     5
static char const * LOG_LEVEL_NAME[] = {
		"FATAL",
		"ERROR",
		"WARNING",
		"INFO",
		"DEBUG",
		"TRACE"
};


static const char APP_NAME[] = "SLS";

#define sls_log CSLSLog::log
#define sls_set_log_level CSLSLog::set_log_level
#define sls_set_log_file CSLSLog::set_log_file
/**
 * CSLSLog
 */
class CSLSLog
{
private :
    CSLSLog();
    ~CSLSLog();

public :
    static int  create_instance();
    static int  destory_instance();
    static void log(int level, const char *fmt, ...);
    static void set_log_level(char *level);
    static void set_log_file(char * file_name);

private:
    static CSLSLog* m_pInstance;
    int             m_level;
    CSLSMutex       m_mutex;
    char            log_filename[1024];
    FILE          * m_log_file;

    void print_log(int level, const char *fmt, va_list vl);
};



#endif
