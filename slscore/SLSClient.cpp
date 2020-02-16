
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
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <srt/srt.h>

#include "SLSClient.hpp"
#include "SLSLog.hpp"

/**
 * CSLSClient class implementation
 */

#define POLLING_TIME 1 /// Time in milliseconds between interrupt check


CSLSClient::CSLSClient()
{
    memset(m_url, 0, 1024);
    memset(m_ts_file_name, 0, 1024);
    memset(m_out_file_name, 0, 1024);

    m_eid        = 0;
    m_out_file   = 0;
    m_data_count = 0;
    m_bit_rate   = 0;

    sprintf(m_role_name, "client");
}

CSLSClient::~CSLSClient()
{
}

int CSLSClient::init_epoll()
{
    int ret = 0;

    m_eid = CSLSSrt::libsrt_epoll_create();
    if (m_eid < 0) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSClient::work, srt_epoll_create failed.", this);
        return CSLSSrt::libsrt_neterrno();
    }
    //compatible with srt v1.4.0 when container is empty.
    srt_epoll_set(m_eid, SRT_EPOLL_ENABLE_EMPTY);
    return ret;
}

int CSLSClient::uninit_epoll()
{
    int ret = 0;
    if (m_eid >= 0) {
        CSLSSrt::libsrt_epoll_release(m_eid);
        sls_log(SLS_LOG_INFO, "[%p]CSLSEpollThread::work, srt_epoll_release ok.", this);
    }
    return ret;
}


int CSLSClient::play(const char* url, const char *out_file_name)
{
    m_is_write = false;
    if (strlen(out_file_name) > 0) {
    	strcpy(m_out_file_name, out_file_name);
    }
	return open_url(url);
}

int CSLSClient::open_url(const char* url)
{
    //
    if (strlen(url) == 0) {
    	sls_log(SLS_LOG_INFO, "[%p]CSLSClient::play, url='%s', must like 'srt://hostname:port?streamid=your_stream_id' or 'srt://hostname:port/app/stream_name'.",
    			this, url);
        return SLS_ERROR;
    }

    int ret = open(url);
    if (SLS_OK != ret) {
    	return ret;
    }

    //add to epoll
    ret = init_epoll();
    if (SLS_OK != ret) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSClient::play, init_epoll failed.", this);
        return CSLSSrt::libsrt_neterrno();
    }
    ret = add_to_epoll(m_eid);
    if (SLS_OK != ret) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSClient::play, add_to_epoll failed.", this);
        return CSLSSrt::libsrt_neterrno();
    }
    return ret;
}

int CSLSClient::push(const char* url, const char *ts_file_name)
{
    return SLS_OK;
}

int CSLSClient::close()
{

	int ret = SLS_OK;
	if (0 != m_out_file) {
		::close(m_out_file);
		m_out_file = 0;
	}
	if (0 != m_eid) {
    	sls_log(SLS_LOG_INFO, "[%p]CSLSClient::close, ok, url='%s'.", this, m_url);
		remove_from_epoll();
		uninit_epoll();
		m_eid = 0;
	}
    return CSLSRelay::close();
}

int CSLSClient::handler()
{
	char szData[TS_UDP_LEN];
    SRTSOCKET  read_socks[1];
    SRTSOCKET  write_socks[1];
    int read_len   = 0;
    int write_len  = 0;

	if (m_is_write) {
		//push
		return SLS_OK;
	} else {
		//play
		if (NULL == m_srt) {
	        sls_log(SLS_LOG_ERROR, "[%p]CSLSClient::handler, failed, m_srt is null.", this);
		    return SLS_ERROR;
		}
		if (0 == m_eid) {
	        sls_log(SLS_LOG_ERROR, "[%p]CSLSClient::handler, failed, m_eid = 0.", this);
		    return SLS_ERROR;
		}
	    read_len = 1;
		//check epoll
	    int ret = srt_epoll_wait(m_eid, read_socks, &read_len, write_socks, &write_len, POLLING_TIME, 0, 0, 0, 0);
	    if (0 > ret) {
	    	return SLS_OK;
	    }
	    if (0 >= read_socks[0]) {
	    	return SLS_OK;
	    }

	    //read data
	    int n = m_srt->libsrt_read(szData, TS_UDP_LEN);
		if (n <= 0) {
	        sls_log(SLS_LOG_ERROR, "[%p]CSLSClient::handler, libsrt_read failure, n=%d.", this, n, TS_UDP_LEN);
		    return SLS_OK;
		}

		//update invalid begin time
		//m_invalid_begin_tm = sls_gettime();

        if (0 == m_out_file) {
			if (strlen(m_out_file_name) > 0) {
				m_out_file = ::open(m_out_file_name, O_WRONLY|O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IXOTH);
			    if (0 == m_out_file) {
			    	sls_log(SLS_LOG_ERROR, "[%p]CSLSClient::handler, open file='%s' failed, '%s'.\n", m_out_file_name, strerror(errno));
			    	return SLS_ERROR;
			    }
			}
		}
        if (0 != m_out_file) {
            ::write(m_out_file, szData, TS_UDP_LEN);
        }
        m_data_count += n;
        int64_t cur_tm = sls_gettime_ms();
        int d = (cur_tm - m_invalid_begin_tm)/1000;
        if (d >= 500) {
            m_bit_rate = m_data_count*8/d;
            m_data_count = 0;
            m_invalid_begin_tm = sls_gettime_ms();
        }

		if (n != TS_UDP_LEN) {
	        sls_log(SLS_LOG_INFO, "[%p]CSLSClient::handler, libsrt_read n=%d, expect %d.", this, n, TS_UDP_LEN);
	    }
		return n;
	}
}

int64_t CSLSClient::get_bitrate()
{
	return m_bit_rate;
}



