
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


#ifndef _SLSRole_INCLUDE_
#define _SLSRole_INCLUDE_

#include <map>


#include "SLSRole.hpp"
#include "SLSSrt.hpp"
#include "SLSMapData.hpp"
#include "conf.hpp"
#include "SLSLock.hpp"
#include "common.hpp"
#include "HttpClient.hpp"


enum SLS_ROLE_STATE {
    SLS_RS_UNINIT = 0,
    SLS_RS_INITED = 1,
    SLS_RS_INVALID = 2,
};

const int DATA_BUFF_SIZE    = 100 * 1316;
const int UNLIMITED_TIMEOUT = -1;
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
    virtual int handler();

    int         open(char *url);
    int         close();

    int         get_fd();
    int         set_eid(int eid);
    bool        is_write(){return m_is_write;};

    int         set_srt(CSLSSrt *srt);
    int         invalid_srt();

    int         write(const char *buf, int size);

    int         add_to_epoll(int eid);
    int         remove_from_epoll();
    int         get_state(int64_t cur_time_microsec = 0);
    int         get_sock_state();
    char      * get_role_name();

    void        set_conf(sls_conf_base_t *conf);
    void        set_map_data(char *map_key, CSLSMapData *map_data);

    void        set_idle_streams_timeout(int timeout);
    bool        check_idle_streams_duration(int64_t cur_time_ms = 0);

    char      * get_streamid();
    bool        is_reconnect();

    void        set_stat_info_base(std::string &v);
    virtual std::string get_stat_info();
    void        update_stat_info();
    virtual int get_peer_info(char *peer_name, int &peer_port);

    void        set_http_url(const char *http_url);
    int         on_connect();
    int         on_close();
    int         check_http_client();
    int         check_http_passed();

    void        set_record_hls_path(const char *hls_path);
protected:
    CSLSSrt     *m_srt;
    bool         m_is_write;//listener: 0, publisher: 0, player: 1
    int64_t      m_invalid_begin_tm;//
    int64_t      m_stat_bitrate_last_tm;//
    int          m_stat_bitrate_interval ;//ms
    int          m_stat_bitrate_datacount ;
    int          m_kbitrate;//kb
    int          m_idle_streams_timeout;//unit: s, -1: unlimited
    int          m_latency;//ms

    int          m_state;
    int          m_back_log;//maximum number of connections at the same time
    int          m_port;
    char         m_peer_ip[IP_MAX_LEN];
    int          m_peer_port;
    char         m_role_name[STR_MAX_LEN];
    char         m_streamid[URL_MAX_LEN];
    char         m_http_url[URL_MAX_LEN];
    bool         m_http_passed;

    sls_conf_base_t    *m_conf;
    CSLSMapData        *m_map_data;
    char                m_map_data_key[URL_MAX_LEN];
    SLSRecycleArrayID   m_map_data_id;

    char          m_data[DATA_BUFF_SIZE];
    int           m_data_len;
    int           m_data_pos;
    bool          m_need_reconnect;
    std::string   m_stat_info_base;
    CHttpClient  *m_http_client;

    char          m_record_hls[SHORT_STR_MAX_LEN];
    int           m_record_hls_ts_fd ;
    char          m_record_hls_ts_filename[URL_MAX_LEN];
    int           m_record_hls_vod_fd ;
    char          m_record_hls_vod_filename[URL_MAX_LEN];
    char          m_record_hls_path[URL_MAX_LEN];
    int64_t       m_record_hls_begin_tm_ms ;
    int           m_record_hls_segment_duration;
    float         m_record_hls_target_duration;

    int  handler_write_data();
    int  handler_read_data(int64_t *last_read_time=NULL);
    void record_data2hls(char *data, int len);
    void check_hls_file();
    void close_hls_file();
private:

};


#endif
