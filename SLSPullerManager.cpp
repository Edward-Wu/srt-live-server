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
#include "SLSPullerManager.hpp"
#include "SLSLog.hpp"
#include "SLSPuller.hpp"

/**
 * CSLSPullerManager class implementation
 */

CSLSPullerManager::CSLSPullerManager()
{
	m_cur_loop_index = -1;
}

CSLSPullerManager::~CSLSPullerManager()
{
}


//start to connect from next of cur index per time.
int CSLSPullerManager::connect_loop()
{
	int ret = SLS_ERROR;

	if (m_sri == NULL || m_sri->m_upstreams.size() == 0) {
	    sls_log(SLS_LOG_INFO, "[%p]CSLSPullerManager::connect_loop, failed, m_upstreams.size()=0, m_app_uplive=%s, m_stream_name=%s.",
	    		this, m_app_uplive, m_stream_name);
		return ret;
	}

	CSLSRelay *puller = NULL;
	if (-1 == m_cur_loop_index) {
		m_cur_loop_index = m_sri->m_upstreams.size() - 1;
	}
	int index = m_cur_loop_index ;
	index ++;

	char szURL[1024] = {0};
	while (true) {
		if (index >= m_sri->m_upstreams.size())
			index = 0;

		sprintf(szURL, "srt://%s/%s", m_sri->m_upstreams[index].c_str(), m_stream_name);
		ret = connect(szURL);
		if (SLS_OK == ret) {
			break;
		}
		if (index == m_cur_loop_index) {
		    sls_log(SLS_LOG_INFO, "[%p]CSLSPullerManager::connect_loop, failed, no available pullers, m_app_uplive=%s, m_stream_name=%s.",
		    		this, m_app_uplive, m_stream_name);
			break;
		}
	    sls_log(SLS_LOG_INFO, "[%p]CSLSPullerManager::connect_loop, failed, index=%d, m_app_uplive=%s, m_stream_name=%s, szURL=‘%s’.",
	    		this, m_app_uplive, m_stream_name, szURL);
		index ++;
	}
	m_cur_loop_index = index;
    return ret;
}

int CSLSPullerManager::start()
{
	int ret = SLS_ERROR;

	if (NULL == m_sri) {
	    sls_log(SLS_LOG_INFO, "[%p]CSLSPullerManager::start, failed, m_upstreams.size()=0, m_app_uplive=%s, m_stream_name=%s.",
	    		this, m_app_uplive, m_stream_name);
		return ret;
	}

	if (SLS_PM_LOOP == m_sri->m_mode) {
		ret = connect_loop();
	} else if (SLS_PM_HASH == m_sri->m_mode) {
		ret =  connect_hash();
	} else {
	    sls_log(SLS_LOG_INFO, "[%p]CSLSPullerManager::start, failed, wrong m_sri->m_mode=%d, m_app_uplive=%s, m_stream_name=%s.",
	    		this, m_sri->m_mode, m_app_uplive, m_stream_name);
	}
	return ret;
}

CSLSRelay *CSLSPullerManager::create_relay()
{
     CSLSRelay * relay = new CSLSPuller;
     return relay;
}

int CSLSPullerManager::check_relay_param()
{
	if (NULL == m_role_list) {
		sls_log(SLS_LOG_WARNING, "[%p]CSLSRelayManager::check_relay_param, failed, m_role_list is null, stream=%s.",
					this, m_stream_name);
		return SLS_ERROR;
	}
	if (NULL == m_map_publisher) {
		sls_log(SLS_LOG_WARNING, "[%p]CSLSRelayManager::check_relay_param, failed, m_map_publisher is null, stream=%s.",
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

int CSLSPullerManager::set_relay_param(CSLSRelay *relay)
{
	char key_stream_name[1024] = {0};
	sprintf(key_stream_name, "%s/%s", m_app_uplive, m_stream_name);

	if (SLS_OK != check_relay_param()){
		sls_log(SLS_LOG_WARNING, "[%p]CSLSRelayManager::set_relay_param, check_relay_param failed, stream=%s.",
					this, key_stream_name);
		return SLS_ERROR;
	}

	if (SLS_OK != m_map_publisher->set_push_2_pushlisher(key_stream_name, relay)) {
		sls_log(SLS_LOG_WARNING, "[%p]CSLSRelayManager::set_relay_param, m_map_publisher->set_push_2_pushlisher, stream=%s.",
					this, key_stream_name);
		return SLS_ERROR;
	}

	if (SLS_OK != m_map_data->add(key_stream_name)) {
		sls_log(SLS_LOG_WARNING, "[%p]CSLSRelayManager::set_relay_param, m_map_data->add failed, stream=%s, remove from relay=%p, m_map_publisher.",
					this, key_stream_name, relay);
		m_map_publisher->remove(relay);
		return SLS_ERROR;
	}

	relay->set_map_data(key_stream_name, m_map_data);
	relay->set_map_publisher(m_map_publisher);
	relay->set_relay_manager(this);
	m_role_list->push(relay);

	return SLS_OK;

}

int  CSLSPullerManager::add_reconnect_stream(char* relay_url)
{
	m_reconnect_begin_tm = sls_gettime_relative()/1000;
}

int CSLSPullerManager::reconnect(int64_t cur_tm_ms)
{
	int ret = SLS_ERROR;
	if (cur_tm_ms - m_reconnect_begin_tm < (m_sri->m_reconnect_interval * 1000)) {
		return ret;
	}
	m_reconnect_begin_tm = cur_tm_ms;

    if (SLS_OK != check_relay_param()) {
		sls_log(SLS_LOG_WARNING, "[%p]CSLSPullerManager::reconnect, check_relay_param failed, stream=%s.",
					this, m_stream_name);
		return SLS_ERROR;
    }

	char key_stream_name[1024] = {0};
	sprintf(key_stream_name, "%s/%s", m_app_uplive, m_stream_name);

	ret = start();
	if (SLS_OK != ret) {
		sls_log(SLS_LOG_INFO, "[%p]CSLSPullerManager::reconnect, start failed, key_stream_name=%s.",
					this, key_stream_name);
	} else {
		sls_log(SLS_LOG_INFO, "[%p]CSLSPullerManager::reconnect, start ok, key_stream_name=%s.",
					this, key_stream_name);
	}
	return ret;
}

