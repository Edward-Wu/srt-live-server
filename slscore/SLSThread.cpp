
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


#include "SLSThread.hpp"
#include "SLSLog.hpp"

/**
 * CSLSThread class implementation
 */

CSLSThread::CSLSThread()
{
	m_exit     = 0;
	m_th_id     = 0;
}
CSLSThread::~CSLSThread()
{
	stop();
}

int CSLSThread::start()
{
	int ret = 0;
    int err;
    pthread_t th_id;

    err = pthread_create(&th_id, NULL, thread_func, (void *)this);
    if (err != 0) {
    	sls_log(SLS_LOG_ERROR, "[%p]CSLSThread::start, can't create thread, error: %s\n", this, strerror(err));
        return -1;
    }
    m_th_id = th_id;
    sls_log(SLS_LOG_INFO, "[%p]CSLSThread::start, pthread_create ok, m_th_id=%lld.", this, m_th_id);

	return ret;

}
int CSLSThread::stop()
{
	int ret = 0;
    if (0 == m_th_id) {
        return ret;
    }
    sls_log(SLS_LOG_INFO, "[%p]CSLSThread::stop, m_th_id=%lld.", this, m_th_id);

    m_exit = 1;
	pthread_join(m_th_id, NULL);
	m_th_id = 0;
    clear();

	return ret;

}

void CSLSThread::clear()
{

}

bool  CSLSThread::is_exit()
{
	return m_exit == 1;
}

void * CSLSThread::thread_func(void * arg)
{
	CSLSThread *pThis = (CSLSThread *)arg;
	if (!pThis)
	{
    	sls_log(SLS_LOG_ERROR, "CSLSThread::thread_func, thread arg is null.\n");
	}

	pThis->work();
	return NULL;
}


int CSLSThread::work()
{
	int ret = 0;

	return ret;

}
