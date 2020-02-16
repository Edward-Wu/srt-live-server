
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


#include <stdio.h>


#include "SLSRecycleArray.hpp"
#include "SLSLog.hpp"

const int DEFAULT_MAX_DATA_SIZE = 1024*1316;//about 5mbps*2sec

CSLSRecycleArray::CSLSRecycleArray()
{
    m_nDataSize     = DEFAULT_MAX_DATA_SIZE;
    m_nWritePos     = 0;
    m_nDataCount    = 0;

    m_last_read_time = sls_gettime_ms();

    m_arrayData      = new char[m_nDataSize];

}

CSLSRecycleArray::~CSLSRecycleArray()
{
    CSLSLock lock(&m_rwclock, true);
    if (m_arrayData != NULL) {
        delete[] m_arrayData;
        m_arrayData = NULL;
    }
}

int CSLSRecycleArray::count()
{
    CSLSLock lock(&m_rwclock, false);
    return m_nDataCount;
}

//please call this function before get and put,
//if not, the read data will be make confusion.
void CSLSRecycleArray::setSize(int n)
{
    CSLSLock lock(&m_rwclock, false);
    delete[] m_arrayData ;
    m_nDataSize      = n;
    m_nWritePos      = 0;
    m_arrayData      = new char[m_nDataSize];
}

int CSLSRecycleArray::put(char * data, int len)
{
    if (!data || len <= 0) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSRecycleArray::put, failed, data=%p, len=%d.",
                this, data, len);
        return SLS_ERROR;
    }

    if (len > m_nDataSize) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSRecycleArray::put, failed, len=%d is bigger than m_nDataSize=%d.",
                this, data, len, m_nDataSize);
        return SLS_ERROR;
    }

    {
        CSLSLock lock(&m_rwclock, true);
        if (m_nDataSize - m_nWritePos >= len) {
            //copy directly
            memcpy(m_arrayData + m_nWritePos, data, len);
            m_nWritePos += len;
        } else {
            int first_len = m_nDataSize - m_nWritePos;
            memcpy(m_arrayData + m_nWritePos, data, first_len);
            memcpy(m_arrayData, data + first_len, len - first_len);
            m_nWritePos = (len - first_len);
        }

        if (m_nWritePos==m_nDataSize)
        	m_nWritePos = 0;
    }
    //no consider int wrapround;
    m_nDataCount += len;
    sls_log(SLS_LOG_TRACE, "[%p]CSLSRecycleArray::put, len=%d, m_nWritePos=%d, m_nDataCount=%d, m_nDataSize=%d.",
            this, len, m_nWritePos, m_nDataCount, m_nDataSize);
    return len;
}

int CSLSRecycleArray::get(char *data, int size, SLSRecycleArrayID *read_id)
{
    if (NULL == m_arrayData) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSRecycleArray::get, failed, m_arrayData is NULL.", this);
        return SLS_ERROR;
    }

    if (NULL == read_id) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSRecycleArray::get, failed, read_id is NULL.", this);
        return SLS_ERROR;
    }

    if (read_id->bFirst) {
        read_id->nReadPos   = m_nWritePos;
        read_id->nDataCount = m_nDataCount;
    	read_id->bFirst     = false;
        sls_log(SLS_LOG_TRACE, "[%p]CSLSRecycleArray::get, the first time.");
        return SLS_OK;
    }

    CSLSLock lock(&m_rwclock, false);
    if (read_id->nReadPos == m_nWritePos && m_nDataCount == read_id->nDataCount) {
        sls_log(SLS_LOG_TRACE, "[%p]CSLSRecycleArray::get, no new data.", this);
        return SLS_OK;
    }
    sls_log(SLS_LOG_TRACE, "[%p]CSLSRecycleArray::get, read_id->nReadPos=%d, m_nWritePos=%d, m_nDataCount=%d, m_nDataSize=%d.",
            this, read_id->nReadPos, m_nWritePos, m_nDataCount, m_nDataSize);

    //update the last read time
    m_last_read_time = sls_gettime_ms();

    int ready_data_len = 0;
    int copy_data_len  = 0;
    if (read_id->nReadPos < m_nWritePos) {
        //read pos is behind in the write pos
        ready_data_len = m_nWritePos - read_id->nReadPos;
        copy_data_len = ready_data_len <= size ? ready_data_len : size;
        //sls_log(SLS_LOG_TRACE, "[%p]CSLSRecycleArray::get, read pos is behind in the write pos, copy_data_len=%d, ready_data_len=%d, size=%d.",
        //		this, copy_data_len, ready_data_len, size);
        memcpy(data, m_arrayData + read_id->nReadPos, copy_data_len);
        read_id->nReadPos += copy_data_len;
    } else {
        ready_data_len = m_nDataSize - read_id->nReadPos + m_nWritePos;
        copy_data_len = ready_data_len <= size ? ready_data_len : size;
        //sls_log(SLS_LOG_TRACE, "[%p]CSLSRecycleArray::get, read pos is before of the write pos, copy_data_len=%d, ready_data_len=%d, size=%d.",
        //		this, copy_data_len, ready_data_len, size);
        if (m_nDataSize - read_id->nReadPos >= copy_data_len) {
            //no wrap round
            memcpy(data, m_arrayData + read_id->nReadPos, copy_data_len);
            read_id->nReadPos += copy_data_len;
        } else {
            memcpy(data, m_arrayData + read_id->nReadPos, m_nDataSize - read_id->nReadPos);
            //wrap around
            memcpy(data + (m_nDataSize - read_id->nReadPos), m_arrayData, copy_data_len - (m_nDataSize - read_id->nReadPos));
            read_id->nReadPos = copy_data_len - (m_nDataSize - read_id->nReadPos);
        }
    }
    if (read_id->nReadPos == m_nDataSize)
    	read_id->nReadPos = 0;

    if (read_id->nReadPos > m_nDataSize) {
        sls_log(SLS_LOG_WARNING, "[%p]CSLSRecycleArray::get, read_id->nReadPos=%d, but m_nDataSize=%d.",
        		this, read_id->nReadPos, m_nDataSize);
    	read_id->nReadPos = 0;
    }
    read_id->nDataCount = m_nDataCount ;
    sls_log(SLS_LOG_TRACE, "[%p]CSLSRecycleArray::get, copy_data_lens=%d.",
    		this, copy_data_len);
    return copy_data_len;
}

int64_t CSLSRecycleArray::get_last_read_time()
{
	return m_last_read_time;
}



