
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


#include <errno.h>
#include <string.h>


#include "SLSPusher.hpp"
#include "SLSLog.hpp"

const char SLS_RELAY_STAT_INFO_BASE[] = "\
{\
\"port\": \"%d\",\
\"role\": \"%s\",\
\"pub_domain_app\": \"%s\",\
\"stream_name\": \"%s\",\
\"url\": \"%s\",\
\"remote_ip\": \"%s\",\
\"remote_port\": \"%d\",\
\"start_time\": \"%s\",\
\"kbitrate\":\
";

/**
 * CSLSPusher class implementation
 */

CSLSPusher::CSLSPusher()
{
    m_is_write             = 1;
    sprintf(m_role_name, "pusher");

}

CSLSPusher::~CSLSPusher()
{
    //release
}

int CSLSPusher::handler()
{
	return handler_write_data();
}

int   CSLSPusher::get_stat_base(char *stat_base)
{
    strcpy(stat_base, SLS_RELAY_STAT_INFO_BASE);
    return SLS_OK;
}


