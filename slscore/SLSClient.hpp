
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


#ifndef _SLSClient_INCLUDE_
#define _SLSClient_INCLUDE_

#include <list>

#include "SLSRelay.hpp"


/**
 * CSLSClient
 */
class CSLSClient: public CSLSRelay
{
public :
	CSLSClient();
    virtual ~CSLSClient();

    int play(const char *url, const char *out_file_name);
    int push(const char *url, const char *ts_file_name);

    virtual int close();
    virtual int handler();

    int64_t get_bitrate();

protected:
    int init_epoll();
    int uninit_epoll();

    int open_url(const char* url);

    char  m_url[1024];
    char  m_ts_file_name[1024];
    char  m_out_file_name[1024];

    int   m_eid;
    int   m_out_file;

    int64_t m_data_count;
    int64_t m_bit_rate;//kbit/s

};


#endif
