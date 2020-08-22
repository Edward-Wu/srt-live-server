
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
#include "SLSManager.hpp"
#include "SLSLog.hpp"
#include "SLSListener.hpp"
#include "SLSPublisher.hpp"

/**
 * srt conf
 */
SLS_CONF_DYNAMIC_IMPLEMENT(srt)

/**
 * CSLSManager class implementation
 */
#define DEFAULT_GROUP 1

CSLSManager::CSLSManager()
{
    m_worker_threads = DEFAULT_GROUP;
    m_server_count = 1;
    m_list_role      = NULL;
    m_single_group   = NULL;

    m_map_data       = NULL;
    m_map_publisher  = NULL;
    m_map_puller     = NULL;
    m_map_pusher     = NULL;

}

CSLSManager::~CSLSManager()
{
}

int CSLSManager::start()
{
	int ret = 0;
	int i = 0;

    //read config info from config file
    sls_conf_srt_t * conf_srt = (sls_conf_srt_t *)sls_conf_get_root_conf();

    if (!conf_srt) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSManager::start, no srt info, please check the conf file.", this);
        return SLS_ERROR;
    }
    //set log level
    if (strlen(conf_srt->log_level) > 0) {
        sls_set_log_level(conf_srt->log_level);
    }
    //set log file
    if (strlen(conf_srt->log_file) > 0) {
        sls_set_log_file(conf_srt->log_file);
    }

    sls_conf_server_t * conf_server = (sls_conf_server_t *)conf_srt->child;
    if (!conf_server) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSManager::start, no server info, please check the conf file.", this);
        return SLS_ERROR;
    }
    m_server_count = sls_conf_get_conf_count(conf_server);
    sls_conf_server_t * conf = conf_server;
    m_map_data      = new CSLSMapData[m_server_count];
    m_map_publisher = new CSLSMapPublisher[m_server_count];
    m_map_puller    = new CSLSMapRelay[m_server_count];
    m_map_pusher    = new CSLSMapRelay[m_server_count];

    //role list
    m_list_role = new CSLSRoleList;
    sls_log(SLS_LOG_INFO, "[%p]CSLSManager::start, new m_list_role=%p.", this, m_list_role);

    //create listeners according config, delete by groups
    for (i = 0; i < m_server_count; i ++) {
    	CSLSListener * p = new CSLSListener();//deleted by groups
    	p->set_role_list(m_list_role);
        p->set_conf(conf);
        p->set_record_hls_path_prefix(conf_srt->record_hls_path_prefix);
        p->set_map_data("", &m_map_data[i]);
        p->set_map_publisher(&m_map_publisher[i]);
        p->set_map_puller(&m_map_puller[i]);
        p->set_map_pusher(&m_map_pusher[i]);
    	if (p->init() != SLS_OK) {
            sls_log(SLS_LOG_INFO, "[%p]CSLSManager::start, p->init failed.", this);
            return SLS_ERROR;
    	}
        if (p->start() != SLS_OK) {
            sls_log(SLS_LOG_INFO, "[%p]CSLSManager::start, p->start failed.", this);
            return SLS_ERROR;
        }
    	m_servers.push_back(p);
    	conf = (sls_conf_server_t *)conf->sibling;
    }
    sls_log(SLS_LOG_INFO, "[%p]CSLSManager::start, init listeners, count=%d.", this, m_server_count);

    //create groups

    m_worker_threads = conf_srt->worker_threads;
    if (m_worker_threads == 0) {
        CSLSGroup * p = new CSLSGroup();
        p->set_worker_number(0);
        p->set_role_list(m_list_role);
        p->set_worker_connections(conf_srt->worker_connections);
        p->set_stat_post_interval(conf_srt->stat_post_interval);
        if (SLS_OK != p->init_epoll()) {
            sls_log(SLS_LOG_INFO, "[%p]CSLSManager::start, p->init_epoll failed.", this);
            return SLS_ERROR;
        }
        m_workers.push_back(p);
        m_single_group  =p;

    } else {
        for (i = 0; i < m_worker_threads; i ++) {
            CSLSGroup * p = new CSLSGroup();
            p->set_worker_number(i);
            p->set_role_list(m_list_role);
            p->set_worker_connections(conf_srt->worker_connections);
            p->set_stat_post_interval(conf_srt->stat_post_interval);
            if (SLS_OK != p->init_epoll()) {
                sls_log(SLS_LOG_INFO, "[%p]CSLSManager::start, p->init_epoll failed.", this);
                return SLS_ERROR;
            }
            p->start();
            m_workers.push_back(p);
        }
    }
    sls_log(SLS_LOG_INFO, "[%p]CSLSManager::start, init worker, count=%d.", this, m_worker_threads);

	return ret;

}

int CSLSManager::single_thread_handler()
{
    if (m_single_group) {
        return m_single_group->handler();
    }
    return SLS_OK;
}

bool CSLSManager::is_single_thread(){
    if (m_single_group)
        return true;
    return false;
}

int CSLSManager::stop()
{
	int ret = 0;
	int i = 0;
    //
    sls_log(SLS_LOG_INFO, "[%p]CSLSManager::stop.", this);

    //stop all listeners
    std::list<CSLSListener *>::iterator it;
    for ( it = m_servers.begin(); it != m_servers.end(); it++) {
    	CSLSListener *server = *it;
    	if (NULL == server) {
    		continue;
    	}
    	server->uninit();
    }
    m_servers.clear();

    std::list<CSLSGroup *>::iterator it_worker;
    for ( it_worker = m_workers.begin(); it_worker != m_workers.end(); it_worker++) {
    	CSLSGroup *p = *it_worker;
    	if (p) {
    		p->stop();
    		p->uninit_epoll();
    		delete p;
    		p = NULL;
    	}
    }
    m_workers.clear();

    if (m_map_data) {
    	delete[] m_map_data;
    	m_map_data = NULL;
    }
    if (m_map_publisher) {
    	delete[] m_map_publisher;
    	m_map_publisher = NULL;
    }

    if (m_map_puller) {
    	delete[] m_map_puller;
    	m_map_puller = NULL;
    }

    if (m_map_pusher) {
    	delete[] m_map_pusher;
    	m_map_pusher = NULL;
    }

    //release rolelist
    if(m_list_role) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSManager::stop, release rolelist, size=%d.", this, m_list_role->size());
    	m_list_role->erase();
    	delete m_list_role;
    	m_list_role = NULL;
    }
    return ret;
}

int CSLSManager::reload()
{
    sls_log(SLS_LOG_INFO, "[%p]CSLSManager::reload begin.", this);

    //stop all listeners
    std::list<CSLSListener *>::iterator it;
    for ( it = m_servers.begin(); it != m_servers.end(); it++) {
    	CSLSListener *server = *it;
    	if (NULL == server) {
    		continue;
    	}
    	server->uninit();
    }
    m_servers.clear();

    //set all groups reload flag
    std::list<CSLSGroup *>::iterator it_worker;
    for ( it_worker = m_workers.begin(); it_worker != m_workers.end(); it_worker++) {
    	CSLSGroup *worker = *it_worker;
    	if (worker) {
    		worker->reload();
    	}
    }
	return 0;
}

int  CSLSManager::check_invalid()
{
    std::list<CSLSGroup *>::iterator it;
    std::list<CSLSGroup *>::iterator it_erase;
    std::list<CSLSGroup *>::iterator it_end = m_workers.end();
    for ( it = m_workers.begin(); it != it_end; ) {
    	CSLSGroup *worker = *it;
    	it_erase = it;
    	it++;
    	if (NULL == worker) {
			m_workers.erase(it_erase);
			continue;
    	}
		if (worker->is_exit()) {
			sls_log(SLS_LOG_INFO, "[%p]CSLSManager::check_invalid, delete worker=%p.",
					this, worker);
			worker->stop();
			worker->uninit_epoll();
			delete worker;
            m_workers.erase(it_erase);
		}
    }

    if (m_workers.size() == 0)
        return SLS_OK;
    return SLS_ERROR;
}

void CSLSManager::get_stat_info(std::string &info_str)
{
    info_str += '[';

    std::list<CSLSGroup *>::iterator it;
    std::list<CSLSGroup *>::iterator it_end = m_workers.end();
    for ( it = m_workers.begin(); it != it_end; it++ ) {
    	CSLSGroup *worker = *it;
    	if (NULL != worker) {
            std::string worker_info;
    		worker->get_stat_info(worker_info);
            // add delimiter between JSON objects
            if (it != m_workers.begin() && !worker_info.empty()) info_str += ',';
            info_str += worker_info;
    	}
    }

    info_str += ']';
}

int  CSLSManager::stat_client_callback(void *p, HTTP_CALLBACK_TYPE type, void *v, void* context)
{
	CSLSManager *manager = (CSLSManager *)context;
	if (HCT_REQUEST_CONTENT == type) {
		std::string * p_response = (std::string *)v;
		manager->get_stat_info(*p_response);
	} else if (HCT_RESPONSE_END == type) {
		//response info maybe include info that server send client, such as reload cmd...
	} else {

	}
	return SLS_OK;
}



