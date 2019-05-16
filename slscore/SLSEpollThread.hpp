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
