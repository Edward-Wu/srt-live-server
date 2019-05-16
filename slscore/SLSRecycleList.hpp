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

#ifndef _SLSRecycleList_INCLUDE_
#define _SLSRecycleList_INCLUDE_

#include <list>
#include <string.h>

#include "SLSLock.hpp"


struct SLSRecycleNode
{
    char* data;
    int   data_len;

    SLSRecycleNode(char *src, int len) {
        data = new char[len];
        data_len = len;
        memcpy(data, src, len);
    }

    SLSRecycleNode(SLSRecycleNode *p) {
        SLSRecycleNode(p->data, p->data_len);
    }

    ~SLSRecycleNode() {
    	if (data) {
            delete[] data;
            data = NULL;
    	}
    }

    int copy(char *src, int len) {
        if (len != data_len) {
            delete[] data;
            data = new char[len];
            data_len = len;
        }
        memcpy(data, src, len);
        return len;
    }

    int copy(SLSRecycleNode * p) {
        return copy(p->data, p->data_len);
    }
};

/**
 * CSLSRecycleList
 */
class CSLSRecycleList
{
public :
    CSLSRecycleList();
    ~CSLSRecycleList();

public :
    int              push(SLSRecycleNode *node);
    int              push(char * data, int len);
    SLSRecycleNode * pop();
    void             recycle(SLSRecycleNode *node);

    void setNodeSize(int n);
    int  size();
    void erase();
private:
    std::list<SLSRecycleNode *> m_listData;
    std::list<SLSRecycleNode *> m_listIdle;

    int       m_nNodeSize ;
    CSLSMutex m_mutex;


};



#endif
