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

#ifndef _SLSMapPublisher_INCLUDE_
#define _SLSMapPublisher_INCLUDE_

#include <map>
#include <string>

#include "conf.hpp"
#include "SLSLock.hpp"
#include "SLSRole.hpp"

class CSLSMapPublisher
{
public:
	CSLSMapPublisher();
    virtual ~CSLSMapPublisher();

    void set_conf(std::string key, sls_conf_base_t * ca);
    void set_live_2_uplive(std::string strLive, std::string strUplive);
    int  set_push_2_pushlisher(std::string app_streamname, CSLSRole * role);
    int  remove(CSLSRole *role);
    void clear();

    std::string      get_uplive(std::string key_app);
    sls_conf_base_t *get_ca(std::string key_app);

    CSLSRole *       get_publisher(std::string strAppStreamName);


private:
    std::map<std::string, std::string>           m_map_live_2_uplive;   // 'hostname/live':'hostname/uplive'
    std::map<std::string, sls_conf_base_t*>      m_map_uplive_2_conf;   // 'hostname/uplive':sls_app_conf_t
    std::map<std::string, CSLSRole *>            m_map_push_2_pushlisher;    // 'hostname/uplive/steam_name':publisher'

    CSLSRWLock          m_rwclock;


};


#endif
