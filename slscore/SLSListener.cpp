/*
 * This file is part of SLS Live Server.
 *
 * SLS Live Server is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * SLS Live Server is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with SLS Live Server;
 * if not, please contact with the author: Edward.Wu(edward_email@126.com)
 */

#include <errno.h>
#include <string.h>
#include <vector>

#include "SLSListener.hpp"
#include "SLSLog.hpp"
#include "SLSPublisher.hpp"
#include "SLSPlayer.hpp"

/**
 * server conf
 */
SLS_CONF_DYNAMIC_IMPLEMENT(server)

/**
 * CSLSListener class implementation
 */

CSLSListener::CSLSListener()
{
    m_conf      = NULL;
    m_back_log  = 1024;
    m_is_write  = 0;

    sprintf(m_role_name, "listener");
}
CSLSListener::~CSLSListener()
{
}

int CSLSListener::init()
{
	int ret = 0;

	//read stream info form config file
	//...
    //for test

    m_back_log  = 1024;

    //test end

    return CSLSRole::init();
}

int CSLSListener::uninit()
{
    stop();
    m_map_live_2_uplive.clear();
    return CSLSRole::uninit();
}

void CSLSListener::set_role_list(CSLSRoleList *list_role)
{
	m_list_role = list_role;
}


int CSLSListener::init_conf_app()
{
    string strLive;
    string strUplive;
    string strLiveDomain;
    string strUpliveDomain;
    string strTemp;
    vector<string> domain_players;
    sls_conf_server_t * conf_server;


    if (!m_conf) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::init_conf_app failed, conf is null.", this);
        return SLS_ERROR;
    }
    conf_server = (sls_conf_server_t *)m_conf;

    //domain
    domain_players = sls_conf_string_split(string(conf_server->domain_player), string(" "));
    if (domain_players.size() == 0) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::init_conf_app, wrong domain_player='%s'.", this, conf_server->domain_player);
        return SLS_ERROR;
    }
    strUpliveDomain = conf_server->domain_publisher;
    if (strUpliveDomain.length() == 0) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::init_conf_app, wrong domain_publisher='%s'.", this, conf_server->domain_publisher);
        return SLS_ERROR;
    }
    sls_conf_app_t * conf_app = (sls_conf_app_t *)conf_server->child;
    if (!conf_app) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::init_conf_app, no app conf info.", this);
        return SLS_ERROR;
    }

    int app_count = sls_conf_get_conf_count(conf_app);
    sls_conf_app_t * ca = conf_app;
    for (int i = 0; i < app_count; i ++) {
        strUplive = ca->app_publisher;
        if (strUplive.length() == 0) {
            sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::init_conf_app, wrong app_publisher='%s', domain_publisher='%s'.",
                    this, strUplive.c_str(), strUpliveDomain.c_str());
            return SLS_ERROR;
        }
        strUplive = strUpliveDomain + "/" + strUplive;
        m_map_uplive_2_conf[strUplive] = ca;
        sls_log(SLS_LOG_INFO, "[%p]CSLSListener::init_conf_app, add app push '%s'.",
                this, strUplive.c_str());

        strLive = ca->app_player;
        if (strLive.length() == 0) {
            sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::init_conf_app, wrong app_player='%s', domain_publisher='%s'.",
                    this, strLive.c_str(), strUpliveDomain.c_str());
            return SLS_ERROR;
        }

        for (int j = 0; j < domain_players.size(); j ++) {
            strLiveDomain = domain_players[j];
            strTemp =strLiveDomain + "/" + strLive;
            if (strUplive == strTemp) {
                sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::init_conf_app faild, domain/uplive='%s' and domain/live='%s' must not be equal.",
                        this, strUplive.c_str(), strTemp.c_str());
                return SLS_ERROR;
            }
            m_map_live_2_uplive[strTemp]  = strUplive;
            sls_log(SLS_LOG_INFO, "[%p]CSLSListener::init_conf_app, add app live='%s', app push='%s'.",
                    this, strTemp.c_str(), strUplive.c_str());
        }
        ca = (sls_conf_app_t *)ca->sibling;
    }
    return SLS_OK;

}

int CSLSListener::start()
{
	int ret = 0;
    std::string strLive;
    std::string strUplive;
    std::string strLiveDomain;
    std::string strUpliveDomain;


	if (!m_conf) {
	    sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::start failed, conf is null.", this);
	    return SLS_ERROR;
	}
    sls_log(SLS_LOG_INFO, "[%p]CSLSListener::start...", this);

    ret = init_conf_app();
    if (SLS_OK != ret) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::start, init_conf_app failed.", this);
        return SLS_ERROR;
    }

    //init listener
    if (NULL == m_srt)
        m_srt = new CSLSSrt();

    int port = ((sls_conf_server_t*)m_conf)->listen;
    ret = m_srt->libsrt_setup(port);
    if (SLS_OK != ret) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::start, libsrt_setup failure.", this);
        return ret;
    }
    sls_log(SLS_LOG_INFO, "[%p]CSLSListener::start, libsrt_setup ok.", this);

    int latency = ((sls_conf_server_t*)m_conf)->latency;
    if (latency > 0) {
        ret = m_srt->libsrt_setsockopt(SRTO_LATENCY, "SRTO_LATENCY",  &latency, sizeof (latency));
        if (SLS_OK != ret) {
            sls_log(SLS_LOG_INFO, "[%p]CSLSListener::start, libsrt_setsockopt failure.", this);
            return ret;
        }
    }

    ret = m_srt->libsrt_listen(m_back_log);
    if (SLS_OK != ret) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSListener::start, libsrt_listen failure.", this);
        return ret;
    }
    //
    if (NULL == m_list_role) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSListener::start, m_roleList is null.", this);
        return ret;
    }

    m_list_role->push(this);

	return ret;
}

int CSLSListener::stop()
{
	int ret = 0;
    sls_log(SLS_LOG_INFO, "[%p]CSLSListener::stop.", this);
    clear(false, false);
	return ret;
}


int CSLSListener::handler()
{
	int ret = SLS_OK;
	int fd_client = 0;
	CSLSSrt *srt = NULL;
	char sid[1024] = {0};
	int  sid_size = sizeof(sid);
	char host_name[1024] = {0};
	char app_name[1024] = {0};
	char stream_name[1024] = {0};
	char key_app[1024] = {0};
	char key_stream_name[1024] = {0};
    char tmp[1024] = {0};
    char peer_name[256] = {0};
    int  peer_port = 0;
    int  client_count = 0;

    CSLSPublisher * publisher = NULL;


    //check invalid publisher
    check_invalid_publisher();

    fd_client = m_srt->libsrt_accept();
    if (ret < 0) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::handler, srt_accept failed, fd=%d.", this, get_fd());
        CSLSSrt::libsrt_neterrno();
        return client_count;
    }
    client_count = 1;

    //check publisher or player
	srt = new CSLSSrt;
	srt->libsrt_set_fd(fd_client);
    ret = srt->libsrt_getpeeraddr(peer_name, peer_port);
    if (ret != 0) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::handler, libsrt_getpeeraddr failed, fd=%d.", this, srt->libsrt_get_fd());
        srt->libsrt_close();
        delete srt;
        return client_count;
    }

    sls_log(SLS_LOG_INFO, "[%p]CSLSListener::handler, new client[%s:%d], fd=%d.", this, peer_name, peer_port, fd_client);

    if (0 != srt->libsrt_getsockopt(SRTO_STREAMID, "SRTO_STREAMID", &sid, &sid_size)) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::handler, [%s:%d], fd=%d, get streamid info failed.",
                this, peer_name, peer_port, srt->libsrt_get_fd());
    	srt->libsrt_close();
        delete srt;
    	return client_count;
    }

    if (0 != srt->libsrt_split_sid(sid, host_name, app_name, stream_name)) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::handler, [%s:%d], parse sid='%s' failed.", this, peer_name, peer_port, sid);
    	srt->libsrt_close();
    	delete srt;
    	return client_count;
    }
    sls_log(SLS_LOG_INFO, "[%p]CSLSListener::handler, [%s:%d], sid '%s/%s/%s'",
            this, peer_name, peer_port, host_name, app_name, stream_name);

    // app exist?
    sprintf(key_app, "%s/%s", host_name, app_name);

    bool b_player = false;
    std::string app_uplive = "";//player?
    sls_conf_app_t * ca = NULL;
    std::map<std::string, std::string>::iterator it;
    it = m_map_live_2_uplive.find(key_app);
    if (it == m_map_live_2_uplive.end()) {
        std::map<std::string, sls_conf_app_t *>::iterator it;
        it = m_map_uplive_2_conf.find(key_app);
        if (it == m_map_uplive_2_conf.end()) {
            sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::handler, [%s:%d], domain/app='%s' not config, refused.",
                    this, peer_name, peer_port, key_app);
            srt->libsrt_close();
            delete srt;
            return client_count;
        } else {
            //publisher
            app_uplive = key_app;
            ca = it->second;
        }
    } else {
        //player
        app_uplive = it->second;
        ca = m_map_uplive_2_conf[app_uplive];
        b_player = true;
    }

    //stream publisher exist?
    sprintf(key_stream_name, "%s/%s", app_uplive.c_str(), stream_name);
    std::map<std::string, CSLSPublisher *>::iterator item;
    item = m_map_push_2_pushlisher.find(key_stream_name);
    if (item != m_map_push_2_pushlisher.end()) {
        publisher = item->second;
    }
    if (NULL == publisher) {
        if (b_player) {//player
            sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::handler, new player[%s:%d], stream='%s' no publisher, refused.",
                    this, peer_name, peer_port, key_stream_name);
    	    srt->libsrt_close();
    	    delete srt;
    	    return client_count;
        } else {
        	//is publisher, create new publisher
            CSLSPublisher * pub = new CSLSPublisher;
            pub->set_srt(srt);
            pub->set_parent(this);
            pub->set_conf(ca);
            pub->init();

            sprintf(tmp, "%s/%s", app_uplive.c_str(), stream_name);
            m_map_push_2_pushlisher[tmp] = pub;
            m_list_role->push(pub);
            sls_log(SLS_LOG_INFO, "[%p]CSLSListener::handler, new publisher[%s:%d], uplive_key=%s, live_key=%s.",
                    this, peer_name, peer_port, key_stream_name, tmp);
        }
    } else {
        if (b_player) {//player
            //new player
            if (srt->libsrt_socket_nonblock(0) < 0)
                sls_log(SLS_LOG_WARNING, "[%p]CSLSListener::handler, new player[%s:%d], libsrt_socket_nonblock failed.",
                        this, peer_name, peer_port);

            CSLSPlayer * player = new CSLSPlayer;
            player->init();
            player->set_srt(srt);
            player->set_parent(publisher);

            publisher->add_player(player);
            m_list_role->push(player);
            sls_log(SLS_LOG_INFO, "[%p]CSLSListener::handler, new player[%s:%d]=[%p], publisher='%s:%p', player count=%d.",
                    this, peer_name, peer_port, player, key_stream_name, publisher, publisher->get_player_count());
            ret = 1;
        } else {//publisher
            if (SLS_RS_INITED == publisher->get_state()) {
                sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::handler, new publisher[%s:%d], stream='%s',but publisher already exist, refused.",
                        this, peer_name, peer_port, key_stream_name);
                srt->libsrt_close();
                delete srt;
                return client_count;
            } else {
                CSLSPublisher *pub = new CSLSPublisher;
                pub->set_srt(srt);
                pub->set_parent(this);
                pub->set_player_list(publisher->get_player_list());
                pub->set_conf(ca);
                pub->init();

                sprintf(tmp, "%s/%s", app_uplive.c_str(), stream_name);
                m_map_push_2_pushlisher[tmp] = pub;
                m_list_role->push(pub);

                publisher->set_player_list(NULL);
                publisher->set_parent(NULL);

                sls_log(SLS_LOG_INFO, "[%p]CSLSListener::handler, publisher[%s:%d]=%p come back, live_key=%s，players=%d.",
                        this, peer_name, peer_port, publisher, key_stream_name, pub->get_player_count());

            }
        }
    }

	return client_count;
}


void CSLSListener::check_invalid_publisher()
{
    std::map<std::string, CSLSPublisher *>::iterator it;
    std::map<std::string, CSLSPublisher *>::iterator it_erase;
    for(it=m_map_push_2_pushlisher.begin(); it!=m_map_push_2_pushlisher.end(); ) {
        std::string live_stream_name = it->first;
        CSLSPublisher * pub = it->second;
        if (NULL == pub) {
            it_erase = it;
            it ++;
            m_map_push_2_pushlisher.erase(it_erase);
            sls_log(SLS_LOG_INFO, "[%p]CSLSListener::check_invalid_publisher, publiser is null, live_key=%s.",
                    this, live_stream_name.c_str());
            continue;
        }
        int state = pub->get_state();
        if (SLS_RS_INVALID == state) {
            sls_log(SLS_LOG_INFO, "[%p]CSLSListener::check_invalid_publisher, remove publiser[%p], live_key=%s.",
                    this, pub, live_stream_name.c_str());
            it_erase = it;
            it ++;
            m_map_push_2_pushlisher.erase(it_erase);
            pub->set_parent(NULL);
        } else {
            it ++;
        }
    }
}

void CSLSListener::clear(bool invalid, bool del)
{
    std::map<std::string, CSLSPublisher *>::iterator it;
    std::map<std::string, CSLSPublisher *>::iterator it_erase;
    for(it=m_map_push_2_pushlisher.begin(); it!=m_map_push_2_pushlisher.end(); ) {
        std::string live_stream_name = it->first;
        CSLSPublisher * pub = it->second;
        if (pub) {
            if (invalid) {
                pub->set_parent(NULL);
            }
            if (del) {
                delete pub;
            }
            it_erase = it;
            it ++;
            m_map_push_2_pushlisher.erase(it_erase);
        } else {
            it ++;
        }
    }
    m_map_push_2_pushlisher.clear();
}

