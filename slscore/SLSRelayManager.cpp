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

#include "common.hpp"
#include "SLSRelayManager.hpp"
#include "SLSLog.hpp"


/**
 * CSLSRelayManager class implementation
 */
CSLSRelayManager::CSLSRelayManager()
{
	m_reconnect_begin_tm = 0;
    m_map_publisher      = NULL;
    m_map_data           = NULL;
    m_role_list          = NULL;
    m_sri                = NULL;

    memset(m_app_uplive, 0, sizeof(m_app_uplive));
    memset(m_stream_name, 0, sizeof(m_stream_name));

}

CSLSRelayManager::~CSLSRelayManager()
{
}

void CSLSRelayManager::set_map_publisher(CSLSMapPublisher *map_publisher)
{
	m_map_publisher = map_publisher;
}

void CSLSRelayManager::set_map_data(CSLSMapData *map_data)
{
	m_map_data = map_data;
}

void CSLSRelayManager::set_role_list(CSLSRoleList *role_list)
{
	m_role_list = role_list;
}

void CSLSRelayManager::set_relay_conf(SLS_RELAY_INFO *sri)
{
	m_sri = sri;
}

void CSLSRelayManager::set_relay_info(const char *app_uplive, const char *stream_name)
{
	strcpy(m_app_uplive, app_uplive);
	strcpy(m_stream_name, stream_name);
}

int CSLSRelayManager::connect(const char *url)
{
	int ret = SLS_ERROR;
	if (url == NULL || strlen(url) == 0) {
	    sls_log(SLS_LOG_INFO, "[%p]CSLSManager::connect, failed, url=%s.", url?url:"null");
        return ret;
	}

	CSLSRelay * cur_relay = create_relay();//new relay;
    cur_relay->init();
    ret = cur_relay->open(url);
    if (SLS_OK == ret) {
    	ret = set_relay_param(cur_relay);
    	if (SLS_OK != ret) {
    		cur_relay->uninit();
    		delete cur_relay;
    		cur_relay = NULL;
    	}
	    return ret;
    } else {
    	cur_relay->uninit();
    	delete cur_relay ;
    	cur_relay = NULL;
    }
	return ret;
}

int CSLSRelayManager::connect_hash()
{
	//make hash to hostnames by stream_name
	std::string url = get_hash_url();
	char szURL[1024] = {0};
	sprintf(szURL, "srt://%s/%s", url.c_str(), m_stream_name);
	int ret = connect(szURL);
	if (SLS_OK != ret) {
	    sls_log(SLS_LOG_INFO, "[%p]CSLSRelayManager::connect_hash, faild, connect szURL=%s, m_stream_name=%s.",
	    		this, szURL, m_stream_name);
	} else {
        sls_log(SLS_LOG_INFO, "[%p]CSLSRelayManager::connect_hash, ok, connect szURL=%s, m_stream_name=%s.",
    		    this, szURL, m_stream_name);
	}
    return ret;
}


std::string CSLSRelayManager::get_hash_url()
{
	if (NULL == m_sri) {
		return "";
	}
	uint32_t key = sls_hash_key(m_stream_name, strlen(m_stream_name));
	uint32_t index = key % m_sri->m_upstreams.size();
	return m_sri->m_upstreams[index];

}
