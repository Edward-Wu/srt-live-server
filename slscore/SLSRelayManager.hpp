
/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2019-2020 Edward.Wu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef _SLSRelayManager_INCLUDE_
#define _SLSRelayManager_INCLUDE_

#include <vector>
#include <string>

#include "SLSRelay.hpp"
#include "SLSMapPublisher.hpp"
#include "conf.hpp"
#include "SLSRoleList.hpp"

typedef struct SLS_RELAY_INFO {
    std::vector<std::string>      m_upstreams;
    char     m_type[32];
    int      m_mode;
    int      m_reconnect_interval;//unit: s
    int      m_idle_streams_timeout;//unit: s
};

/**
 * CSLSRelayManager
 */
class CSLSRelayManager
{
public :
	CSLSRelayManager();
    virtual ~CSLSRelayManager();

    virtual int start() = 0;
    virtual int reconnect(int64_t cur_tm_ms) = 0;

    virtual int  add_reconnect_stream(char* relay_url) = 0;

    void set_map_publisher(CSLSMapPublisher *publisher);
    void set_map_data(CSLSMapData *map_data);
    void set_role_list(CSLSRoleList *role_list);

    void set_relay_conf(SLS_RELAY_INFO *sri);
    void set_relay_info(const char *app_uplive, const char *stream_name);
    void set_listen_port(int port);

protected:
    CSLSMapPublisher    *m_map_publisher ;
    CSLSMapData         *m_map_data ;
    CSLSRoleList        *m_role_list;
    SLS_RELAY_INFO      *m_sri;
    int64_t              m_reconnect_begin_tm;//unit: ms
    int                  m_listen_port;

    char     m_app_uplive[1024];
    char     m_stream_name[1024];

    int connect(const char *url);
    int connect_hash();

    virtual CSLSRelay *create_relay() = 0;
    std::string get_hash_url();

    virtual int set_relay_param(CSLSRelay *relay) = 0;

};


#endif
