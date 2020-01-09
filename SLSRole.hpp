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

#ifndef _SLSRole_INCLUDE_
#define _SLSRole_INCLUDE_

#include <map>


#include "SLSRole.hpp"
#include "SLSSrt.hpp"
#include "SLSMapData.hpp"
#include "conf.hpp"
#include "SLSLock.hpp"
#include "common.hpp"


enum SLS_ROLE_STATE {
    SLS_RS_UNINIT = 0,
    SLS_RS_INITED = 1,
	SLS_RS_IDLE_STREAM = 2,
    SLS_RS_INVALID = 3,
};

const int DATA_BUFF_SIZE = 100 * 1316;
/**
 * CSLSRole , the base of player, publisher and listener
 */
class CSLSRole
{
public :
	CSLSRole();
	virtual ~CSLSRole();


    virtual int init();
    virtual int uninit();
    virtual int handler();

    int         open(char *url);
    int         close();

    int         get_fd();
    int         set_eid(int eid);
    bool        is_write(){return m_is_write;};

    int         set_srt(CSLSSrt *srt);
    int         invalid_srt();

    int         write(const char *buf, int size);

    int         add_to_epoll(int eid);
    int         remove_from_epoll();
    int         get_state(int64_t cur_time_microsec = 0);
    int         get_sock_state();
    char      * get_role_name();

    void        set_conf(sls_conf_base_t *conf);
    void        set_map_data(char *map_key, CSLSMapData *map_data);

    void        set_idle_streams_timeout(int timeout);

    char      * get_streamid();
    bool        is_reconnect();

protected:
    CSLSSrt    * m_srt;
    bool         m_is_write;//listener: 0, publisher: 0, player: 1
    int64_t      m_invalid_begin_tm;
    int          m_idle_streams_timeout;//unit: s
    int          m_latency;//ms

    int          m_state;
    int          m_back_log;//maximum number of connections at the same time
    char         m_role_name[256];
    char         m_streamid[256];

    sls_conf_base_t   * m_conf;

    CSLSMapData       * m_map_data;
    char                m_map_data_key[1024];
    SLSRecycleArrayID   m_map_data_id;

    int handler_write_data();
    int handler_read_data();


    char          m_data[DATA_BUFF_SIZE];
    int           m_data_len;
    int           m_data_pos;

    bool          m_need_reconnect;

private:

};


#endif
