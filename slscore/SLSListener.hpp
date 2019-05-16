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

#ifndef _SLSListener_INCLUDE_
#define _SLSListener_INCLUDE_

#include <map>
#include <string>

#include "SLSRole.hpp"
#include "SLSRoleList.hpp"
#include "SLSPublisher.hpp"
#include "conf.hpp"

/**
 * server conf
 */
SLS_CONF_DYNAMIC_DECLARE_BEGIN(server)
char             domain_player[1024];
char             domain_publisher[1024];
int              listen;
int              backlog;
int              latency;
SLS_CONF_DYNAMIC_DECLARE_END

/**
 * sls_conf_server_t
 */
SLS_CONF_CMD_DYNAMIC_DECLARE_BEGIN(server)
SLS_SET_CONF(server, string, domain_player,      "play domain", 1,    1023),
SLS_SET_CONF(server, string, domain_publisher,   "", 1,    1023),
SLS_SET_CONF(server, int,    listen,             "listen port", 1024, 10000),
SLS_SET_CONF(server, int,    backlog,            "how many sockets may be allowed to wait until they are accepted", 1,    1024),
SLS_SET_CONF(server, int,    latency,            "latency.", 1, 300),
SLS_CONF_CMD_DYNAMIC_DECLARE_END


/**
 * SLSListener
 */
class CSLSListener : public CSLSRole
{
public :
	CSLSListener();
    ~CSLSListener();

    int         init();
    int         uninit();


    virtual int start();
    virtual int stop();

    virtual int  handler();
    virtual void clear(bool invalid, bool del);

    void set_role_list(CSLSRoleList *list_role);

private:
    CSLSRoleList      * m_list_role;

    std::map<std::string, std::string>      m_map_live_2_uplive;   // 'hostname/live':'hostname/uplive'
    std::map<std::string, sls_conf_app_t*>  m_map_uplive_2_conf;   // 'hostname/uplive':sls_app_conf_t

    std::map<std::string, CSLSPublisher *>  m_map_push_2_pushlisher;    // 'hostname/uplive/steam_name':publisher'

    void check_invalid_publisher();
    int  init_conf_app();


};


#endif
