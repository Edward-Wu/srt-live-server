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

#ifndef _SLSMapRelay_INCLUDE_
#define _SLSMapRelay_INCLUDE_

#include <map>
#include <string>

#include "SLSRelayManager.hpp"
#include "SLSLock.hpp"

class CSLSMapRelay
{
public:
	CSLSMapRelay();
    virtual ~CSLSMapRelay();

    CSLSRelayManager *add_relay_manager(const char *app_uplive, const char *stream_name);
    void clear();

    int add_relay_conf(std::string app_uplive, sls_conf_relay_t * cr);
    SLS_RELAY_INFO *get_relay_conf(std::string app_uplive);

private:
    CSLSRWLock          m_rwclock;
    std::map<std::string, CSLSRelayManager *>   m_map_relay_manager;        //stream_name: relay_manager

    std::map<std::string, SLS_RELAY_INFO *>     m_map_relay_info;        //uplive: relay_conf_info
};


#endif
