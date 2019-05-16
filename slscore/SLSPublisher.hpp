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

#ifndef _SLSPublisher_INCLUDE_
#define _SLSPublisher_INCLUDE_

#include <list>

#include "SLSRole.hpp"
#include "SLSRoleList.hpp"

/**
 * sls_conf_app_t
 */
SLS_CONF_DYNAMIC_DECLARE_BEGIN(app)
char             app_player[1024];
char             app_publisher[1024];
int              publisher_exit_delay;
SLS_CONF_DYNAMIC_DECLARE_END

/**
 * app cmd declare
 */
SLS_CONF_CMD_DYNAMIC_DECLARE_BEGIN(app)
SLS_SET_CONF(app, string, app_player,           "live", 1, 1023),
SLS_SET_CONF(app, string, app_publisher,        "uplive", 1, 1023),
SLS_SET_CONF(app, int,    publisher_exit_delay, "delay exit time, unit second.", 1, 300),
SLS_CONF_CMD_DYNAMIC_DECLARE_END


/**
 * CSLSPublisher
 */
class CSLSPublisher: public CSLSRole
{
public :
    CSLSPublisher();
    ~CSLSPublisher();

    virtual int init();

    int add_player(CSLSRole * player);
    int get_player_count();

    virtual int  handler();
    virtual void clear(bool invalid, bool del);

    CSLSRoleList * get_player_list();
    void           set_player_list(CSLSRoleList * players);

private:
    CSLSRoleList   * m_list_player;
};


#endif
