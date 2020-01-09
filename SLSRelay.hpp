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

#ifndef _SLSRelay_INCLUDE_
#define _SLSRelay_INCLUDE_

#include <list>

#include "SLSRole.hpp"
#include "SLSMapPublisher.hpp"

/**
 * sls_conf_relay_t
 */

SLS_CONF_DYNAMIC_DECLARE_BEGIN(relay)
char             type[32];
char             mode[32];
char             upstreams[1024];
int              reconnect_interval;
SLS_CONF_DYNAMIC_DECLARE_END


/**
 * relay cmd declare
 */

SLS_CONF_CMD_DYNAMIC_DECLARE_BEGIN(relay)
SLS_SET_CONF(relay, string, type,               "pull, push",  1, 31),
SLS_SET_CONF(relay, string, mode,               "relay mode.", 1, 31),
SLS_SET_CONF(relay, string, upstreams,          "upstreams",   1, 1023),
SLS_SET_CONF(relay, int,    reconnect_interval, "reconnect interval, unit s", 1, 3600),

SLS_CONF_CMD_DYNAMIC_DECLARE_END


enum SLS_PULL_MODE {
	SLS_PM_LOOP  = 0,
	SLS_PM_HASH  = 1,
	SLS_PM_ALL   = 2,
};

/**
 * CSLSRelay
 */
class CSLSRelay: public CSLSRole
{
public :
	CSLSRelay();
    virtual ~CSLSRelay();

    virtual int uninit();

    void  set_map_publisher(CSLSMapPublisher *publisher);
    void  set_relay_manager(void *relay_manager);
    void *get_relay_manager();

    int   open(const char *url);
    int   close();

    char *get_url();

protected:
    char         m_url[1024];
    char         m_upstream[1024];

    CSLSMapPublisher  *m_map_publisher;
    void              *m_relay_manager;

    int          parse_url(char* url, char * ip, int& port, char * streamid);

};


#endif
