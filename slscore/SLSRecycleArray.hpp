
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


#ifndef _SLSRecycleArray_INCLUDE_
#define _SLSRecycleArray_INCLUDE_

#include <list>
#include <string.h>

#include "common.hpp"
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
    int  get(char *dst, int size, SLSRecycleArrayID *read_id, int aligned=0);

    void setSize(int n);
    int  count();

    int64_t get_last_read_time();
private:
    char     *m_arrayData;
    int       m_nDataSize;
    int       m_nDataCount;
    int       m_nWritePos;
    int64_t   m_last_read_time;


    CSLSRWLock m_rwclock;


};



#endif
