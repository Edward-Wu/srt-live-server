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

#include "SLSRole.hpp"
#include "SLSSrt.hpp"
#include "SLSRecycleList.hpp"
#include "conf.hpp"

enum SLS_ROLE_STATE {
    SLS_RS_UNINIT = 0,
    SLS_RS_INITED = 1,
    SLS_RS_INVALID = 2,
};
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

    virtual int  handler();
    virtual void clear(bool invalid, bool del);

    int         get_fd();
    int         set_eid(int eid);
    bool        is_write(){return m_is_write;};

    int         set_srt(CSLSSrt *srt);
    int         invalid_srt();

    int         write(const char *buf, int size);
    int         push_data(const char *buf, int size);

    int         add_to_epoll(int eid);
    int         remove_from_epoll();
    int         get_state();
    int         get_sock_state();
    char      * get_role_name();
    void        set_parent(void * parent);
    void      * get_parent();

    void        set_conf(sls_conf_base_t * conf);


protected:
    CSLSSrt    * m_srt;
    bool         m_is_write;//listener: 0, publisher: 0, player: 1
    bool         m_inited;
    int64_t      m_invalid_tm;
    int          m_exit_delay;//unit: s
    int          m_latency;//ms

    int          m_state;
    int          m_back_log;//maximum number of connections at the same time
    void       * m_parent;
    char         m_role_name[256];

    sls_conf_base_t   * m_conf;
    CSLSRecycleList     m_list_data;

private:

};


#endif
