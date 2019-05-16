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


#include "SLSRoleList.hpp"
#include "SLSLog.hpp"
#include "SLSLock.hpp"

/**
 * CSLSRoleList class implementation
 */

CSLSRoleList::CSLSRoleList()
{
}
CSLSRoleList::~CSLSRoleList()
{
}

int CSLSRoleList::push(CSLSRole * role)
{
	if (role) {
	    CSLSLock lock(&m_mutex);
	    m_list_role.push_back(role);
	}
	return 0;
}

CSLSRole * CSLSRoleList::pop()
{
	CSLSLock lock(&m_mutex);
	CSLSRole * role = NULL;
    if (!m_list_role.empty()) {
        role = m_list_role.front();
        m_list_role.pop_front();
    }
	return role;
}

void CSLSRoleList::erase(bool invalid, bool del)
{
    CSLSLock lock(&m_mutex);
    sls_log(SLS_LOG_TRACE, "[%p]CSLSRoleList::erase, list.count=%d", this, m_list_role.size());
    std::list<CSLSRole * >::iterator it_erase;
    for (std::list<CSLSRole * >::iterator it = m_list_role.begin(); it != m_list_role.end();)
    {
        CSLSRole * role = *it;
        if (!role) {
            sls_log(SLS_LOG_WARNING, "[%p]CSLSRoleList::erase, %s[%p] is null, remove it.",
                    this, role->get_role_name(), role);
        }
        if (role) {
            if (invalid)
            {
                role->set_parent(NULL);
            }
            if (del)
            {
                delete role;
            }
        }
        it_erase = it;
        it ++;
        m_list_role.erase(it_erase);
    }
}



int CSLSRoleList::size()
{
    CSLSLock lock(&m_mutex);
	return m_list_role.size();
}
/*
int CSLSRoleList::read(char *buf, int size)
{
    int ret;
	//dispatch data
    for (std::list<CSLSRole * >::iterator it=m_list_role.begin(); it != m_list_role.end(); ++it)
    {
    	CSLSRole * role = *it;
    	if (role) {
    		role->read(buf, size);
    	} else {
            sls_log(SLS_LOG_INFO, "CSLSRoleList::read, role is null, remove it.");
            it = m_list_role.erase(it);
    	}
    }

    return ret;
}
*/


int CSLSRoleList::push_data(const char *buf, int size)
{
    int ret;
    CSLSLock lock(&m_mutex);
    //dispatch data
    std::list<CSLSRole * >::iterator it_erase;
    for (std::list<CSLSRole * >::iterator it=m_list_role.begin(); it != m_list_role.end();)
    {
        CSLSRole * role = *it;
        if (!role) {
            it_erase = it;
            it ++;
            m_list_role.erase(it_erase);
            sls_log(SLS_LOG_WARNING, "[%p]CSLSRoleList::push_data, %s[%p] is null, remove it, size=%d.",
                    this, role->get_role_name(), role, m_list_role.size());
            continue;

        }

        int state =role->get_state();
        if (SLS_RS_INITED == state) {
            ret = role->push_data(buf, size);
            //ret = role->write(buf, size);
            it ++;
            continue;
        }

        sls_log(SLS_LOG_WARNING, "[%p]CSLSRoleList::push_data, %s[%p] is invalid, remove it, size=%d.",
                this, role->get_role_name(), role, m_list_role.size());
        role->set_parent(NULL);
        it_erase = it;
        it ++;
        m_list_role.erase(it_erase);
    }
    return ret;

}



