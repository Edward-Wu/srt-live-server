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


#include "SLSPlayer.hpp"
#include "SLSLog.hpp"

/**
 * CSLSPlayer class implementation
 */

CSLSPlayer::CSLSPlayer()
{
    m_is_write = 1;
    sprintf(m_role_name, "player");
}
CSLSPlayer::~CSLSPlayer()
{
}


int CSLSPlayer::handler()
{
	int ret = 0;
	int write_size = 0;
    sls_log(SLS_LOG_TRACE, "[%p]CSLSPlayer::handler, m_list_data.count=%d.", this, m_list_data.size());
    while (1) {
        SLSRecycleNode * node = m_list_data.pop();
        if (NULL == node) {
            break;//no data to send
        }
        sls_log(SLS_LOG_TRACE, "[%p]CSLSPlayer::handler, m_list_data.count=%d, write node=%p.", this, m_list_data.size(), node);
        ret = write(node->data, node->data_len);
        sls_log(SLS_LOG_TRACE, "[%p]CSLSPlayer::handler, m_list_data.count=%d, recycle node=%p.", this, m_list_data.size(), node);
        m_list_data.recycle(node);
        if (ret <= 0) {
            break;
        }
        write_size += ret;
    }
    sls_log(SLS_LOG_TRACE, "[%p]CSLSPlayer::handler, ret=%d.", this, ret);
    return write_size ;
}




