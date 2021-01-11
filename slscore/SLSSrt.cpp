
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
#include <map>
#include <string>
#include <memory.h>

#include "SLSSrt.hpp"
#include "SLSLog.hpp"
#include "SLSLock.hpp"

/**
 * CSLSSrt class implementation
 */

bool CSLSSrt::m_inited = false;


CSLSSrt::CSLSSrt()
{
    memset(&m_sc, 0x0, sizeof(m_sc));
    memset(m_http_url_passphrase, 0, URL_MAX_LEN);
    m_sc.port = 8000;//for test
    m_sc.fd   = 0;
    m_sc.eid  = 0;
    m_sc.latency = 20;

    m_sc.backlog = 1024;
    memset(m_peer_name, 0, sizeof(m_peer_name));
    m_peer_port = 0;
}
CSLSSrt::~CSLSSrt()
{
}

int CSLSSrt::libsrt_init()
{
    if (m_inited)
        return SLS_OK;

    if (srt_startup() < 0) {
        return SLSERROR_UNKNOWN;
    }
    m_inited = true;
	return SLS_OK;
}

int CSLSSrt::libsrt_uninit()
{
    if (!m_inited)
        return SLS_OK;
    srt_cleanup();
	return SLS_OK;
}

int CSLSSrt::libsrt_epoll_create()
{
	return srt_epoll_create();
}

void CSLSSrt::libsrt_epoll_release(int eid)
{
	srt_epoll_release(eid);
}

void CSLSSrt::libsrt_print_error_info()
{
    /**
SRTS_BROKEN: The socket was connected, but the connection was broken
SRTS_CLOSING: The socket may still be open and active, but closing is requested, so no further operations will be accepted (active operations will be completed before closing)
SRTS_CLOSED: The socket has been closed, but not yet removed by the GC thread
SRTS_NONEXIST:
     */

#define set_error_map(k)\
        map_error[k] = std::string(#k);

    char szBuf[1024] = {0};
    std::map<int, std::string> map_error;

    set_error_map(SRTS_INIT);
    set_error_map(SRTS_OPENED);
    set_error_map(SRTS_LISTENING);
    set_error_map(SRTS_CONNECTING);
    set_error_map(SRTS_CONNECTED);
    set_error_map(SRTS_BROKEN);
    set_error_map(SRTS_CLOSING);
    set_error_map(SRTS_CLOSED);
    set_error_map(SRTS_NONEXIST);

    set_error_map(SRT_ENOCONN);
    set_error_map(SRT_ECONNLOST);
    set_error_map(SRT_EINVALMSGAPI);
    set_error_map(SRT_EINVALBUFFERAPI);
    set_error_map(SRT_EASYNCSND);
    set_error_map(SRT_ETIMEOUT);
    set_error_map(SRT_EPEERERR);

    printf("--------srt error--------\n");
    std::map<int, std::string>::iterator it;
    for(it=map_error.begin(); it!=map_error.end(); ++it) {
        sprintf(szBuf, "%d: %s\n", it->first, it->second.c_str());
        printf(szBuf);
    }
    printf("----------end------------\n");
    map_error.clear();


}

int CSLSSrt::libsrt_neterrno()
{
    int err = srt_getlasterror(NULL);
    sls_log(SLS_LOG_ERROR, "CSLSSrt::libsrt_neterrno, err=%d, %s.",  err, srt_getlasterror_str());
    return err;
}

void CSLSSrt::libsrt_set_context(SRTContext *sc)
{
    m_sc = *sc;
}

void CSLSSrt::libsrt_set_latency(int latency)
{
    m_sc.latency = latency;
}

int CSLSSrt::libsrt_setup(int port)
{
    struct addrinfo hints = { 0 }, *ai;
    int fd = -1;
    int ret;
    char portstr[10];
    int timeout = 10;//ms
    int eid;
    SRTContext *s = &m_sc;

    m_sc.port = port;

    hints.ai_family = AF_INET;//AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    snprintf(portstr, sizeof(portstr), "%d", s->port);
    hints.ai_flags |= AI_PASSIVE;
    ret = getaddrinfo(s->hostname[0] ? s->hostname : NULL, portstr, &hints, &ai);
    if (ret) {
        sls_log(SLS_LOG_ERROR,
               "[%p]CSLSSrt::libsrt_setup, Failed to resolve hostname %s: %s.",
               this, s->hostname, gai_strerror(ret));
        return ret;
    }

    fd = srt_socket(ai->ai_family, ai->ai_socktype, 0);
    if (fd < 0) {
        ret = libsrt_neterrno();
        freeaddrinfo(ai);
        return ret;
    }

/*
    if (libsrt_setsockopt(h, fd, SRTO_STREAMID, "SRTO_STREAMID", sc->streamid, strlen(s->streamid)) < 0) {
        ret = libsrt_neterrno();
        freeaddrinfo(ai);
        return ret;
    }
*/

    /* Set the socket's send or receive buffer sizes, if specified.
       If unspecified or setting fails, system default is used. */
    if (s->latency > 0) {
        srt_setsockopt(fd, SOL_SOCKET, SRTO_LATENCY, &s->latency, sizeof (s->latency));
    }
    if (s->recv_buffer_size > 0) {
        srt_setsockopt(fd, SOL_SOCKET, SRTO_UDP_RCVBUF, &s->recv_buffer_size, sizeof (s->recv_buffer_size));
    }
    if (s->send_buffer_size > 0) {
        srt_setsockopt(fd, SOL_SOCKET, SRTO_UDP_SNDBUF, &s->send_buffer_size, sizeof (s->send_buffer_size));
    }
    if (s->reuse) {
        if (srt_setsockopt(fd, SOL_SOCKET, SRTO_REUSEADDR, &s->reuse, sizeof(s->reuse)))
            sls_log(SLS_LOG_WARNING, "[%p]CSLSSrt::libsrt_setup, setsockopt(SRTO_REUSEADDR) failed.", this);
    }

    ret = srt_bind(fd, ai->ai_addr, ai->ai_addrlen);
    if (ret) {
        srt_close(fd);
        freeaddrinfo(ai);
        return libsrt_neterrno();
    }

    s->fd = fd;

    freeaddrinfo(ai);
    sls_log(SLS_LOG_INFO, "[%p]CSLSSrt::libsrt_setup, fd=%d.", this, fd);

    return SLS_OK;
}

void CSLSSrt::setup_passphrase_endpoint(const char *http_url_passphrase) {
    if (NULL == http_url_passphrase || strlen(http_url_passphrase) == 0) {
	    return ;
	}
	strcpy(m_http_url_passphrase, http_url_passphrase);
}

char *CSLSSrt::get_passphrase_endpoint() {
    return m_http_url_passphrase;
}

srt_listen_callback_fn handle_srt_listen_callback;
int handle_srt_listen_callback(void* opaque, SRTSOCKET ns, int hs_version, const struct sockaddr* peeraddr, const char* streamid) {
    CSLSSrt* tmp = (CSLSSrt*)(opaque);
    char * pass_url = tmp->get_passphrase_endpoint();

    if (NULL != pass_url && sizeof(pass_url) > 0) {
        CHttpClient* m_http_client = new CHttpClient;

        char get_passphrase_url[URL_MAX_LEN] = {0};
        sprintf(get_passphrase_url, "%s?streamid=%s",
                pass_url, streamid);

        m_http_client->open(get_passphrase_url);

        while (true) {
            m_http_client->handler();
            if (SLS_OK == m_http_client->check_finished() || SLS_OK == m_http_client->check_timeout()) {
                break;
            }
            usleep(10);
        }

        HTTP_RESPONSE_INFO * re = m_http_client->get_response_info();

        if (NULL == re) {
            return SLS_ERROR;
        }

        int ret = strcmp(re->m_response_code.c_str(), HTTP_RESPONSE_CODE_200);
        if (0 == ret) {
            const char* response = re->m_response_content.c_str();
            sls_log(SLS_LOG_INFO, "[%p]CSLSSrt::listen_callback, http finished, %s, http_url='%s', response_code=%s, response='%s'.",
                            tmp, "m_role_name", get_passphrase_url, re->m_response_code.c_str(), response);

            int ret = SLS_OK;

            if (NULL != response && strlen(response) > 0) {
                ret = srt_setsockopt(ns, SOL_SOCKET, SRTO_PASSPHRASE, response, strlen(response));
                if (SLS_OK != ret) {
                    sls_log(SLS_LOG_INFO, "[%p]CSLSSrt::listen_callback, failed to set passphrase, response='%d'.",
                            tmp, ret);
                }
            }

            m_http_client->close();
            delete m_http_client;
            m_http_client = NULL;

            return ret;
        } else {
            sls_log(SLS_LOG_INFO, "[%p]CSLSSrt::listen_callback, http refused, invalid %s http_url='%s', response_code=%s, response='%s'.",
                            tmp, "m_role_name", get_passphrase_url, re->m_response_code.c_str(), re->m_response_content.c_str());
            m_http_client->close();
            delete m_http_client;
            m_http_client = NULL;
            return SLS_ERROR;
        }
    }

    return 0;
}

int CSLSSrt::libsrt_listen(int backlog)
{
    m_sc.backlog = backlog;
    if (NULL != m_http_url_passphrase && strlen(m_http_url_passphrase) > 0) {
        (void)srt_listen_callback(m_sc.fd, &handle_srt_listen_callback, this);
    }
    int ret = srt_listen(m_sc.fd, backlog);
    if (ret)
        return libsrt_neterrno();

    sls_log(SLS_LOG_INFO, "[%p]CSLSSrt::libsrt_listen, ok, fd=%d, at port=%d.", this, m_sc.fd, m_sc.port);
    return SLS_OK;
}

int CSLSSrt::libsrt_accept()
{
    struct sockaddr_in scl;
    int sclen = sizeof(scl);
    char ip[30] = {0};
    struct sockaddr_in * addrtmp;

    int new_sock = srt_accept(m_sc.fd, (struct sockaddr*)&scl, &sclen);//NULL, NULL);//(sockaddr*)&scl, &sclen);
    if (new_sock == SRT_INVALID_SOCK) {
        int err_no = libsrt_neterrno();
        sls_log(SLS_LOG_INFO, "[%p]CSLSSrt::libsrt_accept failed, sock=%d, error_no=%d,.",
                   this, m_sc.fd, err_no);
        return SLS_ERROR;
    }
    addrtmp = (struct sockaddr_in *)&scl;
    inet_ntop(AF_INET, &addrtmp->sin_addr, ip, sizeof(ip));
    sls_log(SLS_LOG_INFO, "[%p]CSLSSrt::libsrt_accept ok, new sock=%d, %s:%d.",
               this, new_sock, ip, ntohs(addrtmp->sin_port));

    return new_sock;
}

int CSLSSrt::libsrt_close()
{
    if (m_sc.fd) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSSrt::libsrt_close, fd=%d.", this, m_sc.fd);
      srt_close(m_sc.fd);
      m_sc.fd = 0;
    }
    return SLS_OK;
}

int CSLSSrt::libsrt_set_fd(int fd)
{
    libsrt_close();
    m_sc.fd = fd;
    return SLS_OK;
}


int CSLSSrt::libsrt_get_fd()
{
    return m_sc.fd;
}

int CSLSSrt::libsrt_set_eid(int eid)
{
    m_sc.eid = eid;
    return SLS_OK;
}


int CSLSSrt::libsrt_getsockopt(SRT_SOCKOPT optname, const char * optnamestr, void * optval, int * optlen)
{
    if (srt_getsockopt(m_sc.fd, 0, optname, optval, optlen) < 0) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSSrt::libsrt_getsockopt, failed to get option %s on socket: %s.", this, optnamestr, srt_getlasterror_str());
        return SLSERROR(EIO);
    }
    return 0;
}

int CSLSSrt::libsrt_setsockopt(SRT_SOCKOPT optname, const char * optnamestr, const void * optval, int optlen)
{
    if (srt_setsockopt(m_sc.fd, 0, optname, optval, optlen) < 0) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSSrt::libsrt_setsockopt, failed to set option %s on socket: %s\n", this, optnamestr, srt_getlasterror_str());
        return SLSERROR(EIO);
    }
    return 0;
}


int CSLSSrt::libsrt_socket_nonblock(int enable)
{
    int ret = srt_setsockopt(m_sc.fd, 0, SRTO_SNDSYN, &enable, sizeof(enable));
    if (ret < 0)
        return ret;
    return srt_setsockopt(m_sc.fd, 0, SRTO_RCVSYN, &enable, sizeof(enable));
}

int CSLSSrt::libsrt_split_sid(char *sid, char *host, char *app, char *name)
{
    int i = 0;
    char *p, *p1 ;
    p1 = sid;

    //host
    p = strchr(p1, '/');
    if (p) {
        strncpy(host, (const char *)p1, p - p1);
        p1 = p+1;
    } else {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSSrt::libsrt_split_sid, sid='%s' is not as host/app/name.", this, sid);
        return -1;
    }
    //app
    p = strchr(p1, '/');
    if (p) {
        strncpy(app, (const char *)p1, p - p1);
        p1 = p+1;
    } else {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSSrt::libsrt_split_sid, sid='%s' is not as host/app/name.", this, sid);
        return -1;
    }

    strcpy(name, (const char *)p1);

    return 0;
}

int CSLSSrt::libsrt_read(char *buf, int size)
{
    int ret;
    ret = srt_recvmsg(m_sc.fd, buf, size);
    if (ret < 0) {
        int err_no = libsrt_neterrno();
        sls_log(SLS_LOG_WARNING, "[%p]CSLSSrt::libsrt_read failed, sock=%d, ret=%d, err_no=%d.",
        		this, m_sc.fd, ret, err_no);
    }
    return ret;
}

int CSLSSrt::libsrt_write(const char *buf, int size)
{
    int ret;
    ret = srt_sendmsg(m_sc.fd, buf, size, -1, 0);
    if (ret < 0) {//SRTS_BROKEN
        int err_no = libsrt_neterrno();
        sls_log(SLS_LOG_WARNING, "[%p]CSLSSrt::libsrt_write failed, sock=%d, ret=%d, errno=%d.",
                this, m_sc.fd, ret, err_no);
    }
    return ret;
}

int CSLSSrt::libsrt_add_to_epoll(int eid, bool write)
{
    int ret = SLS_OK;
    int fd = m_sc.fd;
    int modes = write ? SRT_EPOLL_OUT : SRT_EPOLL_IN;

    if (!eid) {
        sls_log(SLS_LOG_ERROR, "CSLSSrt::libsrt_add_to_epoll failed, m_eid=%d.", eid);
        return SLS_ERROR;
    }

    modes |= SRT_EPOLL_ERR;
    ret = srt_epoll_add_usock(eid, fd, &modes);
    if (ret < 0) {
        sls_log(SLS_LOG_ERROR, "CSLSSrt::libsrt_add_to_epoll, srt_epoll_add_usock failed, m_eid=%d, fd=%d, modes=%d.",
                eid, fd, modes);
        return libsrt_neterrno();
    }
    return ret;
}

int CSLSSrt::libsrt_remove_from_epoll()
{
    int ret = SLS_OK;
    int fd = m_sc.fd;
    int eid = m_sc.eid;

    if (!eid) {
        sls_log(SLS_LOG_ERROR, "CSLSSrt::remove_from_epoll failed, m_eid=%d.", eid);
        return SLS_ERROR;
    }

    ret = srt_epoll_remove_usock(eid, fd);
    if (ret < 0) {
        sls_log(SLS_LOG_ERROR, "CSLSSrt::remove_from_epoll, srt_epoll_remove_usock failed, m_eid=%d, fd=%d.",
                eid, fd);
        return libsrt_neterrno();
    }
    return ret;
}

int  CSLSSrt::libsrt_getsockstate()
{
    return srt_getsockstate(m_sc.fd);
}


int CSLSSrt::libsrt_getpeeraddr(char * peer_name, int& port)
{
    int ret = SLS_ERROR;
    struct sockaddr_in peer_addr;
    int peer_addr_len = sizeof(peer_addr);

    if (strlen(m_peer_name) == 0 || m_peer_port == 0) {
        ret = srt_getpeername(m_sc.fd, (struct sockaddr *)&peer_addr, &peer_addr_len);
        if (0 == ret) {
            inet_ntop(AF_INET, &peer_addr.sin_addr, m_peer_name, sizeof(m_peer_name));
            m_peer_port = ntohs(peer_addr.sin_port);

            strcpy(peer_name, m_peer_name);
            port = m_peer_port;
            ret = SLS_OK;
        }
    } else {
        strcpy(peer_name, m_peer_name);
        port = m_peer_port;
        ret = SLS_OK;
    }
    return ret;
}

