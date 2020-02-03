
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


#ifndef _SLSEpollThread_INCLUDE_
#define _SLSEpollThread_INCLUDE_

#include <srt/srt.h>
#include "SLSThread.hpp"

#define MAX_SOCK_COUNT 1024

/**
 * CSLSEpollThread , the base thread class
 */
class CSLSEpollThread: public CSLSThread
{
public :
	CSLSEpollThread();
    ~CSLSEpollThread();

    virtual int     work();

    int init_epoll();
    int uninit_epoll();

protected:
    virtual int     handler();

    int         add_to_epoll(int fd, bool write) ;

    int        m_eid;
    SRTSOCKET  m_read_socks[MAX_SOCK_COUNT];
    SRTSOCKET  m_write_socks[MAX_SOCK_COUNT];

};


#endif
