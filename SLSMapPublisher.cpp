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


#include "SLSMapPublisher.hpp"
#include "SLSLog.hpp"


/**
 * CSLSMapPublisher class implementation
 */

CSLSMapPublisher::CSLSMapPublisher()
{
}
CSLSMapPublisher::~CSLSMapPublisher()
{
    clear();
}

void CSLSMapPublisher::set_conf(std::string key, sls_conf_base_t* ca)
{
    CSLSLock lock(&m_rwclock, true);
	m_map_uplive_2_conf[key] = ca;
}

void CSLSMapPublisher::set_live_2_uplive(std::string strLive, std::string strUplive)
{
    CSLSLock lock(&m_rwclock, true);
	m_map_live_2_uplive[strLive] = strUplive;
}

int CSLSMapPublisher::set_push_2_pushlisher(std::string app_streamname, CSLSRole * role)
{
    CSLSLock lock(&m_rwclock, true);
    std::map<std::string, CSLSRole *>::iterator it;
    it = m_map_push_2_pushlisher.find(app_streamname);
    if (it != m_map_push_2_pushlisher.end()) {
    	CSLSRole *cur_role = it->second;
    	if (NULL != cur_role) {
            sls_log(SLS_LOG_INFO, "[%p]CSLSMapPublisher::set_push_2_pushlisher, failed, cur_role=%p, exist, app_streamname=%s, m_map_push_2_pushlisher.size()=%d.",
                this, cur_role, app_streamname.c_str(), m_map_push_2_pushlisher.size());
            return SLS_ERROR;
    	}
    }

	m_map_push_2_pushlisher[app_streamname] = role;
    sls_log(SLS_LOG_INFO, "[%p]CSLSMapPublisher::set_push_2_pushlisher, ok, %s=%p, app_streamname=%s, m_map_push_2_pushlisher.size()=%d.",
            this, role->get_role_name(), role, app_streamname.c_str(), m_map_push_2_pushlisher.size());
    return SLS_OK;
}

std::string  CSLSMapPublisher::get_uplive(std::string key_app)
{
    CSLSLock lock(&m_rwclock, false);
	std::string uplive_app = "";
    std::map<std::string, std::string>::iterator it;
    it = m_map_live_2_uplive.find(key_app);//is publiser?
    if (it == m_map_live_2_uplive.end()) {
    	return uplive_app;
    }
    uplive_app = it->second;
    return uplive_app;
}


sls_conf_base_t * CSLSMapPublisher::get_ca(std::string key_app)
{
    CSLSLock lock(&m_rwclock, false);
    sls_conf_base_t * ca = NULL;
    std::map<std::string, sls_conf_base_t *>::iterator it;
    it = m_map_uplive_2_conf.find(key_app);
    if (it == m_map_uplive_2_conf.end()) {
        return ca;
    }
    ca = it->second;
    return ca;
}

CSLSRole * CSLSMapPublisher::get_publisher(std::string strAppStreamName)
{
    CSLSLock lock(&m_rwclock, false);

	CSLSRole * publisher = NULL;
    std::map<std::string, CSLSRole *>::iterator item;
    item = m_map_push_2_pushlisher.find(strAppStreamName);
    if (item != m_map_push_2_pushlisher.end()) {
        publisher = item->second;
    }
    return publisher;
}

int CSLSMapPublisher::remove(CSLSRole *role)
{
    int ret = SLS_ERROR;

    CSLSLock lock(&m_rwclock, true);

    std::map<std::string, CSLSRole *>::iterator it;
    std::map<std::string, CSLSRole *>::iterator it_erase;
    for(it=m_map_push_2_pushlisher.begin(); it!=m_map_push_2_pushlisher.end(); ) {
        std::string live_stream_name = it->first;
        CSLSRole * pub = it->second;
        if (role == pub) {
            sls_log(SLS_LOG_INFO, "[%p]CSLSMapPublisher::remove, %s=%p, live_key=%s.",
                    this, pub->get_role_name(), pub, live_stream_name.c_str());
            it_erase = it;
            it ++;
            m_map_push_2_pushlisher.erase(it_erase);
            ret = SLS_OK;
            break;
        } else {
            it ++;
        }
    }
    return ret;
}

void CSLSMapPublisher::clear()
{
    CSLSLock lock(&m_rwclock, true);
    m_map_push_2_pushlisher.clear();
    m_map_live_2_uplive.clear();
    m_map_uplive_2_conf.clear();
}



