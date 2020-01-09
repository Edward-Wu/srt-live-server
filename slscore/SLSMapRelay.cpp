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

#include "SLSMapRelay.hpp"

#include <errno.h>
#include <string.h>

#include "SLSPullerManager.hpp"
#include "SLSPusherManager.hpp"
#include "SLSLog.hpp"


/**
 * CSLSMapRelay class implementation
 */

CSLSMapRelay::CSLSMapRelay()
{
}

CSLSMapRelay::~CSLSMapRelay()
{
    clear();
}

CSLSRelayManager * CSLSMapRelay::add_relay_manager(const char *app_uplive, const char *stream_name)
{
    //find conf info
	SLS_RELAY_INFO * sri = get_relay_conf(std::string(app_uplive));
	if (NULL == sri) {
    	sls_log(SLS_LOG_INFO, "[%p]CSLSMapRelay::add_relay_manager, no relay conf info, app_uplive=%s, stream_name=%s.",
                this, app_uplive, stream_name);
		return NULL;
	}

    std::string key_stream_name = std::string(app_uplive) + std::string("/") + std::string(stream_name);
    CSLSLock lock(&m_rwclock, true);
    CSLSRelayManager * cur_mananger = NULL;
    std::map<std::string, CSLSRelayManager *>::iterator item;
    item = m_map_relay_manager.find(key_stream_name);
    if (item != m_map_relay_manager.end()) {
    	cur_mananger = item->second;
        if (NULL != cur_mananger) {
        	sls_log(SLS_LOG_INFO, "[%p]CSLSMapRelay::add, cur_mananger=%p, exist, app_uplive=%s, stream_name=%s.",
                    this, app_uplive, stream_name, cur_mananger);
            return cur_mananger;
        }
    }

    if (strcmp(sri->m_type, "pull") == 0)
        cur_mananger = new CSLSPullerManager;
    else if (strcmp(sri->m_type, "push") == 0)
        cur_mananger = new CSLSPusherManager;
    else {
    	sls_log(SLS_LOG_INFO, "[%p]CSLSMapRelay::add, failed, wrong , app_uplive=%s, stream_name=%s.",
                this, app_uplive, stream_name, cur_mananger);
    	return NULL;
    }
    cur_mananger->set_relay_conf(sri);
    cur_mananger->set_relay_info(app_uplive, stream_name);

    m_map_relay_manager[key_stream_name] = cur_mananger ;
	sls_log(SLS_LOG_INFO, "[%p]CSLSMapRelay::add_relay_manager, ok, app_uplive=%s, stream_name=%s, cur_mananger=%p.",
            this, app_uplive, stream_name, cur_mananger);
    return cur_mananger;
}

void CSLSMapRelay::clear()
{
    CSLSLock lock(&m_rwclock, true);

    std::map<std::string, CSLSRelayManager *>::iterator it;
    for(it=m_map_relay_manager.begin(); it!=m_map_relay_manager.end(); ) {
        CSLSRelayManager * relay_manager = it->second;
        if (NULL != relay_manager) {
		    delete relay_manager;
		}
    	it ++;
    }
    m_map_relay_manager.clear();

    std::map<std::string, SLS_RELAY_INFO *>::iterator it_sri;
    for(it_sri=m_map_relay_info.begin(); it_sri!=m_map_relay_info.end(); ) {
        SLS_RELAY_INFO * sri = it_sri->second;
        if (NULL != sri) {
		    delete sri;
		}
        it_sri ++;
    }
    m_map_relay_info.clear();
}

int CSLSMapRelay::add_relay_conf(std::string app_uplive, sls_conf_relay_t * cr)
{
    if (NULL == cr) {
    	sls_log(SLS_LOG_INFO, "[%p]CSLSMapRelay::add_app_conf, failed, cr is null.",
                this);
    	return SLS_ERROR;
    }

    SLS_RELAY_INFO * sri = get_relay_conf(app_uplive);
    if (NULL != sri) {
    	sls_log(SLS_LOG_INFO, "[%p]CSLSMapRelay::add_app_conf, failed, sri exist, app_uplive=%s.",
                this, app_uplive.c_str());
    	return SLS_ERROR;
    }
    sri = new SLS_RELAY_INFO;
    strcpy(sri->m_type, cr->type);
    sri->m_reconnect_interval = cr->reconnect_interval;

    if (strcmp(cr->mode, "loop") == 0) {
    	sri->m_mode = SLS_PM_LOOP;
    } else if (strcmp(cr->mode, "all") == 0) {
    	sri->m_mode = SLS_PM_ALL;
    } else if (strcmp(cr->mode, "hash") == 0) {
    	sri->m_mode = SLS_PM_HASH;
    } else {
    	sri->m_mode = SLS_PM_HASH;
    	sls_log(SLS_LOG_INFO, "[%p]CSLSMapRelay::add_app_conf, wrong mode='%s', use default SLS_PM_LOOP.",
                this, cr->mode);
    }

    //parse upstreams
    sri->m_upstreams = sls_conf_string_split(string(cr->upstreams), string(" "));
    if (sri->m_upstreams.size() == 0) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSMapRelay::add_app_conf, wrong upstreams='%s'.", this, cr->upstreams);
    }
    m_map_relay_info[app_uplive] = sri;
    return SLS_OK;

}

SLS_RELAY_INFO *CSLSMapRelay::get_relay_conf(std::string app_uplive)
{
    SLS_RELAY_INFO * sri = NULL;
	std::map<std::string, SLS_RELAY_INFO *>::iterator item;
    item = m_map_relay_info.find(app_uplive);
    if (item != m_map_relay_info.end()) {
    	sri = item->second;
    }
    return sri;

}




