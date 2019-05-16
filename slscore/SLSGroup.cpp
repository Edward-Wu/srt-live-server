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


#include "SLSGroup.hpp"
#include "SLSLog.hpp"

#define POLLING_TIME 100 /// Time in milliseconds between interrupt check

/**
 * CSLSGroup class implementation
 */

CSLSGroup::CSLSGroup()
{
    m_list_role = NULL;
    m_worker_connections = 100;//for test
    m_worker_number      = 0;
}

CSLSGroup::~CSLSGroup()
{
}

int CSLSGroup::start()
{
    sls_log(SLS_LOG_INFO, "[%p]CSLSGroup::start, worker_number=%d.", this, m_worker_number);
	//do something here

    return CSLSEpollThread::start();

}

int CSLSGroup::stop()
{
	int ret = 0;
    sls_log(SLS_LOG_INFO, "[%p]CSLSGroup::stop, worker_number=%d.", this,m_worker_number);
	ret = CSLSEpollThread::stop();

	return ret;
}

void CSLSGroup::check_new_role() {

    // first, check rolelist
    if (NULL == m_list_role)
        return ;
    if (m_map_role.size() >= m_worker_connections)
        return;

    CSLSRole * role = m_list_role->pop();
    if (NULL == role)
        return ;

    int fd = role->get_fd();
    if (fd == 0) {
        //invalid role
        delete role;
        return ;
    }

    //add to epoll
    if (0 == role->add_to_epoll(m_eid)) {
        m_map_role[fd] = role;
        sls_log(SLS_LOG_INFO, "[%p]CSLSGroup::check_new_role, worker_number=%d, %s=%p, add_to_epoll fd=%d, role_map.size=%d.",
                this, m_worker_number, role->get_role_name(), role, fd, m_map_role.size());
    } else {
        sls_log(SLS_LOG_INFO, "[%p]CSLSGroup::check_new_role, worker_number=%d, %s=%p, add_to_epoll failed, fd=%d.",
                this, m_worker_number, role->get_role_name(), role, fd);
        delete role;
    }
}

int CSLSGroup::handler()
{
	int ret = 0;
	int i;
    int read_len  = MAX_SOCK_COUNT;
    int write_len = MAX_SOCK_COUNT;

    int handler_count = 0;

    //check epoll event
    ret = srt_epoll_wait(m_eid, m_read_socks, &read_len, m_write_socks, &write_len, POLLING_TIME, 0, 0, 0, 0);
    if (ret < 0) {
        sls_log(SLS_LOG_TRACE, "[%p]CSLSGroup::handle, worker_number=%d, srt_epoll_wait, no epoll event, ret=%d.",
                this, m_worker_number, ret);
    	ret = srt_getlasterror(NULL);
        if (ret == SRT_ETIMEOUT)//6003
            ret = SLSERROR(EAGAIN);
        else
            ret = CSLSSrt::libsrt_neterrno();

        sls_log(SLS_LOG_TRACE, "[%p]CSLSGroup::handle, worker_number=%d, check_invalid_sock, no epoll event.", this, m_worker_number);
        check_invalid_sock();
        sls_log(SLS_LOG_TRACE, "[%p]CSLSGroup::handle, worker_number=%d, check_new_role, no epoll event.", this, m_worker_number);
        check_new_role();
        // no event sleep 1 ms.
        msleep(1);
        return ret;
    }

    //handle writable sock
    if (write_len > 0) {
        sls_log(SLS_LOG_TRACE, "[%p]CSLSGroup::handle, worker_number=%d, writable sock count=%d.", this, write_len);
    }
    for (i = 0; i < write_len; i ++) {
        std::map<int, CSLSRole *>::iterator it = m_map_role.find(m_write_socks[i]);
        if (it == m_map_role.end()) {
            sls_log(SLS_LOG_WARNING, "[%p]CSLSGroup::handle, worker_number=%d, no role map writable sock=%d,why?",
                    this, m_worker_number, m_write_socks[i]);
            continue;
        }

        CSLSRole* role = it->second;
        if (!role) {
            sls_log(SLS_LOG_WARNING, "[%p]CSLSGroup::handle, worker_number=%d, role is null, writable sock=%d,why?",
                    this, m_worker_number, m_write_socks[i]);
            continue;
        }

        ret = role->handler();
        if (ret < 0) {
        	//handle exception
            role->uninit();
            sls_log(SLS_LOG_WARNING, "[%p]CSLSGroup::handle, worker_number=%d, write sock=%d is invalid, %s=%p, write_len=%d, role_map.size=%d.",
                    this, m_worker_number, m_write_socks[i], role->get_role_name(), role, write_len, m_map_role.size());
        }
        handler_count += ret;
    }

    //handle readable sock
    if (read_len > 0) {
        sls_log(SLS_LOG_TRACE, "[%p]CSLSGroup::handle, worker_number=%d, readable sock count=%d.",
                this, m_worker_number, read_len);
    }
    for (i = 0; i < read_len; i ++) {
        std::map<int, CSLSRole *>::iterator it = m_map_role.find(m_read_socks[i]);
        if (it == m_map_role.end()) {
            sls_log(SLS_LOG_WARNING, "[%p]CSLSGroup::handle, worker_number=%d, no role map readable sock=%d,why?",
                    this, m_worker_number, m_read_socks[i]);
            continue;
        }

        CSLSRole* role = it->second;
        if (!role) {
            sls_log(SLS_LOG_WARNING, "[%p]CSLSGroup::handle, worker_number=%d, role is null, readable sock=%d,why?",
                     this, m_worker_number, m_read_socks[i]);
            continue;
        }

        ret = role->handler();
        if (ret < 0) {
        	//handle exception
            role->uninit();
            sls_log(SLS_LOG_WARNING, "[%p]CSLSGroup::handle, worker_number=%d, readable sock=%d is invalid, %s=%p, readable len=%d, role_map.size=%d.",
                      this, m_worker_number, m_read_socks[i], role->get_role_name(), role, read_len, m_map_role.size());
        }
        handler_count += ret;
    }

    if (0 == handler_count) {
        sls_log(SLS_LOG_TRACE, "[%p]CSLSGroup::handle, worker_number=%d, check_invalid_sock, handler_count=0.", this, m_worker_number);
        check_invalid_sock();
        sls_log(SLS_LOG_TRACE, "[%p]CSLSGroup::handle, worker_number=%d, check_new_role, handler_count=0.", this, m_worker_number);
        check_new_role();
        //sls_log(SLS_LOG_TRACE, "[%p]CSLSGroup::handle, handler_count=0, sleep a moment.", this);
        msleep(1);
    }
	return SLS_OK;
}

void CSLSGroup::check_invalid_sock()
{
    std::map<int, CSLSRole *>::iterator it;
    std::map<int, CSLSRole *>::iterator it_erase;
    for(it=m_map_role.begin(); it!=m_map_role.end();) {
        CSLSRole * role = it->second;
        if (!role) {
            it ++;
            continue;
        }

        if (role->get_parent() == NULL) {
             //no parent, delete
            sls_log(SLS_LOG_INFO, "[%p]CSLSGroup::check_invalid_sock, worker_number=%d, %s=%p no parent, remove and delete, role_map.size=%d.",
                    this, m_worker_number, role->get_role_name(), role, m_map_role.size());

            it_erase = it;
            it ++;
            m_map_role.erase(it_erase);

            role->uninit();
            delete role;
            continue;
        }

        int state = role->get_state();
        if (SLS_RS_INITED == state) {
            //close in network
            int ret = role->get_sock_state();
            if (SRTS_BROKEN == ret || SRTS_CLOSED == ret || SRTS_NONEXIST == ret) {
                sls_log(SLS_LOG_WARNING, "[%p]CSLSGroup::check_invalid_sock, worker_number=%d, %s=%p, invalid sock=%d, role_map.size=%d.",
                          this, m_worker_number, role->get_role_name(), role, role->get_fd(), m_map_role.size());
                role->uninit();
            }
        }
        it ++;
    }
}

void CSLSGroup::clear()
{
    sls_log(SLS_LOG_INFO, "[%p]CSLSGroup::clear, worker_number=%d, role_map.size=%d.",
            this, m_worker_number, m_map_role.size());
    std::map<int, CSLSRole *>::iterator it;
    for(it=m_map_role.begin(); it!=m_map_role.end(); it++) {
        CSLSRole * role = it->second;
        if (role) {
            role->uninit();
            role->clear(false, false);
            delete role;
        }
    }
    m_map_role.clear();
}

void CSLSGroup::set_role_list(CSLSRoleList *list_role)
{
    m_list_role = list_role;
}

void CSLSGroup::set_worker_number(int n)
{
    m_worker_number = n;
}

void CSLSGroup::set_worker_connections(int n)
{
    m_worker_connections = n;
}


