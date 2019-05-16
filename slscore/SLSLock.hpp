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

#ifndef _SLSLock_INCLUDE_
#define _SLSLock_INCLUDE_

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>


/**
 * CSLSMutex
 */

class CSLSMutex
{
public :
    CSLSMutex()
    {
        pthread_mutex_init(&m_mutex,NULL);
    }
    ~CSLSMutex()
    {
        pthread_mutex_destroy(&m_mutex);
    }
    pthread_mutex_t * get_mutex() {  return &m_mutex; }
private:

    pthread_mutex_t m_mutex;

};

/**
 * CSLSLock
 */
class CSLSLock
{
public :
	CSLSLock(CSLSMutex * m)
    {
	    m_mutex = NULL;
	    m_locked =false;
		if (m) {
            m_mutex = m->get_mutex();
            int ret = pthread_mutex_lock(m_mutex);
            if (0 == ret) {
		        m_locked = true;
            } else {
			    printf("SLS Error: pthread_mutex_lock failure, ret=%d.\n", ret);
            }
		}

    }
    ~CSLSLock()
    {
		if (m_locked && m_mutex)
			pthread_mutex_unlock(m_mutex);
    }

private:

    pthread_mutex_t * m_mutex;
    bool m_locked;

};



#endif
