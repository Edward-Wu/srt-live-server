
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


#ifndef _SLSPublisher_INCLUDE_
#define _SLSPublisher_INCLUDE_

#include <list>

#include "SLSRole.hpp"
#include "SLSRoleList.hpp"
#include "SLSMapPublisher.hpp"

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
    virtual ~CSLSPublisher();

    void set_map_publisher(CSLSMapPublisher * publisher);

    virtual int init();
    virtual int uninit();

    virtual int  handler();
private:
    CSLSMapPublisher  * m_map_publisher;

};


#endif
