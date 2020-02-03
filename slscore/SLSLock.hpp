
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
