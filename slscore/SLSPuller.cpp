
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


#include "SLSPuller.hpp"
#include "SLSLog.hpp"
#include "SLSMapRelay.hpp"

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
 * CSLSPuller class implementation
 */

CSLSPuller::CSLSPuller()
{
    m_is_write             = 0;
    sprintf(m_role_name, "puller");

}

int CSLSPuller::uninit()
{
	int ret = SLS_ERROR;
	if (NULL != m_map_publisher) {
		ret = m_map_publisher->remove(this);
		sls_log(SLS_LOG_INFO, "[%p]CSLSPuller::uninit, removed relay from m_map_publisher, ret=%d.",
				this, ret);
	}
	if (m_map_data) {
        ret = m_map_data->remove(m_map_data_key);
		sls_log(SLS_LOG_INFO, "[%p]CSLSPuller::uninit, removed relay from m_map_data, ret=%d.",
				this, ret);
	}
	return CSLSRelay::uninit();

}

CSLSPuller::~CSLSPuller()
{
    //release
}

int CSLSPuller::handler()
{
	int64_t last_read_time = 0;
	int ret = handler_read_data(&last_read_time);
	if (ret >= 0) {
		//*check if there is any player?
		if (-1 == m_idle_streams_timeout) {
			return ret;
		}
		int64_t cur_time = sls_gettime_ms();
		if (cur_time - last_read_time >= (m_idle_streams_timeout*1000)) {
	        sls_log(SLS_LOG_INFO, "[%p]CSLSPuller::handler, no any reader for m_idle_streams_timeout=%ds, last_read_time=%lld, close puller.",
	        		this, m_idle_streams_timeout, last_read_time);
			m_state = SLS_RS_INVALID;
			invalid_srt();
	        return SLS_ERROR;
		}
		//*/
	}
	return ret;
}

int   CSLSPuller::get_stat_base(char *stat_base)
{
    strcpy(stat_base, SLS_RELAY_STAT_INFO_BASE);
    return SLS_OK;
}



