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


#include "SLSPuller.hpp"
#include "SLSLog.hpp"
#include "SLSMapRelay.hpp"

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
		sls_log(SLS_LOG_INFO, "[%p]CSLSRelay::uninit, removed relay from m_map_publisher, ret=%d.",
				this, ret);
	}
	if (m_map_data) {
        ret = m_map_data->remove(m_map_data_key);
		sls_log(SLS_LOG_INFO, "[%p]CSLSRelay::uninit, removed relay from m_map_data, ret=%d.",
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
	return handler_read_data();
}



