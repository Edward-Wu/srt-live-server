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

    m_invalid_begin_tm     = 0;
    m_idle_streams_timeout = 0; //unit :s

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
	if (SLS_RS_INITED == m_state){
		if (m_idle_streams_timeout > 0) {
	        sls_log(SLS_LOG_INFO, "[%p]CSLSRole::invalid_srt, m_idle_streams_timeout=%d, m_state=%d, set state idle stream.",
	        		this, m_idle_streams_timeout, m_state);
            m_state = SLS_RS_IDLE_STREAM;
            m_invalid_begin_tm = sls_gettime_relative();
            return 0;
		}
	}
	if (SLS_RS_IDLE_STREAM == m_state){
        return 0;
	}

    if (m_srt) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSRole::invalid_srt, close sock=%d, m_state=%d.", this, get_fd(), m_state);
        m_srt->libsrt_close();
        delete m_srt;
        m_srt = NULL;
    }
    return 0;
}

int CSLSRole::get_state(int64_t cur_time_microsec)
{
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

	if (SLS_RS_INITED == m_state || SLS_RS_INVALID == m_state )
        return m_state;

    if (SLS_RS_IDLE_STREAM == m_state) {
    	if (0 == cur_time_microsec)
    		cur_time_microsec = sls_gettime_relative();
        int d = cur_time_microsec - m_invalid_begin_tm;//sls_gettime_relative() - m_invalid_begin_tm;
        if (d >= m_idle_streams_timeout * 1000000) {
            sls_log(SLS_LOG_INFO, "[%p]CSLSRole::get_state, m_state=%d, d=%d, m_idle_streams_timeout=%d, call invalid_srt.",
            		this, m_state, d/1000000, m_idle_streams_timeout);
            m_state = SLS_RS_INVALID;
            invalid_srt();
        }
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

int CSLSRole::handler_read_data()
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
    if (n != TS_UDP_LEN) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSRole::handler_read_data, libsrt_read n=%d, expect %d.", this, n, TS_UDP_LEN);
        return SLS_ERROR;
    }

    if (NULL == m_map_data) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSRole::handler_read_data, no data handled, m_map_data is NULL.", this);
        return SLS_ERROR;
    }

    int ret = m_map_data->put(m_map_data_key, szData, n);
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
            return SLS_ERROR;
        }
        m_data_pos = 0;
        m_data_len = ret;
    }
    if (SLS_RS_IDLE_STREAM == m_state) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSRole::handler_write_data, change m_state form SLS_RS_IDLE_STREAM to SLS_RS_INITED.",
                this);
    	m_state = SLS_RS_INITED;
    }

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
