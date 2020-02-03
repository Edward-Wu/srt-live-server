
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


#include "SLSRole.hpp"
#include "SLSLog.hpp"


/**
 * CSLSRole class implementation
 */

CSLSRole::CSLSRole()
{
    m_latency          = 10; //default 10ms
	m_srt              = NULL;
	m_state            = SLS_RS_UNINIT;
	m_back_log         = 1024;
	m_is_write         = 1;

    m_conf             = NULL;
    m_map_data         = NULL;        // role:data'

    m_invalid_begin_tm     = sls_gettime_relative();
    m_idle_streams_timeout = 10; //unit :s

    m_data_len = 0;
    m_data_pos = 0;

    m_need_reconnect = false;

    memset(m_map_data_key, 0, 1024);

	sprintf(m_role_name, "role");
}

CSLSRole::~CSLSRole()
{
    uninit();
}


int CSLSRole::init()
{
	int ret = 0;
    m_state      = SLS_RS_INITED;

    m_map_data_id.bFirst     = true;
    m_map_data_id.nDataCount = 0;
    m_map_data_id.nReadPos   = 0;

	return ret;
}

int CSLSRole::uninit()
{
	int ret = 0;
	if (SLS_RS_UNINIT != m_state)
	{
        m_state = SLS_RS_UNINIT;
	    remove_from_epoll();
	    invalid_srt();
	}
	return ret;
}

int CSLSRole::invalid_srt()
{
    if (m_srt) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSRole::invalid_srt, close sock=%d, m_state=%d.", this, get_fd(), m_state);
        m_srt->libsrt_close();
        delete m_srt;
        m_srt = NULL;
    }
    return SLS_OK;
}

int CSLSRole::get_state(int64_t cur_time_microsec)
{
	if (SLS_RS_INVALID == m_state)
		return m_state;

	if (check_idle_streams_duration(cur_time_microsec)) {
		sls_log(SLS_LOG_INFO, "[%p]CSLSRole::get_state, check_idle_streams_duration is true, cur m_state=%d, m_idle_streams_timeout=%ds, call invalid_srt.",
				this, m_state, m_idle_streams_timeout);
		m_state = SLS_RS_INVALID;
		invalid_srt();
        return m_state;
    }

	int ret = get_sock_state();
	if (SLS_ERROR == ret || SRTS_BROKEN == ret || SRTS_CLOSED == ret || SRTS_NONEXIST == ret) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSRole::get_state, get_sock_state, ret=%d, call invalid_srt.",
        		this, ret);
        if (SRTS_BROKEN == ret || SRTS_CLOSED == ret || SRTS_NONEXIST == ret) {
            CSLSSrt::libsrt_neterrno();
        }
		m_state = SLS_RS_INVALID;
		invalid_srt();
		return m_state;
	}
	return m_state;
}

int CSLSRole::handler()
{
	int ret = 0;
    //sls_log(SLS_LOG_INFO, "CSLSRole::handler()");
	return ret;
}

int CSLSRole::get_fd()
{
    if (m_srt)
        return m_srt->libsrt_get_fd();
    return 0;
}

int CSLSRole::set_eid(int eid)
{
    if (m_srt)
        return m_srt->libsrt_set_eid(eid);
    return 0;
}

int CSLSRole::set_srt(CSLSSrt *srt)
{
	if (m_srt) {
	    sls_log(SLS_LOG_ERROR, "[%p]CSLSRole::setSrt, m_srt=%p is not null.", this, m_srt);
		return SLS_ERROR;
	}
	m_srt = srt;
	return 0;
}


int CSLSRole::write(const char * buf, int size)
{
    if (m_srt)
        return m_srt->libsrt_write(buf, size);
    return SLS_ERROR;
}

int CSLSRole::add_to_epoll(int eid)
{
    int ret = SLS_ERROR;
    if (m_srt) {
        m_srt->libsrt_set_eid(eid);
        ret = m_srt->libsrt_add_to_epoll(eid, m_is_write);
        sls_log(SLS_LOG_INFO, "[%p]CSLSRole::add_to_epoll, %s, sock=%d, m_is_write=%d, ret=%d.",
        		this, m_role_name, get_fd(), m_is_write, ret);
    }
    return ret;
}

int CSLSRole::remove_from_epoll()
{
    int ret = SLS_ERROR;
    if (m_srt) {
        ret = m_srt->libsrt_remove_from_epoll();
        sls_log(SLS_LOG_INFO, "[%p]CSLSRole::remove_from_epoll, %s, sock=%d, ret=%d.",
                this, m_role_name, get_fd(), ret);
    }
    return ret;
}

int CSLSRole::get_sock_state()
{
    if (m_srt)
        return m_srt->libsrt_getsockstate();
    return SLS_ERROR;
}

char * CSLSRole::get_role_name()
{
    return m_role_name;
}

char * CSLSRole::get_streamid()
{
	char sid[1024] = {0};
	int  sid_size = sizeof(sid);
    if (m_srt) {
    	m_srt->libsrt_getsockopt(SRTO_STREAMID, "SRTO_STREAMID", sid, &sid_size);
    }
    return sid;
}

bool CSLSRole::is_reconnect()
{
	return m_need_reconnect;
}

void CSLSRole::set_conf(sls_conf_base_t * conf)
{
    m_conf = conf;
}

void CSLSRole::set_map_data(char *map_key, CSLSMapData *map_data)
{
	if (NULL != map_key) {
        strcpy(m_map_data_key, map_key);
        m_map_data     = map_data;
	} else {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSRole::set_map_data, failed, map_key is null.", this);
	}
}

void CSLSRole::set_idle_streams_timeout(int timeout)
{
	m_idle_streams_timeout = timeout;
}

bool CSLSRole::check_idle_streams_duration(int64_t cur_time_microsec)
{
	if (-1 == m_idle_streams_timeout) {
		return false;
	}
	if (0 == cur_time_microsec ) {
		cur_time_microsec = sls_gettime_relative();
	}
	int duration = (cur_time_microsec - m_invalid_begin_tm)/1000000;
    if (duration >= m_idle_streams_timeout) {
    	return true;
    }
	return false;
}

int CSLSRole::handler_read_data(int64_t *last_read_time)
{
	char szData[TS_UDP_LEN];

	if (NULL == m_srt) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSRole::handler_read_data, m_srt is null.", this);
	    return SLS_ERROR;
	}
    //read data
    int n = m_srt->libsrt_read(szData, TS_UDP_LEN);
	if (n <= 0) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSRole::handler_read_data, libsrt_read failure, n=%d.", this, n, TS_UDP_LEN);
	    return SLS_ERROR;
	}

	//update invalid begin time
	m_invalid_begin_tm = sls_gettime_relative();

	if (n != TS_UDP_LEN) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSRole::handler_read_data, libsrt_read n=%d, expect %d.", this, n, TS_UDP_LEN);
        return SLS_ERROR;
    }

    if (NULL == m_map_data) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSRole::handler_read_data, no data handled, m_map_data is NULL.", this);
        return SLS_ERROR;
    }

    sls_log(SLS_LOG_TRACE, "[%p]CSLSRole::handler_read_data, ok, libsrt_read n=%d.", this, n);
    int ret = m_map_data->put(m_map_data_key, szData, n, last_read_time);
	return ret;
}

int CSLSRole::handler_write_data()
{
	int ret = 0;
	int write_size = 0;

    //read data from publisher's data array
    if (NULL == m_map_data) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSRole::handler_write_data, no data, m_map_data is NULL.",
                this);
        return SLS_ERROR ;
    }
    if (strlen(m_map_data_key) == 0) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSRole::handler_write_data, no data, m_map_data_key is ''.",
                this);
        return SLS_ERROR ;
    }

    if (0 == m_data_len) {
        ret = m_map_data->get(m_map_data_key, m_data, DATA_BUFF_SIZE, &m_map_data_id);
        if (ret < 0) {
        	//maybe no publisher, wait for timeout.
            return SLS_OK;
        }
        m_data_pos = 0;
        m_data_len = ret;
    }

    //update invalid begin time
    m_invalid_begin_tm = sls_gettime_relative();

    int len = m_data_len - m_data_pos;
    while (m_data_pos < m_data_len) {
        ret = write(m_data + m_data_pos, TS_UDP_LEN);
        if (ret < TS_UDP_LEN) {
            sls_log(SLS_LOG_INFO, "[%p]CSLSRole::handler_write_data, write data failed, ret=%d, not %d.", this, len, ret, TS_UDP_LEN);
            break;
        }
        m_data_pos += TS_UDP_LEN;
        write_size += TS_UDP_LEN;
    }

    if (m_data_pos < m_data_len) {
        sls_log(SLS_LOG_TRACE, "[%p]CSLSRole::handler_write_data, write data, len=%d, remainder=%d.", this, len, m_data_len - m_data_pos);
        return SLS_OK;
    }
    if (m_data_pos > m_data_len) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSRole::handler_write_data, write data, data error, m_data_pos=%d > m_data_len=%d.", this, len, m_data_pos, m_data_len);
    } else {
        //sls_log(SLS_LOG_TRACE, "[%p]CSLSRole::handler_write_data, write data, m_data_len=%d.", this, m_data_len);
    }
    m_data_pos = m_data_len = 0;

    return write_size ;
}
