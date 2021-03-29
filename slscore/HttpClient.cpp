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

#include "HttpClient.hpp"
#include "SLSLog.hpp"

#define HTTP_HEADER_SIZE        1024

//	 POST /sls?method=stat HTTP/1.1
#define HTTP_REQUEST_HEADER_ACCEPT         "Accept: text/html, */*\r\n"
//   Accept: text/html, */*
#define HTTP_REQUEST_HEADER_USER_AGENT     "User-Agent: srt-live-server\r\n"
#define HTTP_REQUEST_HEADER_CONTENT_TYPE   "Content-Type: application/x-www-form-urlencoded\r\n"
//   Host: localhost:8080
//   Content-Length: 15
#define HTTP_REQUEST_HEADER_CONNECTION     "Connection: Keep-Alive\r\n"
//
#define HTTP_REQUEST_HEADER_CACHE_CONTROL  "Cache-Control: no-cache\r\n"
//   Cache-Control: no-cache

/**
 * CHttpClient class implementation
 */

CHttpClient::CHttpClient()
{
    memset(m_url, 0, URL_MAX_LEN);
    memset(m_uri, 0, URL_MAX_LEN);
    memset(m_remote_host, 0, 1024);
    memset(m_http_method, 0, 32);


    m_remote_port  = 80;
    m_out_pos      = 0;
    m_out_data_len = 0;
    m_begin_tm_ms  = 0;
    m_end_tm_ms    = 0;
    m_timeout      = 5;//default 5s
    m_interval     = 0; //unit s
    m_id           = SLS_HTTP_INVALID_CLIENT_ID;

    m_callback         = NULL;
    m_callback_context = NULL;

    m_response_info.m_response_content_length    = -1;
    sprintf(m_role_name, "http_client");
    sprintf(m_http_method, "POST");

}

CHttpClient::~CHttpClient()
{
    close();
}

int  CHttpClient::open(const char *url, const char *method, int interval)
{
    m_begin_tm_ms = sls_gettime_ms();
    int ret = SLS_OK;
	strcpy(m_url, url);
	if (NULL == m_url) {
        sls_log(SLS_LOG_INFO, "[%p]CHttpClient::open, failed, m_url is NULL.", this);
		goto FUNC_END;
	}
	if (strlen(m_url) == 0) {
        sls_log(SLS_LOG_INFO, "[%p]CHttpClient::open, failed, m_url='%s'.", this, m_url);
		goto FUNC_END;
	}
	if (NULL != method && strlen(method) > 0) {
		sprintf(m_http_method, method);
	}

	m_interval = interval;

    ret = parse_url();
    if (SLS_OK != ret) {
        sls_log(SLS_LOG_INFO, "[%p]CHttpClient::open, parse_url failed.", this);
		goto FUNC_END;
    }

    char resolved_ip[128];
    if (SLS_OK != sls_gethostbyname(m_remote_host, resolved_ip)) {
        sls_log(SLS_LOG_INFO, "[%p]CHttpClient::open, failed to resolve, remote_host='%s', remote_port=%d.",
        		this, m_remote_host, m_remote_port);
		goto FUNC_END;
    }
    ret = CTCPRole::open(resolved_ip, m_remote_port);
    if (SLS_OK != ret) {
        sls_log(SLS_LOG_INFO, "[%p]CHttpClient::open, failed, remote_host='%s', remote_port=%d.",
        		this, m_remote_host, m_remote_port);
		goto FUNC_END;
    }
	//clear response
    m_response_info.m_response_header.clear();
    m_response_info.m_response_content.clear();
    m_response_info.m_response_content_length   = -1;
	m_end_tm_ms = 0;
	m_out_array.clear();

    ret = generate_http_request();
    if (SLS_OK != ret) {
        sls_log(SLS_LOG_INFO, "[%p]CHttpClient::open, generate_http_request failed, remote_host='%s', remote_port=%d.",
        		this, m_remote_host, m_remote_port);
		goto FUNC_END;
    }
    //send data
    handler();
FUNC_END:
    if (NULL != m_callback) {
		m_callback(this, HCT_OPEN, &ret, m_callback_context);
    }
    return ret;
}

int  CHttpClient::close()
{
	int ret = SLS_OK;
    sls_log(SLS_LOG_TRACE, "[%p]CHttpClient::close, m_url='%s', m_response_content_length=%d.",
    		this, m_url, m_response_info.m_response_content_length);
    ret = CTCPRole::close();
	if (NULL != m_callback) {
		m_callback(this, HCT_CLOSE, &ret, m_callback_context);
	}
    m_response_info.m_response_header.clear();
    m_response_info.m_response_content.clear();
    m_response_info.m_response_content_length   = -1;
	m_end_tm_ms = 0;
	m_out_array.clear();
	return ret;
}


int  CHttpClient::reopen()
{
	 close();
     return open(m_url, m_http_method, m_interval);
}


int  CHttpClient::check_timeout(int64_t cur_tm_ms)
{
	if (0 == m_fd) {
		return SLS_OK;
	}

	if (m_end_tm_ms > 0) {
		if (m_response_info.m_response_content_length == m_response_info.m_response_content.length())
		    return SLS_ERROR;
		return SLS_OK;
	}
	if (cur_tm_ms == 0){
		cur_tm_ms = sls_gettime_ms();
	}

    int d = cur_tm_ms - m_begin_tm_ms;
    if ( d < m_timeout*1000) {
    	return SLS_ERROR;
    }
    m_end_tm_ms = cur_tm_ms;
	if (NULL != m_callback) {
		m_callback(this, HCT_RESPONSE_END, &m_response_info, m_callback_context);
	}
	sls_log(SLS_LOG_INFO, "[%p]CHttpClient::check_timeout, ok, url='%s', http_method='%s', content_len=%d, m_response_content_length=%d.",
    		this, m_url, m_http_method, m_response_info.m_response_content.length(), m_response_info.m_response_content_length);

	return SLS_OK;
}

int  CHttpClient::check_repeat(int64_t cur_tm_ms)
{
	if (m_interval <= 0) {
		//no repeat
		return SLS_ERROR;
	}
	if (cur_tm_ms == 0){
		cur_tm_ms = sls_gettime_ms();
	}

    int d = cur_tm_ms - m_begin_tm_ms;
    if ( d < m_interval*1000) {
    	return SLS_ERROR;
    }
	return SLS_OK;
}

void  CHttpClient::set_stage_callback(HTTP_CALLBACK callback, void *context)
{
	m_callback         = callback;
	m_callback_context = context;
}

void CHttpClient::set_timeout(int v)
{
	m_timeout = v;
}

void CHttpClient::set_id(uint32_t client_id)
{
	m_id  = client_id;
}

uint32_t CHttpClient::get_id()
{
	return m_id;
}

HTTP_RESPONSE_INFO *CHttpClient::get_response_info()
{
	return &m_response_info;
}

int CHttpClient::check_finished()
{
	if (m_response_info.m_response_content_length == m_response_info.m_response_content.length()) {
		return SLS_OK;
	}
	if (0 == m_fd) {
		return SLS_OK;
	}
   return SLS_ERROR;
}

int CHttpClient::parse_http_response(std::string &response)
{
	if (m_response_info.m_response_header.size() > 0) {
		m_response_info.m_response_content += response;
		if (m_response_info.m_response_content_length == m_response_info.m_response_content.length()) {
        	sls_log(SLS_LOG_INFO, "[%p]CHttpClient::parse_http_response, finished, url='%s', http_method='%s', content_len=%d.",
            		this, m_url, m_http_method, m_response_info.m_response_content.length());
			m_end_tm_ms = sls_gettime_ms();
			if (NULL != m_callback) {
				m_callback(this, HCT_RESPONSE_END, &m_response_info, m_callback_context);
			}
		}
	    return SLS_OK;
	}

    std::vector<std::string> response_parts;
    sls_split_string(response, "\r\n\r\n", response_parts, 1);
    if (response_parts.size() == 0) {
        sls_log(SLS_LOG_TRACE, "[%p]CHttpClient::parse_http_response, failed, sls_split_string, not found '\r\n\r\n', continue.",
        		this);
    	return SLS_ERROR;
    }

    sls_split_string(response_parts[0], "\r\n", m_response_info.m_response_header);
    if (m_response_info.m_response_header.size() > 0) {
        //HTTP/1.1 200 OK
    	std::vector<std::string> parts;
    	sls_split_string(m_response_info.m_response_header[0], " ", parts);
    	if (parts.size() == 3) {
    		m_response_info.m_response_code = parts[1];//
        	sls_log(SLS_LOG_INFO, "[%p]CHttpClient::parse_http_response, m_response_code:'%s', url='%s', http_method='%s'.",
            		this, m_response_info.m_response_code.c_str(), m_url, m_http_method);
    	}

    	//Content-Length
		std::string dst = std::string("Content-Length:");
		std::string content_length = sls_find_string(m_response_info.m_response_header, dst);
		if (content_length.length() > 0) {
	    	sls_split_string(content_length, ":", parts, 1);
		    if (parts.size() == 2) {
				m_response_info.m_response_content_length = atoi(parts[1].c_str());
		    	sls_log(SLS_LOG_INFO, "[%p]CHttpClient::parse_http_response, m_response_content_length=%d.",
		        		this, m_response_info.m_response_content_length);
		    }
		}

    }

    if (response_parts.size() == 2) {
    	m_response_info.m_response_content = response_parts[1];
    	if (m_response_info.m_response_content_length == m_response_info.m_response_content.length()) {
    		m_end_tm_ms = sls_gettime_ms();
    		if (NULL != m_callback) {
    			m_callback(this, HCT_RESPONSE_END, &m_response_info, m_callback_context);
    		}
        	sls_log(SLS_LOG_INFO, "[%p]CHttpClient::parse_http_response, finished, url='%s', http_method='%s', content_len=%d.",
            		this, m_url, m_http_method, m_response_info.m_response_content.length());
    		return SLS_OK;
    	}
    }

    return SLS_OK;
}

int CHttpClient::recv()
{
	int ret = SLS_OK;
	//response data
	char data[HTTP_DATA_SIZE+1] = {0};
	int data_size = HTTP_DATA_SIZE;
	std::string http_response;
	while (true) {
		//just handle string type data
        int n = read(data, data_size);
        if (n <= 0) {
        	break;
        }
        data[n] = 0;
        http_response += std::string(data);
				usleep(10);
	}
	if (http_response.length() > 0) {
		//parse the response
        ret = parse_http_response(http_response);
        if (SLS_OK == check_finished()) {
            sls_log(SLS_LOG_INFO, "[%p]CHttpClient::recv, finished.",
            		this);
        }
	}
    return ret;
}

int CHttpClient::send()
{
	int ret = SLS_OK;
	while (true)
	{
		if (m_out_data_len <= 0) {
    	    if (m_out_array.count() == 0) {
    	    	break;
    	    }
		    m_out_data_len = m_out_array.get(m_out_data, HTTP_DATA_SIZE);
    	    if (m_out_data_len <= 0){
                m_out_pos       = 0;
                m_out_data_len  = 0;
 	            break;
    	    }
    	    m_out_pos = 0;
		}

        int n = write((char*)m_out_data + m_out_pos, m_out_data_len);
        if (n <= 0) {
        	break;
        }
        ret += n;
        if (n < m_out_data_len) {
        	//remainder, maybe net is busy, break
            m_out_pos      += n;
            m_out_data_len -= n;
            break;
        } else if (n > m_out_data_len) {
            m_out_pos       = 0;
            m_out_data_len  = 0;
            sls_log(SLS_LOG_WARNING, "[%p]CHttpClient::send, write failed, m_url='%s', m_out_data_len=%d, but n=%d.",
            		this, m_out_data_len, n);
            break;
        } else {
        	//continue;
            m_out_pos       = 0;
        	m_out_data_len  = 0;
        	continue;
        }
	}
    return ret;
}

int CHttpClient::handler(DATA_PARAM *p)
{
	int ret = SLS_OK;
    if (p->readable) {
    	ret = recv();
    } else if (p->writable) {
    	ret = send();
    }
	return ret;
}

int CHttpClient::handler()
{
    int ret = 0;
    int timeout = 10;//10ms

    int sfd = m_fd;
    fd_set read_fds;
    fd_set write_fds;
    struct timeval tout;
    int max_sockfd;

    FD_ZERO(&read_fds);
    FD_SET(sfd, &read_fds);
    FD_ZERO(&write_fds);
    FD_SET(sfd, &write_fds);
    max_sockfd = sfd;

    if (!m_valid)
        return 0;
    tout.tv_sec = 0;
    tout.tv_usec = timeout * 1000;
    ret = select (max_sockfd + 1, &read_fds, &write_fds, NULL, &tout);
    if (ret == 0) {
        return SLS_OK;
    }
    if (ret == -1) {
        sls_log(SLS_LOG_INFO, "[%p]CHttpClient::handler, ret=%d, errno=%d, err='%s'",
                this, ret, errno, strerror(errno));
        return SLS_ERROR;
    }

    //write is ready?
    if(FD_ISSET(sfd, &write_fds)){
        send();
    }
    //read is ready?
    if(FD_ISSET(sfd, &read_fds)){
        recv();
    }
    return SLS_OK;
}



int CHttpClient::parse_url()
{
    //http://hostname:port/sls?method=stat or http://hostname:port/sls?method=on_connect&srt_url=srt://....
    if (NULL == m_url || strlen(m_url) == 0) {
    	return SLS_ERROR;
    }

    char url[1024] = {0};
    strcpy(url, m_url);

    char *p = url;
    //protocal
    p = strchr(url, ':');
    if (!p) {
    	sls_log(SLS_LOG_INFO, "[%p]CHttpClient::parse_url, no ':', url='%s'", this, m_url);
        return SLS_ERROR;
    }
    p[0] = 0x00;
    if (strcmp(url, "http") != 0) {
    	sls_log(SLS_LOG_INFO, "[%p]CHttpClient::parse_url, not 'http' prefix, url='%s'.",
    			this, m_url);
        return SLS_ERROR;
    }
    p += 3;//skip '://'

    //hostname:port
    char *p_tmp = strchr(p, ':');
    if (p_tmp) {
        p_tmp[0] = 0x00;
        strcpy(m_remote_host, p);
        p = p_tmp + 1;
        p_tmp = strchr(p, '/');
        if (p_tmp) {
            p_tmp[0] = 0x00;
            m_remote_port = atoi(p);
            p_tmp[0] = '/';
            p = p_tmp;
        }else{
        	sls_log(SLS_LOG_INFO, "[%p]CHttpClient::parse_url, no '/' in '%s'.",
        			this, m_url);
            m_remote_port = atoi(p);
            p = p_tmp;
        }
    } else {
    	m_remote_port = 80;
        p_tmp = strchr(p, '/');
        if (p_tmp){
            p_tmp[0] = 0x00;
            strcpy(m_remote_host, p);
            p_tmp[0] = '/';
            p = p_tmp;
		}else{
        	sls_log(SLS_LOG_INFO, "[%p]CHttpClient::parse_url, no '/' in '%s'.",
        			this, m_url);
            p = p_tmp;
        }
    }
    if (NULL != p)
        strcpy(m_uri, p);
    return SLS_OK;
}

int CHttpClient::write_http_header(int data_len)
{
	std:string http_header;
	char data[HTTP_HEADER_SIZE] = {0};

	if (strcmp(m_http_method, "GET") != 0 && strcmp(m_http_method, "POST") != 0) {
        sls_log(SLS_LOG_INFO, "[%p]CHttpClient::write_http_header, wrong method='%s'.",
            this, m_http_method);
		return SLS_ERROR;
	}
	sprintf(data, "%s %s HTTP/1.1\r\n", m_http_method, m_uri);
	http_header = std::string(data);

	http_header += std::string(HTTP_REQUEST_HEADER_ACCEPT);
	http_header += std::string(HTTP_REQUEST_HEADER_USER_AGENT);
	http_header += std::string(HTTP_REQUEST_HEADER_CONTENT_TYPE);

	sprintf(data, "Host: %s\r\n", m_remote_host);
	http_header += std::string(data);

	if (data_len > 0) {
	    sprintf(data, "Content-Length: %d\r\n", data_len);
	    http_header += std::string(data);
	}

	http_header += std::string(HTTP_REQUEST_HEADER_CONNECTION);
	http_header += std::string(HTTP_REQUEST_HEADER_CACHE_CONTROL);
	http_header += std::string("\r\n");

	//int ret = write(http_header.c_str(), http_header.length());
	int len = http_header.length();
	int ret = m_out_array.put((const uint8_t*)(http_header.c_str()), len);
	if (len != ret) {
        sls_log(SLS_LOG_INFO, "[%p]CHttpClient::write_http_header, failed, m_out_array.put len=%d, ret=%d, url='%s', http_method='%s'.",
            this, len, ret, m_url, m_http_method);
        return SLS_ERROR;
	}
	return ret;
}

int CHttpClient::write_string(std::string *str)
{
	if (!m_valid) {
		return SLS_ERROR;
	}
	if (!str) {
		return SLS_ERROR;
	}

	int len = str->length();
	if (len > 0) {
		int ret = m_out_array.put(((const uint8_t*)str->c_str()), len);
		if (len != ret) {
			sls_log(SLS_LOG_INFO, "[%p]CHttpClient::write_string, failed, m_out_array.put len=%d, ret=%d.",
				this, len, ret);
			return SLS_ERROR;
		}
	}
	return SLS_OK;
}

void CHttpClient::get_request_data()
{
	m_request_data.clear();
	if (m_callback) {
		m_callback(this, HCT_REQUEST_CONTENT, &m_request_data, m_callback_context);
	}
}

int CHttpClient::generate_http_request()
{
	int ret = SLS_ERROR;
	get_request_data();
    int post_data_len = m_request_data.length();

    ret = write_http_header(post_data_len);
    if (0 >= ret) {
        sls_log(SLS_LOG_INFO, "[%p]CHttpClient::generate_http_request, write_get_header failed, remote_host='%s', remote_port=%d.",
        		this, m_remote_host, m_remote_port);
    	return ret;
    }

    if (post_data_len > 0) {
        //post content
        ret = write_string(&m_request_data);
        sls_log(SLS_LOG_INFO, "[%p]CHttpClient::generate_http_request, write_post_content, ret=%d, remote_host='%s', remote_port=%d.",
        		this, ret, m_remote_host, m_remote_port);
    }
    sls_log(SLS_LOG_INFO, "[%p]CHttpClient::generate_http_request, ok, m_url='%s', content len=%d.",
    		this, m_url, post_data_len);
    return SLS_OK;
}
