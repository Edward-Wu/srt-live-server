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


#include "SLSPublisher.hpp"
#include "SLSPlayer.hpp"
#include "SLSLog.hpp"

/**
 * app conf
 */
SLS_CONF_DYNAMIC_IMPLEMENT(app)

/**
 * CSLSPublisher class implementation
 */

CSLSPublisher::CSLSPublisher()
{
    m_is_write             = 0;
    m_map_publisher        = NULL;

    sprintf(m_role_name, "publisher");

}

CSLSPublisher::~CSLSPublisher()
{
    //release
}

int CSLSPublisher::init()
{
    int ret = CSLSRole::init();
    if (m_conf) {
        //m_exit_delay = ((sls_conf_app_t *)m_conf)->publisher_exit_delay;
    }

    return ret;
}

int CSLSPublisher::uninit()
{
    int ret = SLS_OK;

	if (m_map_data) {
        ret = m_map_data->remove(m_map_data_key);
		sls_log(SLS_LOG_INFO, "[%p]CSLSPublisher::uninit, removed publisher from m_map_data, ret=%d.",
				this, ret);
	}

	if (m_map_publisher) {
        ret = m_map_publisher->remove(this);
		sls_log(SLS_LOG_INFO, "[%p]CSLSPublisher::uninit, removed publisher from m_map_publisher, ret=%d.",
				this, ret);
	}
    return CSLSRole::uninit();
}

void CSLSPublisher::set_map_publisher(CSLSMapPublisher * publisher)
{
	m_map_publisher = publisher;
}

int CSLSPublisher::handler()
{
    return handler_read_data();
}



