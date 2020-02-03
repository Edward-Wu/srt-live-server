
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

void CSLSRoleList::erase()
{
    CSLSLock lock(&m_mutex);
    sls_log(SLS_LOG_TRACE, "[%p]CSLSRoleList::erase, list.count=%d", this, m_list_role.size());
    std::list<CSLSRole * >::iterator it_erase;
    for (std::list<CSLSRole * >::iterator it = m_list_role.begin(); it != m_list_role.end();)
    {
        CSLSRole * role = *it;
        if (role) {
        	role->uninit();
            delete role;
        }
        it ++;
    }
    m_list_role.clear();
}



int CSLSRoleList::size()
{
    CSLSLock lock(&m_mutex);
	return m_list_role.size();
}




