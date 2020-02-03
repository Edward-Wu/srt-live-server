
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
#include <vector>

#include "SLSListener.hpp"
#include "SLSLog.hpp"
#include "SLSPublisher.hpp"
#include "SLSPlayer.hpp"
#include "SLSPullerManager.hpp"
#include "SLSPusherManager.hpp"

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

    m_list_role     = NULL;
    m_map_publisher = NULL;
    m_map_puller    = NULL;
    m_idle_streams_timeout      = UNLIMITED_TIMEOUT;
    m_idle_streams_timeout_role = 0;

    sprintf(m_role_name, "listener");
}

CSLSListener::~CSLSListener()
{
}

int CSLSListener::init()
{
	int ret = 0;
    return CSLSRole::init();
}

int CSLSListener::uninit()
{
	CSLSLock lock(&m_mutex);
    stop();
    return CSLSRole::uninit();
}

void CSLSListener::set_role_list(CSLSRoleList *list_role)
{
	m_list_role = list_role;
}

void CSLSListener::set_map_publisher(CSLSMapPublisher * publisher)
{
	m_map_publisher = publisher;
}

void CSLSListener::set_map_puller(CSLSMapRelay *map_puller)
{
    m_map_puller     = map_puller;
}

void CSLSListener::set_map_pusher(CSLSMapRelay *map_pusher)
{
    m_map_pusher     = map_pusher;
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

    if (NULL == m_map_puller) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::init_conf_app failed, m_map_puller is null.", this);
        return SLS_ERROR;
    }

    if (NULL == m_map_pusher) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::init_conf_app failed, m_map_pusher is null.", this);
        return SLS_ERROR;
    }

    if (!m_conf) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::init_conf_app failed, conf is null.", this);
        return SLS_ERROR;
    }
    conf_server = (sls_conf_server_t *)m_conf;

    m_back_log                   = conf_server->backlog;
    m_idle_streams_timeout_role  = conf_server->idle_streams_timeout;
    sls_log(SLS_LOG_INFO, "[%p]CSLSListener::init_conf_app, m_back_log=%d, m_idle_streams_timeout=%d.",
            this, m_back_log, m_idle_streams_timeout_role);

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
        m_map_publisher->set_conf(strUplive, ca);
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
            //m_map_live_2_uplive[strTemp]  = strUplive;
            m_map_publisher->set_live_2_uplive(strTemp, strUplive);

            sls_log(SLS_LOG_INFO, "[%p]CSLSListener::init_conf_app, add app live='%s', app push='%s'.",
                    this, strTemp.c_str(), strUplive.c_str());
        }

        if (NULL != ca->child) {
        	sls_conf_relay_t *cr = (sls_conf_relay_t *)ca->child;
            while (cr) {
				if (strcmp(cr->type, "pull") == 0 ) {
					if (SLS_OK != m_map_puller->add_relay_conf(strUplive.c_str(), cr)) {
						sls_log(SLS_LOG_WARNING, "[%p]CSLSListener::init_conf_app, m_map_puller.add_app_conf faile. relay type='%s', app push='%s'.",
								this, cr->type, strUplive.c_str());
					}
				}
				else if (strcmp(cr->type, "push") == 0) {
					if (SLS_OK != m_map_pusher->add_relay_conf(strUplive.c_str(), cr)) {
						sls_log(SLS_LOG_WARNING, "[%p]CSLSListener::init_conf_app, m_map_pusher.add_app_conf faile. relay type='%s', app push='%s'.",
								this, cr->type, strUplive.c_str());
					}
				} else {
					sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::init_conf_app, wrong relay type='%s', app push='%s'.",
							this, cr->type, strUplive.c_str());
					return SLS_ERROR;
				}
				cr = (sls_conf_relay_t *)cr->sibling;
            }
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

    sls_log(SLS_LOG_INFO, "[%p]CSLSListener::start, m_list_role=%p.", this, m_list_role);
    if (NULL == m_list_role) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSListener::start, m_roleList is null.", this);
        return ret;
    }


    sls_log(SLS_LOG_INFO, "[%p]CSLSListener::start, push to m_list_role=%p.", this, m_list_role);
    m_list_role->push(this);

	return ret;
}

int CSLSListener::stop()
{
	int ret = SLS_OK;
    sls_log(SLS_LOG_INFO, "[%p]CSLSListener::stop.", this);

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

    //1: accept
    fd_client = m_srt->libsrt_accept();
    if (ret < 0) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::handler, srt_accept failed, fd=%d.", this, get_fd());
        CSLSSrt::libsrt_neterrno();
        return client_count;
    }
    client_count = 1;

    //2.check streamid, split it
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

    std::string app_uplive = "";
    sls_conf_app_t * ca = NULL;

    //3.is player?
    app_uplive = m_map_publisher->get_uplive(key_app);
    if (app_uplive.length() > 0) {
        sprintf(key_stream_name, "%s/%s", app_uplive.c_str(), stream_name);
        CSLSRole * pub = m_map_publisher->get_publisher(key_stream_name);
        if (NULL == pub) {
        	//*
        	//3.1 check pullers
        	if (NULL == m_map_puller) {
    			sls_log(SLS_LOG_INFO, "[%p]CSLSListener::handler, refused, new role[%s:%d], stream='%s', publisher is NULL and m_map_puller is NULL.",
    						this, peer_name, peer_port, key_stream_name);
    			srt->libsrt_close();
    			delete srt;
    			return client_count;
        	}
        	CSLSRelayManager *puller_manager = m_map_puller->add_relay_manager(app_uplive.c_str(), stream_name);
        	if (NULL == puller_manager) {
    			sls_log(SLS_LOG_INFO, "[%p]CSLSListener::handler, m_map_puller->add_relay_manager failed, new role[%s:%d], stream='%s', publisher is NULL, no puller_manager.",
    						this, peer_name, peer_port, key_stream_name);
    			srt->libsrt_close();
    			delete srt;
    			return client_count;
        	}

        	puller_manager->set_map_data(m_map_data);
        	puller_manager->set_map_publisher(m_map_publisher);
        	puller_manager->set_role_list(m_list_role);

        	if (SLS_OK != puller_manager->start()) {
    			sls_log(SLS_LOG_WARNING, "[%p]CSLSListener::handler, puller_manager->start failed, new client[%s:%d], stream='%s'.",
    						this, peer_name, peer_port, key_stream_name);
    			srt->libsrt_close();
    			delete srt;
    			return client_count;
            }
    	    sls_log(SLS_LOG_WARNING, "[%p]CSLSListener::handler, puller_manager->start ok, new client[%s:%d], stream=%s.",
	            this, peer_name, peer_port, key_stream_name);

    	    pub = m_map_publisher->get_publisher(key_stream_name);
            if (NULL == pub) {
        	    sls_log(SLS_LOG_WARNING, "[%p]CSLSListener::handler, m_map_publisher->get_publisher failed, new client[%s:%d], stream=%s.",
    	            this, peer_name, peer_port, key_stream_name);
    			srt->libsrt_close();
    			delete srt;
    			return client_count;
            } else {
        	    sls_log(SLS_LOG_WARNING, "[%p]CSLSListener::handler, m_map_publisher->get_publisher ok, pub=%p, new client[%s:%d], stream=%s.",
    	            this, pub, peer_name, peer_port, key_stream_name);
            }
        }

        //3.2 handle new play
        if (!m_map_data->is_exist(key_stream_name)) {
            sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::handler, refused, new role[%s:%d], stream=%s,but publisher data doesn't exist in m_map_data.",
                        this, peer_name, peer_port, key_stream_name);
            srt->libsrt_close();
            delete srt;
            return client_count;
        }

		//new player
		if (srt->libsrt_socket_nonblock(0) < 0)
			sls_log(SLS_LOG_WARNING, "[%p]CSLSListener::handler, new player[%s:%d], libsrt_socket_nonblock failed.",
					this, peer_name, peer_port);

		CSLSPlayer * player = new CSLSPlayer;
		player->init();
		player->set_idle_streams_timeout(m_idle_streams_timeout_role);
		player->set_srt(srt);
		player->set_map_data(key_stream_name, m_map_data);

		m_list_role->push(player);
		sls_log(SLS_LOG_INFO, "[%p]CSLSListener::handler, new player[%p]=[%s:%d], key_stream_name=%s, %s=%p, m_list_role->size=%d.",
				this, player, peer_name, peer_port, key_stream_name, pub->get_role_name(), pub, m_list_role->size());
        return client_count;
    }

    //4. is publisher?
    app_uplive = key_app;
	sprintf(key_stream_name, "%s/%s", app_uplive.c_str(), stream_name);
    ca = (sls_conf_app_t *)m_map_publisher->get_ca(app_uplive);
	if (NULL == ca) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::handler, refused, new role[%s:%d], stream='%s',but no sls_conf_app_t info.",
                    this, peer_name, peer_port, key_stream_name);
        srt->libsrt_close();
        delete srt;
        return client_count;
	}

	CSLSRole * publisher = m_map_publisher->get_publisher(key_stream_name);
	if (NULL != publisher) {
		sls_log(SLS_LOG_ERROR, "[%p]CSLSListener::handler, refused, new role[%s:%d], stream='%s',but publisher=%p is not NULL.",
					this, peer_name, peer_port, key_stream_name, publisher);
		srt->libsrt_close();
		delete srt;
		return client_count;
	}
	//create new publisher
	CSLSPublisher * pub = new CSLSPublisher;
	pub->set_srt(srt);
	pub->set_conf(ca);
	pub->init();
	pub->set_idle_streams_timeout(m_idle_streams_timeout_role);
	sls_log(SLS_LOG_INFO, "[%p]CSLSListener::handler, new pub=%p, key_stream_name=%s.",
			this, pub, key_stream_name);

	//init data array
	if (SLS_OK != m_map_data->add(key_stream_name)) {
		sls_log(SLS_LOG_WARNING, "[%p]CSLSListener::handler, m_map_data->add failed, new pub[%s:%d], stream=%s.",
				this, peer_name, peer_port, key_stream_name);
		pub->uninit();
		delete pub;
		pub = NULL;
		return client_count;
	}

	if (SLS_OK != m_map_publisher->set_push_2_pushlisher(key_stream_name, pub)) {
		sls_log(SLS_LOG_WARNING, "[%p]CSLSListener::handler, m_map_publisher->set_push_2_pushlisher failed, key_stream_name=%s.",
					this, key_stream_name);
		pub->uninit();
		delete pub;
		pub = NULL;
		return client_count;
	}
	pub->set_map_publisher(m_map_publisher);
	pub->set_map_data(key_stream_name, m_map_data);
	m_list_role->push(pub);
	sls_log(SLS_LOG_INFO, "[%p]CSLSListener::handler, new publisher[%s:%d], key_stream_name=%s.",
			this, peer_name, peer_port, key_stream_name);

	//5. check pusher
	if (NULL == m_map_pusher) {
        return client_count;
	}
	CSLSRelayManager *pusher_manager = m_map_pusher->add_relay_manager(app_uplive.c_str(), stream_name);
	if (NULL == pusher_manager) {
		sls_log(SLS_LOG_INFO, "[%p]CSLSListener::handler, m_map_pusher->add_relay_manager failed, new role[%s:%d], key_stream_name=%s.",
					this, peer_name, peer_port, key_stream_name);
		return client_count;
	}
	pusher_manager->set_map_data(m_map_data);
	pusher_manager->set_map_publisher(m_map_publisher);
	pusher_manager->set_role_list(m_list_role);

	if (SLS_OK != pusher_manager->start()) {
		sls_log(SLS_LOG_INFO, "[%p]CSLSListener::handler, pusher_manager->start failed, new role[%s:%d], key_stream_name=%s.",
					this, peer_name, peer_port, key_stream_name);
	}
	return client_count;
}




