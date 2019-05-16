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

#include <stdio.h>


#include "SLSRecycleList.hpp"
#include "SLSLog.hpp"

CSLSRecycleList::CSLSRecycleList()
{
    m_nNodeSize = 100;//default
}
CSLSRecycleList::~CSLSRecycleList()
{

}

void CSLSRecycleList::setNodeSize(int n)
{
    m_nNodeSize = n;
}

int CSLSRecycleList::size()
{
    CSLSLock lock(&m_mutex);
    return m_listData.size();
}


int CSLSRecycleList::push(char * data, int len)
{
    if (!data || len <= 0) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSRecycleList::push, failed, data=%p, len=%d.",
                this, data, len);
        return SLS_ERROR;
    }

    CSLSLock lock(&m_mutex);
    SLSRecycleNode * p = NULL;

    sls_log(SLS_LOG_TRACE, "[%p]CSLSRecycleList::push, m_listData.size=%d, m_listIdle.size=%d.",
            this, m_listData.size(), m_listIdle.size());
    if (m_listIdle.size() > 0) {
        p = m_listIdle.front();
        if (p)
        {
            sls_log(SLS_LOG_TRACE, "[%p]CSLSRecycleList::push, m_listIdle.size=%d, p=%p.",
                    this, m_listIdle.size(), p);
            m_listIdle.pop_front();
            p->copy(data, len);
        }
    }
    if (NULL == p){
        if (m_listData.size() < m_nNodeSize) {
            p = new SLSRecycleNode(data, len);
        } else {
            sls_log(SLS_LOG_ERROR, "[%p]CSLSRecycleList::push, push data failed, m_listData.size[%d] = m_nNodeSize[%d].",
                    this, m_listData.size(), m_nNodeSize);
            return SLS_ERROR;
        }
    }
    sls_log(SLS_LOG_TRACE, "[%p]CSLSRecycleList::push, push_back, p=%p.",
            this, p);
    m_listData.push_back(p);
    return len;
}

int CSLSRecycleList::push(SLSRecycleNode *node)
{
	if (NULL == node) {
	    sls_log(SLS_LOG_INFO, "[%p]CSLSRecycleList::push failed, node is null.", this);
		return 0;
	}
    return push(node->data, node->data_len);
}
SLSRecycleNode * CSLSRecycleList::pop()
{
    CSLSLock lock(&m_mutex);
    sls_log(SLS_LOG_TRACE, "[%p]CSLSRecycleList::pop, m_listData.size=%d.",
            this, m_listData.size());
    SLSRecycleNode * p = NULL;
    if (m_listData.size() > 0) {
        p = m_listData.front();
        if (p) {
            sls_log(SLS_LOG_TRACE, "[%p]CSLSRecycleList::pop, pop_front, m_listData.size=%d, p=%p.",
                    this, m_listData.size(), p);
            m_listData.pop_front();
        }
    }
    return p;
}

void CSLSRecycleList::recycle(SLSRecycleNode *node)
{
    if (node)
    {
        CSLSLock lock(&m_mutex);
        m_listIdle.push_back(node);
    }
}

void CSLSRecycleList::erase()
{
    CSLSLock lock(&m_mutex);
    sls_log(SLS_LOG_TRACE, "[%p]CSLSRecycleList::erase, m_listData.size=%d, m_listIdle.size=%d.",
            this, m_listData.size(), m_listIdle.size());

    while (!m_listData.empty()) {
    	SLSRecycleNode * p = m_listData.front();
    	if (p) {
    	    m_listData.pop_front();
    		delete p;
    	}
    }
    while (!m_listIdle.empty()) {
    	SLSRecycleNode * p = m_listIdle.front();
    	if (p) {
    		m_listIdle.pop_front();
    		delete p;
    	}
    }
}

