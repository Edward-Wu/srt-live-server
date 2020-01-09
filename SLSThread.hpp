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

#ifndef _SLSThread_INCLUDE_
#define _SLSThread_INCLUDE_


#include <pthread.h>



/**
 * CSLSThread , the base thread class
 */
class CSLSThread
{
public :
	CSLSThread();
    ~CSLSThread();

    int start();
    int stop();

    virtual int work();
protected:
	bool m_exit;
	pthread_t m_th_id;

    virtual void    clear();

private:
	static void *thread_func(void *);

};


#endif
