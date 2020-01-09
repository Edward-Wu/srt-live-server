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

    //role list
    m_list_role = new CSLSRoleList;
    sls_log(SLS_LOG_INFO, "[%p]CSLSManager::start, new m_list_role=%p.", this, m_list_role);

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
    m_map_data = new CSLSMapData[m_server_count];
    m_map_publisher = new CSLSMapPublisher[m_server_count];
    m_map_puller = new CSLSMapRelay[m_server_count];
    m_map_pusher = new CSLSMapRelay[m_server_count];

    //create listeners according config, delete by groups
    for (i = 0; i < m_server_count; i ++) {
    	CSLSListener * p = new CSLSListener();//deleted by groups
    	p->set_role_list(m_list_role);
        p->set_conf(conf);
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
    	m_vector_server.push_back(p);
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
        if (SLS_OK != p->init_epoll()) {
            sls_log(SLS_LOG_INFO, "[%p]CSLSManager::start, p->init_epoll failed.", this);
            return SLS_ERROR;
        }
        m_vector_worker.push_back(p);
        m_single_group  =p;

    } else {
        for (i = 0; i < m_worker_threads; i ++) {
            CSLSGroup * p = new CSLSGroup();
            p->set_worker_number(i);
            p->set_role_list(m_list_role);
            p->set_worker_connections(conf_srt->worker_connections);
            if (SLS_OK != p->init_epoll()) {
                sls_log(SLS_LOG_INFO, "[%p]CSLSManager::start, p->init_epoll failed.", this);
                return SLS_ERROR;
            }
            p->start();
            m_vector_worker.push_back(p);
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
    //stop groups
    sls_log(SLS_LOG_INFO, "[%p]CSLSManager::stop, release worker, count=%d.", this, m_vector_worker.size());
    for(i = 0; i < m_vector_worker.size(); i ++) {
    	CSLSGroup *p = m_vector_worker[i];
    	if (p) {
    		p->stop();
    		p->uninit_epoll();
    		delete p;
    		p = NULL;
    	}
    }
    m_vector_worker.clear();

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
	//close all old listeners, not to accept new client

	//start new listeners

	return 0;
}

