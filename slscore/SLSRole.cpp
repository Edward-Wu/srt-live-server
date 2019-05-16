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
    m_exit_delay = 0;
    m_latency    = 10; //default 20ms
	m_srt              = NULL;
	m_state            = SLS_RS_UNINIT;
	sprintf(m_role_name, "role");
}
CSLSRole::~CSLSRole()
{
    uninit();
}


int CSLSRole::init()
{
	int ret = 0;
    m_invalid_tm = 0;
    m_state      = SLS_RS_INITED;
	return ret;
}

int CSLSRole::uninit()
{
	int ret = 0;
	if (SLS_RS_INITED == m_state)
	{
	    remove_from_epoll();
	    invalid_srt();
        if (0 == m_exit_delay) {
            m_state = SLS_RS_INVALID;
        } else {
            m_state = SLS_RS_UNINIT;
            m_invalid_tm = sls_gettime_relative();
        }
	}
	return ret;
}

int CSLSRole::get_state()
{
    if (SLS_RS_INITED == m_state || SLS_RS_INVALID == m_state )
        return m_state;

    int d = sls_gettime_relative() - m_invalid_tm;
    if (d >= m_exit_delay*1000000)
        m_state = SLS_RS_INVALID;
    //sls_log(SLS_LOG_INFO, "[%p]CSLSRole::get_state, m_state=%d, d=%d, duration=%d.", this, m_state, d/1000000, m_exit_delay);
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

int CSLSRole::invalid_srt()
{
    if (m_srt) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSRole::invalid_srt, close sock=%d.", this, get_fd());
        m_srt->libsrt_close();
        delete m_srt;
        m_srt = NULL;
    }
    return 0;
}

int CSLSRole::write(const char * buf, int size)
{
    if (m_srt)
        return m_srt->libsrt_write(buf, size);
    return SLS_ERROR;
}

int CSLSRole::push_data(const char * buf, int size)
{
    return m_list_data.push((char *)buf, size);
}

int CSLSRole::add_to_epoll(int eid)
{
    int ret = SLS_ERROR;
    if (m_srt) {
        m_srt->libsrt_set_eid(eid);
        ret = m_srt->libsrt_add_to_epoll(eid, m_is_write);
        sls_log(SLS_LOG_INFO, "[%p]CSLSRole::add_to_epoll, %s, sock=%d, ret=%d.", this, m_role_name, get_fd(), ret);
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

/*
int CSLSRole::invalid()
{
    int ret = remove_from_epoll();
    uninit();
    return ret;
}
*/

void CSLSRole::set_parent(void * parent) {
    m_parent = parent;
}

void  * CSLSRole::get_parent()
{
    return m_parent;
}

void CSLSRole::clear(bool invalid, bool del)
{

}
void CSLSRole::set_conf(sls_conf_base_t * conf)
{
    m_conf = conf;
}




