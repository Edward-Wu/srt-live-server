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

#ifndef _SLSRecycleArray_INCLUDE_
#define _SLSRecycleArray_INCLUDE_

#include <list>
#include <string.h>

#include "SLSLock.hpp"

struct SLSRecycleArrayID
{
    int   nReadPos;
    int   nDataCount;
    bool  bFirst;
};


/**
 * CSLSRecycleArray
 */
class CSLSRecycleArray
{
public :
    CSLSRecycleArray();
    ~CSLSRecycleArray();

public :
    int  put(char *data, int len);
    int  get(char *dst, int size, SLSRecycleArrayID *read_id);

    void setSize(int n);
    int  count();
private:
    char     *m_arrayData;
    int       m_nDataSize;
    int       m_nDataCount;
    int       m_nWritePos;


    CSLSRWLock m_rwclock;


};



#endif
