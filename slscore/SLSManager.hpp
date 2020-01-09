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

#ifndef _SLSManager_INCLUDE_
#define _SLSManager_INCLUDE_

#include <vector>

#include "SLSRole.hpp"
#include "SLSRoleList.hpp"
#include "SLSGroup.hpp"
#include "SLSListener.hpp"
#include "conf.hpp"
#include "SLSMapData.hpp"
#include "SLSMapRelay.hpp"

/**
 * srt conf declare
 */
SLS_CONF_DYNAMIC_DECLARE_BEGIN(srt)
char             log_file[1024];
char             log_level[1024];
int              worker_threads;
int              worker_connections;
SLS_CONF_DYNAMIC_DECLARE_END


/**
 * srt cmd declare
 */
SLS_CONF_CMD_DYNAMIC_DECLARE_BEGIN(srt)
SLS_SET_CONF(srt, string, log_file,           "save log file name.", 1, 1023),
SLS_SET_CONF(srt, string, log_level,          "log level", 1, 1023),
SLS_SET_CONF(srt, int,    worker_threads,     "count of worker thread, if 0, only main thread.", 0, 100),
SLS_SET_CONF(srt, int,    worker_connections, "", 1, 1024),
SLS_CONF_CMD_DYNAMIC_DECLARE_END


/**
 * CSLSManager , manage players, publishers and listener
 */
class CSLSManager
{
public :
	CSLSManager();
	virtual ~CSLSManager();


    int start();
    int stop();
    int reload();
    int single_thread_handler();

    bool is_single_thread();

private:
    std::vector<CSLSListener *>   m_vector_server;
    int                           m_server_count;
    CSLSMapData                 * m_map_data;
    CSLSMapPublisher            * m_map_publisher;
    CSLSMapRelay                * m_map_puller;
    CSLSMapRelay                * m_map_pusher;

    std::vector<CSLSGroup    *>   m_vector_worker;
    int                           m_worker_threads;

    CSLSRoleList * m_list_role;
    CSLSGroup    * m_single_group;

};


#endif
