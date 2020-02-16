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

#ifndef _HttpClient_INCLUDE_
#define _HttpClient_INCLUDE_

#include <list>

#include "TCPRole.hpp"
#include "SLSArray.hpp"
#include "common.hpp"

enum HTTP_CALLBACK_TYPE {
	HCT_OPEN  = 0,
	HCT_CLOSE,
	HCT_RESPONSE_END,
	HCT_REQUEST_CONTENT,
};

#define HTTP_DATA_SIZE             4096
#define SLS_HTTP_INVALID_CLIENT_ID 0
#define HTTP_RESPONSE_CODE_200 "200"

typedef int (*HTTP_CALLBACK)(void *p, HTTP_CALLBACK_TYPE type, void *v, void* context);
typedef struct HTTP_RESPONSE_INFO {
	std::vector<std::string> m_response_header;
	std::string              m_response_code;
	std::string              m_response_content;
	int                      m_response_content_length;
};
/**
 * CHttpClient
 */
class CHttpClient : public CTCPRole
{
public :
	CHttpClient();
	virtual ~CHttpClient();

	int         open(const char *url, const char *method=NULL, int interval=0);
	int         close();
	int         reopen();

	int         send();
	int         recv();
    virtual int handler(DATA_PARAM *p);
    virtual int handler();

    void        set_id(uint32_t client_id);
    void        set_timeout(int v);
    void        set_stage_callback(HTTP_CALLBACK callback, void *context);

    int         check_timeout(int64_t cur_tm_ms=0);
    int         check_repeat(int64_t cur_tm_ms=0);
    int         check_finished();

    uint32_t            get_id();
    HTTP_RESPONSE_INFO *get_response_info();
protected:
    uint32_t    m_id;
    char        m_url[URL_MAX_LEN];
    char        m_uri[URL_MAX_LEN];
	char        m_remote_host[URL_MAX_LEN];
	char        m_http_method[32];
	int         m_remote_port;
	int64_t     m_begin_tm_ms;
	int64_t     m_end_tm_ms;
	int         m_timeout;//s
	int         m_interval;//s

	CSLSArray   m_out_array;
    char        m_out_data[HTTP_DATA_SIZE];
    int         m_out_pos      ;
    int         m_out_data_len ;

    std::string m_request_data;

    HTTP_RESPONSE_INFO       m_response_info;
    /*
	std::vector<std::string> m_response_header;
	std::string              m_response_code;
	std::string              m_response_content;
	int                      m_response_content_length;
    */
	HTTP_CALLBACK            m_callback;
	void                    *m_callback_context;

	int         parse_url();
	int         write_http_header(int data_len);
	int         write_string(std::string *str);
	int         parse_http_response(std::string &response);
    virtual int generate_http_request();
    void        get_request_data();

private:

};


#endif
