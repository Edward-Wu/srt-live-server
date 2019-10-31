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

#include <errno.h>
#include <string.h>


#include "SLSEpollThread.hpp"
#include "SLSLog.hpp"
#include "common.hpp"
#include "SLSRole.hpp"

#include <srt/srt.h>

/**
 * CSLSThread class implementation
 */

CSLSEpollThread::CSLSEpollThread()
{
}

CSLSEpollThread::~CSLSEpollThread()
{

}

int CSLSEpollThread::init_epoll()
{
    int ret = 0;

    m_eid = CSLSSrt::libsrt_epoll_create();
    if (m_eid < 0) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSEpollThread::work, srt_epoll_create failed. th_id=%lld.", this, m_th_id);
        return CSLSSrt::libsrt_neterrno();
    }
    //compatible with srt v1.4.0 when container is empty.
    srt_epoll_set(m_eid, SRT_EPOLL_ENABLE_EMPTY);
    return ret;
}

int CSLSEpollThread::uninit_epoll()
{
    int ret = 0;
    if (m_eid >= 0) {
        CSLSSrt::libsrt_epoll_release(m_eid);
        sls_log(SLS_LOG_INFO, "[%p]CSLSEpollThread::work, srt_epoll_release ok, m_th_id=%lld.", this, m_th_id);
    }
    return ret;
}

int CSLSEpollThread::work()
{
    int ret = 0;
    sls_log(SLS_LOG_INFO, "[%p]CSLSEpollThread::work, th_id=%lld.", this, m_th_id);
	//epoll loop
	while (!m_exit) {
	    handler();
	}

	clear();
	return ret;
}

int CSLSEpollThread::handler()
{
	int ret = 0;

	return ret;
}

