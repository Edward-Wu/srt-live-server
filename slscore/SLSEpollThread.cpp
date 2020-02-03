
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
    sls_log(SLS_LOG_INFO, "[%p]CSLSEpollThread::work, begin th_id=%lld.", this, m_th_id);
	//epoll loop
	while (!m_exit) {
	    handler();
	}

	clear();
    sls_log(SLS_LOG_INFO, "[%p]CSLSEpollThread::work, end th_id=%lld.", this, m_th_id);
	return ret;
}

int CSLSEpollThread::handler()
{
	int ret = 0;

	return ret;
}

