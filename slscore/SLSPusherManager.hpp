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

#ifndef _SLSPusherManager_INCLUDE_
#define _SLSPusherManager_INCLUDE_

#include <vector>
#include <string>

#include "SLSRelayManager.hpp"
#include "conf.hpp"

/**
 * CSLSPusherManager
 */
class CSLSPusherManager: public CSLSRelayManager
{
public :
	CSLSPusherManager();
    virtual ~CSLSPusherManager();

    virtual int start();
    virtual int add_reconnect_stream(char* relay_url);
    virtual int reconnect(int64_t cur_tm_ms);


private:
    int connect_all();
    virtual CSLSRelay *create_relay();
    virtual int set_relay_param(CSLSRelay *relay);
    int check_relay_param();
    int reconnect_all(int64_t cur_tm_ms);

    CSLSRWLock          m_rwclock;
    std::map<std::string, int64_t> m_map_reconnect_relay;//relay:timeout

};


#endif
