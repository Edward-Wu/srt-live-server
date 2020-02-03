
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
		const char *szTmp = m_sri->m_upstreams[i].c_str();
		sprintf(szURL, "srt://%s/%s", szTmp, m_stream_name);
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

	//check publisher
	char key_stream_name[1024] = {0};
	sprintf(key_stream_name, "%s/%s", m_app_uplive, m_stream_name);
	if (NULL != m_map_publisher) {
	    CSLSRole * publisher = m_map_publisher->get_publisher(key_stream_name);
	    if (NULL == publisher) {
	        sls_log(SLS_LOG_INFO, "[%p]CSLSPullerManager::start, failed, key_stream_name=%s, publisher=NULL not exist.",
	    		this, key_stream_name);
	        return ret;
	    }
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

	//check publisher
    bool no_publisher = false;
	char key_stream_name[1024] = {0};
	sprintf(key_stream_name, "%s/%s", m_app_uplive, m_stream_name);
	if (NULL != m_map_publisher) {
	    CSLSRole * publisher = m_map_publisher->get_publisher(key_stream_name);
	    if (NULL == publisher) {
	        no_publisher = true;
	    }
	}

	if (SLS_PM_ALL == m_sri->m_mode) {
		ret = reconnect_all(cur_tm_ms, no_publisher);
		return ret;
	} else if (SLS_PM_HASH == m_sri->m_mode) {
		if (cur_tm_ms - m_reconnect_begin_tm < (m_sri->m_reconnect_interval * 1000)) {
			return ret;
		}
		m_reconnect_begin_tm = cur_tm_ms;
		if (no_publisher) {
	        sls_log(SLS_LOG_INFO, "[%p]CSLSPullerManager::reconnect, connect_hash failed, key_stream_name=%s, publisher=NULL not exist.",
	    		this, key_stream_name);
			return ret;
		}
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

int CSLSPusherManager::reconnect_all(int64_t cur_tm_ms, bool no_publisher)
{
    CSLSLock lock(&m_rwclock, true);

    int ret = SLS_ERROR;
    int all_ret = SLS_OK;
    std::map<std::string, int64_t>::iterator it_cur;
    std::map<std::string, int64_t>::iterator it;
    for(it=m_map_reconnect_relay.begin(); it!=m_map_reconnect_relay.end();) {
        std::string url = it->first;
        int64_t begin_tm = it->second;
        it_cur = it;
        it ++;
        if (cur_tm_ms - begin_tm < (m_sri->m_reconnect_interval * 1000)) {
        	all_ret |= ret;
        	continue;
        }
        if (no_publisher) {
        	//it_cur->second = cur_tm_ms；
        	all_ret |= ret;
    	    m_map_reconnect_relay[url] = cur_tm_ms;
	        sls_log(SLS_LOG_INFO, "[%p]CSLSPullerManager::reconnect_all, failed, url=%s, publisher=NULL not exist.",
	    		this, url.c_str());
        	continue;
        }
    	ret = connect(url.c_str());
    	if (SLS_OK != ret) {
    	    sls_log(SLS_LOG_INFO, "[%p]CSLSRelayManager::reconnect_all, faild, connect url='%s'.",
    	    		this, url.c_str());
    	    m_map_reconnect_relay[url] = cur_tm_ms;
    	} else {
            sls_log(SLS_LOG_INFO, "[%p]CSLSRelayManager::reconnect_all, ok, connect url='%s', erase item from m_map_reconnect_relay.",
        		    this, url.c_str());
            m_map_reconnect_relay.erase(it_cur);
    	}
    	all_ret |= ret;
    }

	return all_ret;
}


