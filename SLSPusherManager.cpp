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
#include "SLSPusherManager.hpp"
#include "SLSLog.hpp"
#include "SLSPusher.hpp"

/**
 * CSLSPusherManager class implementation
 */

CSLSPusherManager::CSLSPusherManager()
{
}

CSLSPusherManager::~CSLSPusherManager()
{
}

//start to connect from next of cur index per time.
int CSLSPusherManager::connect_all()
{
	int ret = SLS_ERROR;
	if (m_sri == NULL) {
	    sls_log(SLS_LOG_INFO, "[%p]CSLSPusherManager::connect_all, failed, m_upstreams.size()=0, m_app_uplive=%s, m_stream_name=%s.",
	    		this, m_app_uplive, m_stream_name);
		return ret;
	}

	int all_ret = SLS_OK;
	for (int i = 0; i < m_sri->m_upstreams.size(); i ++) {
		char szURL[1024] = {0};
		sprintf(szURL, "srt://%s/%s", m_sri->m_upstreams[i].c_str(), m_stream_name);
		ret = connect(szURL);
		if (SLS_OK != ret) {
		    CSLSLock lock(&m_rwclock, true);
			m_map_reconnect_relay[std::string(szURL)] = sls_gettime_relative()/1000;
		}
		all_ret |= ret;
	}
    return all_ret;
}

int CSLSPusherManager::start()
{
	int ret = SLS_ERROR;
	if (m_sri == NULL) {
	    sls_log(SLS_LOG_INFO, "[%p]CSLSPusherManager::start, failed, m_upstreams.size()=0, m_app_uplive=%s, m_stream_name=%s.",
	    		this, m_app_uplive, m_stream_name);
		return ret;
	}

	if (SLS_PM_ALL == m_sri->m_mode) {
		return connect_all();
	} else if (SLS_PM_HASH == m_sri->m_mode) {
		ret = connect_hash();
	} else {
	    sls_log(SLS_LOG_INFO, "[%p]CSLSPusherManager::start, failed, wrong m_sri->m_mode=%d, m_app_uplive=%s, m_stream_name=%s.",
	    		this, m_sri->m_mode, m_app_uplive, m_stream_name);
	}
	return ret;
}

CSLSRelay *CSLSPusherManager::create_relay()
{
     CSLSRelay * relay = new CSLSPusher;
     return relay;
}

int CSLSPusherManager::set_relay_param(CSLSRelay *relay)
{
	char key_stream_name[1024] = {0};
	sprintf(key_stream_name, "%s/%s", m_app_uplive, m_stream_name);
	relay->set_map_data(key_stream_name, m_map_data);
	relay->set_map_publisher(m_map_publisher);
	relay->set_relay_manager(this);
	m_role_list->push(relay);
    return SLS_OK;
}

int CSLSPusherManager::add_reconnect_stream(char* relay_url)
{
	int ret = SLS_ERROR;
	if (m_sri == NULL) {
	    sls_log(SLS_LOG_INFO, "[%p]CSLSPusherManager::add_reconnect_stream, failed, m_upstreams.size()=0, m_app_uplive=%s, m_stream_name=%s.",
	    		this, m_app_uplive, m_stream_name);
		return ret;
	}

	if (SLS_PM_ALL == m_sri->m_mode) {
		std::string url = std::string(relay_url);
	    CSLSLock lock(&m_rwclock, true);
	    int64_t tm = sls_gettime_relative()/1000;
        m_map_reconnect_relay[url] = tm;
        ret = SLS_OK;
	} else if (SLS_PM_HASH == m_sri->m_mode) {
		m_reconnect_begin_tm = sls_gettime_relative()/1000;
        ret = SLS_OK;
	} else {
	    sls_log(SLS_LOG_INFO, "[%p]CSLSPusherManager::add_reconnect_stream, failed, wrong m_sri->m_mode=%d, m_app_uplive=%s, m_stream_name=%s.",
	    		this, m_sri->m_mode, m_app_uplive, m_stream_name);
	}
	return ret;
}

int CSLSPusherManager::reconnect(int64_t cur_tm_ms)
{
	int ret = SLS_ERROR;
    if (SLS_OK != check_relay_param()) {
		sls_log(SLS_LOG_WARNING, "[%p]CSLSPusherManager::reconnect, check_relay_param failed, stream=%s.",
					this, m_stream_name);
		return ret;
    }

	if (m_sri == NULL) {
	    sls_log(SLS_LOG_INFO, "[%p]CSLSPusherManager::reconnect, failed, m_upstreams.size()=0, m_app_uplive=%s, m_stream_name=%s.",
	    		this, m_app_uplive, m_stream_name);
		return ret;
	}

	if (SLS_PM_ALL == m_sri->m_mode) {
		ret = reconnect_all(cur_tm_ms);
		return ret;
	} else if (SLS_PM_HASH == m_sri->m_mode) {
		if (cur_tm_ms - m_reconnect_begin_tm < (m_sri->m_reconnect_interval * 1000)) {
			return ret;
		}
		m_reconnect_begin_tm = cur_tm_ms;
		ret = connect_hash();
	} else {
	    sls_log(SLS_LOG_INFO, "[%p]CSLSPusherManager::reconnect, failed, wrong m_sri->m_mode=%d, m_app_uplive=%s, m_stream_name=%s.",
	    		this, m_sri->m_mode, m_app_uplive, m_stream_name);
	}
	return ret;

}

int CSLSPusherManager::check_relay_param()
{
	if (NULL == m_role_list) {
		sls_log(SLS_LOG_WARNING, "[%p]CSLSRelayManager::check_relay_param, failed, m_role_list is null, stream=%s.",
					this, m_stream_name);
		return SLS_ERROR;
	}
	if (NULL == m_map_data) {
		sls_log(SLS_LOG_WARNING, "[%p]CSLSRelayManager::check_relay_param, failed, m_map_data is null, stream=%s.",
					this, m_stream_name);
		return SLS_ERROR;
	}
    return SLS_OK;
}

int CSLSPusherManager::reconnect_all(int64_t cur_tm_ms)
{
    CSLSLock lock(&m_rwclock, true);

    int ret = SLS_ERROR;
    int all_ret = SLS_OK;
    std::map<std::string, int64_t>::iterator it_rease;
    std::map<std::string, int64_t>::iterator it;
    for(it=m_map_reconnect_relay.begin(); it!=m_map_reconnect_relay.end();) {
        std::string url = it->first;
        int64_t begin_tm = it->second;
        if (cur_tm_ms - begin_tm < (m_sri->m_reconnect_interval * 1000)) {
        	it ++;
        	all_ret |= ret;
        	continue;
        }
    	ret = connect(url.c_str());
    	if (SLS_OK != ret) {
    	    sls_log(SLS_LOG_INFO, "[%p]CSLSRelayManager::reconnect_all, faild, connect url='%s'.",
    	    		this, url.c_str());
    	    m_map_reconnect_relay[url] = cur_tm_ms;
    	} else {
    		it_rease = it;
    		it ++;
            sls_log(SLS_LOG_INFO, "[%p]CSLSRelayManager::reconnect_all, ok, connect url='%s', erase item.",
        		    this, url.c_str());
            m_map_reconnect_relay.erase(it_rease);
    	}
    	all_ret |= ret;
    }

	return all_ret;
}


