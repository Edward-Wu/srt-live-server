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


#include "SLSPublisher.hpp"
#include "SLSPlayer.hpp"
#include "SLSLog.hpp"

/**
 * app conf
 */
SLS_CONF_DYNAMIC_IMPLEMENT(app)

/**
 * CSLSPublisher class implementation
 */

CSLSPublisher::CSLSPublisher()
{
    m_is_write    = 0;
    m_list_player = NULL;
    sprintf(m_role_name, "publisher");

}
CSLSPublisher::~CSLSPublisher()
{
    //release
    clear(true, false);
}

int CSLSPublisher::init()
{
    int ret = CSLSRole::init();
    if (m_conf) {
        m_exit_delay = ((sls_conf_app_t *)m_conf)->publisher_exit_delay;
    }

    return ret;
}

void CSLSPublisher::clear(bool invalid, bool del)
{
    if (m_list_player){
        m_list_player->erase(invalid, del);
        delete m_list_player;
        m_list_player = NULL;
    }
}


int CSLSPublisher::add_player(CSLSRole * player)
{
    if (NULL == m_list_player)
        m_list_player = new CSLSRoleList;
     m_list_player->push(player);
     return 0;
}


int CSLSPublisher::handler()
{
	char szData[TS_UDP_LEN];

    sls_log(SLS_LOG_TRACE, "[%p]CSLSPublisher::handler.", this);
	if (NULL == m_srt) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSPublisher::handler, m_srt is null.", this);
	    return SLS_ERROR;
	}
    //read data
    sls_log(SLS_LOG_TRACE, "[%p]CSLSPublisher::handler, libsrt_read.", this);
    int n = m_srt->libsrt_read(szData, TS_UDP_LEN);
	if (n <= 0) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSPublisher::handler, libsrt_read failure, n=%d.", this, n, TS_UDP_LEN);
		uninit();
	    return SLS_ERROR;
	}
    if (n != TS_UDP_LEN) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSPublisher::handler, libsrt_read n=%d, expect %d.", this, n, TS_UDP_LEN);
		uninit();
        return SLS_ERROR;
    }

    sls_log(SLS_LOG_TRACE, "[%p]CSLSPublisher::handler, push_data, m_list_player.count=%d.", this, m_list_player?m_list_player->size():0);
    if (m_list_player) {
	    m_list_player->push_data(szData, n);
    }

	return n;
}

CSLSRoleList * CSLSPublisher::get_player_list()
{
    return m_list_player;
}

void CSLSPublisher::set_player_list(CSLSRoleList * players)
{
    m_list_player = players;
}

int CSLSPublisher::get_player_count()
{
    if (m_list_player)
        return  m_list_player->size();
    return SLS_OK;
}

