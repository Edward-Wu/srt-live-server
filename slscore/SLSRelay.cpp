
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


#include "SLSRelay.hpp"
#include "SLSLog.hpp"
#include "SLSRelayManager.hpp"

/**
 * relay conf
 */
SLS_CONF_DYNAMIC_IMPLEMENT(relay)

/**
 * CSLSRelay class implementation
 */

CSLSRelay::CSLSRelay()
{
    m_is_write             = 0;
    memset(m_url, 0, URL_MAX_LEN);
    memset(m_server_ip, 0, IP_MAX_LEN);

    m_server_port          = 0;
    m_map_publisher        = NULL;
    m_relay_manager        = NULL;
    m_need_reconnect       = true;

    sprintf(m_role_name, "relay");
}

CSLSRelay::~CSLSRelay()
{
    //release
}

int CSLSRelay::uninit()
{
	// for reconnect
	if (NULL != m_relay_manager) {
		((CSLSRelayManager*)m_relay_manager)->add_reconnect_stream(m_url);
		sls_log(SLS_LOG_INFO, "[%p]CSLSRelay::uninit, add_reconnect_stream, m_url=%s.",
				this, m_url);
	}

	return CSLSRole::uninit();
}

void CSLSRelay::set_map_publisher(CSLSMapPublisher *map_publisher)
{
	m_map_publisher = map_publisher;
}

void CSLSRelay::set_relay_manager(void *relay_manager)
{
	m_relay_manager = relay_manager;
}

void *CSLSRelay::get_relay_manager()
{
	return m_relay_manager;
}

int CSLSRelay::parse_url(char* url, char *host_name, int& port, char * streamid)
{
    //
    if (strlen(url) == 0) {
    	sls_log(SLS_LOG_INFO, "[%p]CSLSRelay::parse_url='%s', url must like 'srt://hostname:port?streamid=your_stream_id' or 'srt://hostname:port/app/stream_name'.",
    			this, url);
        return SLS_ERROR;
    }
    sprintf(m_url, "%s", url);

	char * p = url;
    //protocal
    p = strchr(url, ':');
    if (!p) {
    	sls_log(SLS_LOG_INFO, "[%p]CSLSRelay::parse_url, no ':', url='%s', url must like 'srt://hostname:port?streamid=your_stream_id' or 'srt://hostname:port/app/stream_name'.",
    			this, m_url);
        return SLS_ERROR;
    }
    p[0] = 0x00;
    if (strcmp(url, "srt") != 0) {
    	sls_log(SLS_LOG_INFO, "[%p]CSLSRelay::parse_url, not 'srt' prefix, url='%s', url must like 'srt://hostname:port?streamid=your_stream_id' or 'srt://hostname:port/app/stream_name'.",
    			this, m_url);
        return SLS_ERROR;
    }
    p += 3;//skip 'srt://'

    //hostname:port
    char * p_tmp = strchr(p, ':');
    if (p_tmp) {
        p_tmp[0] = 0x00;
        strcpy(host_name, p);
        p = p_tmp + 1;
    }
    else {
    	sls_log(SLS_LOG_INFO, "[%p]CSLSRelay::parse_url, not 'hostname:port', url='%s', url must like 'srt://hostname:port?streamid=your_stream_id' or 'srt://hostname:port/app/stream_name'.",
    			this, m_url);
        return SLS_ERROR;
    }

    //hostname
	bool b_streamid = false;
    p_tmp = strchr(p, '?');
    if (!p_tmp) {
    	sls_log(SLS_LOG_INFO, "[%p]CSLSRelay::parse_url='%s', no '?' param, come on.",
    			this, m_url);
        p_tmp = strchr(p, '/');//app
        if (!p_tmp) {//app
        	sls_log(SLS_LOG_INFO, "[%p]CSLSRelay::parse_url, url='%s', url must like 'srt://hostname:port?streamid=your_stream_id' or 'srt://hostname:port/app/stream_name'.",
        			this, m_url);
        	return SLS_ERROR;
        }
		p_tmp ++;
		p_tmp = strchr(p_tmp, '/');//stream
		if (!p_tmp) {
			sls_log(SLS_LOG_INFO, "[%p]CSLSRelay::parse_url, url='%s', url must like 'srt://hostname:port?streamid=your_stream_id' or 'srt://hostname:port/app/stream_name'.",
					this, m_url);
			return SLS_ERROR;
		}
        p_tmp ++;
		if (strlen(p_tmp)==0) {
			sls_log(SLS_LOG_INFO, "[%p]CSLSRelay::parse_url, url='%s', url must like 'srt://hostname:port?streamid=your_stream_id' or 'srt://hostname:port/app/stream_name'.",
					this, m_url);
			return SLS_ERROR;
		}
		p_tmp ++;
		p_tmp = strchr(p_tmp, '/');//redundant
		if (p_tmp) {
			sls_log(SLS_LOG_INFO, "[%p]CSLSRelay::parse_url, url='%s', url must like 'srt://hostname:port?streamid=your_stream_id' or 'srt://hostname:port/app/stream_name'.",
					this, m_url);
			return SLS_ERROR;
		}
    }else{
    	b_streamid = true;
    }

    if (b_streamid) {
        p_tmp[0] = 0;
        port = atoi(p);
        p = p_tmp + 1;
		//streamid
		p_tmp = strchr(p, '=');
		if (!p) {
			sls_log(SLS_LOG_INFO, "[%p]CSLSRelay::parse_url='%s', no 'stream=', url must like 'hostname:port?streamid=your_stream_id'.", this, url);
			return SLS_ERROR;
		}
		p_tmp[0] = 0;
		if (strcmp(p, "streamid") != 0) {
			sls_log(SLS_LOG_INFO, "[%p]CSLSRelay::parse_url='%s', no 'stream', url must like 'hostname:port?streamid=your_stream_id'.", this, url);
			return SLS_ERROR;
		}
		p = p_tmp + 1;
	    strcpy(streamid, p);
    } else {
    	p_tmp = m_url + strlen("srt://");
		p_tmp = strchr(p, '/');
    	sprintf(streamid, "%s%s", host_name, p_tmp);
    }
    return SLS_OK;

}

int CSLSRelay::open(const char * srt_url) {
    int yes = 1;
    int no = 0;
    char host_name[128] = "192.168.31.56";//test
    char server_ip[128] = "";
    int server_port = 8080;
    char streamid[1024] = "uplive.sls.net/live/1234";//test
    char url[1024] = {0};
    int latency = 10;

    strcpy(m_url, srt_url);
    strcpy(url, srt_url);

    //init listener
    if (NULL != m_srt)
    {
    	sls_log(SLS_LOG_ERROR, "[%p]CSLSRelay::open, failure, url='%s', m_srt = %p, not NULL.", this, url, m_srt);
        return SLS_ERROR;
    }

    //parse url
    if (SLS_OK != parse_url(url, host_name, server_port, streamid)){
        return SLS_ERROR;
    }
	sls_log(SLS_LOG_INFO, "[%p]CSLSRelay::open, parse_url ok, url='%s'.", this, m_url);

    if (strlen(streamid) == 0)
    {
    	sls_log(SLS_LOG_ERROR, "[%p]CSLSRelay::open, url='%s', no 'stream', url must like 'hostname:port?streamid=your_stream_id'.", this, m_url);
        return SLS_ERROR;
    }

    SRTSOCKET fd = srt_socket(AF_INET, SOCK_DGRAM, 0);

    int status = srt_setsockopt(fd, 0, SRTO_SNDSYN, &no, sizeof no); // for async write
    if (status == SRT_ERROR) {
    	sls_log(SLS_LOG_ERROR, "[%p]CSLSRelay::open, srt_setsockopt SRTO_SNDSYN failure. err=%s.", this, srt_getlasterror_str());
        return SLS_ERROR;
    }

    status = srt_setsockopt(fd, 0, SRTO_RCVSYN, &no, sizeof no); // for async read
    if (status == SRT_ERROR) {
    	sls_log(SLS_LOG_ERROR, "[%p]CSLSRelay::open, srt_setsockopt SRTO_SNDSYN failure. err=%s.", this, srt_getlasterror_str());
        return SLS_ERROR;
    }

//    srt_setsockflag(fd, SRTO_SENDER, &m_is_write, sizeof m_is_write);
/*
    status = srt_setsockopt(fd, 0, SRTO_TSBPDMODE, &yes, sizeof yes); //
    if (status == SRT_ERROR) {
    	sls_log(SLS_LOG_ERROR, "[%p]CSLSRelay::open, srt_setsockopt SRTO_TSBPDMODE failure. err=%s.", this, srt_getlasterror_str());
        return SLS_ERROR;
    }
    /*
    SRT_TRANSTYPE tt = SRTT_LIVE;
    status = srt_setsockopt(fd, 0, SRTO_TRANSTYPE, &tt, sizeof tt);
    if (status == SRT_ERROR) {
    	sls_log(SLS_LOG_ERROR, "[%p]CSLSRelay::open, srt_setsockopt SRTO_TRANSTYPE failure. err=%s.", this, srt_getlasterror_str());
        return SLS_ERROR;
    }
*/

    if (srt_setsockopt(fd, 0, SRTO_STREAMID, streamid, strlen(streamid)) < 0) {
    	sls_log(SLS_LOG_ERROR, "[%p]CSLSRelay::open, srt_setsockopt SRTO_STREAMID failure. err=%s.", this, srt_getlasterror_str());
        return SLS_ERROR;
    }

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(server_port);

    sls_gethostbyname(host_name, server_ip);
    if (inet_pton(AF_INET, server_ip, &sa.sin_addr) != 1) {
    	sls_log(SLS_LOG_ERROR, "[%p]CSLSRelay::open, inet_pton failure. server_ip=%s, server_port=%d.", this, server_ip, server_port);
        return SLS_ERROR;
    }

    struct sockaddr *psa = (struct sockaddr *) &sa;
    status = srt_connect(fd, psa, sizeof sa);
    if (status == SRT_ERROR) {
    	sls_log(SLS_LOG_ERROR, "[%p]CSLSRelay::open, srt_connect failure. server_ip=%s, server_port=%d.", this, server_ip, server_port);
        return SLS_ERROR;
    }
    m_srt = new CSLSSrt();
    m_srt->libsrt_set_fd(fd);
    strcpy(m_server_ip, server_ip);
    m_server_port = server_port;
    return status;
}

int CSLSRelay::close()
{
	int ret = SLS_OK;
	if (m_srt) {
    	sls_log(SLS_LOG_INFO, "[%p]CSLSRelay::close, ok, url='%s'.", this, m_url);
		ret = m_srt->libsrt_close();
		delete m_srt;
		m_srt = NULL;
	}
    return ret;
}

char *CSLSRelay::get_url()
{
    return m_url;
}

int CSLSRelay::get_peer_info(char *peer_name, int &peer_port)
{
	strcpy(peer_name, m_server_ip);
	peer_port = m_server_port;
	return SLS_OK;
}

int  CSLSRelay::get_stat_base(char *stat_base)
{
    return SLS_OK;
}

