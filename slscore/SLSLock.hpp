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

class CSLSRWLock
{
public:
    CSLSRWLock()
    {
        m_inited = false;
        int ret = pthread_rwlock_init(&m_rwlock, NULL);
        if (0 == ret) {
            m_inited = true;
        } else {
            printf("pthread_rwlock_init faild, ret=%d.", ret);;
        }
    }
    ~CSLSRWLock()
    {
        if (m_inited) {
            int ret = pthread_rwlock_destroy(&m_rwlock);
            if (0 != ret)
            {
                printf("pthread_rwlock_destroy faild, ret=%d.", ret);;
            }
        }
    }

    int try_lock_write()
    {
        return pthread_rwlock_trywrlock(&m_rwlock);
    }
    int try_lock_read()
    {
        return pthread_rwlock_tryrdlock(&m_rwlock);
    }
    int lock_write()
    {
        return pthread_rwlock_wrlock(&m_rwlock);
    }
    int lock_read()
    {
        return pthread_rwlock_rdlock(&m_rwlock);
    }
    int unlock()
    {
        return pthread_rwlock_unlock(&m_rwlock);
    }
private:
    pthread_rwlock_t m_rwlock;
    bool             m_inited;
};

/**
 * CSLSLock
 */
class CSLSLock
{
public :
    CSLSLock(CSLSRWLock * clock, int write)
    {
        m_mutex = NULL;
        m_clock = NULL;
        m_locked =false;
	    if (NULL == clock)
	        return ;
	    int ret = 0;
	    if (write) {
	        ret = clock->lock_write();
        } else {
            ret = clock->lock_read();
        }
	    if (0 != ret) {
            printf("SLS Error: clock failure, ret=%d.\n", ret);
	    } else {
	        m_clock = clock;
	        m_locked =true;
	    }
    }

    CSLSLock(CSLSMutex * m)
    {
        m_clock = NULL;
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
        if (m_locked && m_clock)
            m_clock->unlock();
    }
private:

    pthread_mutex_t * m_mutex;
    CSLSRWLock      * m_clock;

    bool m_locked;

};





#endif
