
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
int              idle_streams_timeout;
SLS_CONF_DYNAMIC_DECLARE_END


/**
 * relay cmd declare
 */

SLS_CONF_CMD_DYNAMIC_DECLARE_BEGIN(relay)
SLS_SET_CONF(relay, string, type,                 "pull, push",  1, 31),
SLS_SET_CONF(relay, string, mode,                 "relay mode.", 1, 31),
SLS_SET_CONF(relay, string, upstreams,            "upstreams",   1, 1023),
SLS_SET_CONF(relay, int,    reconnect_interval,   "reconnect interval, unit s", 1, 3600),
SLS_SET_CONF(relay, int,    idle_streams_timeout, "idle streams timeout, unit s", -1, 3600),

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
