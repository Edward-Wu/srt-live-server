
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


#include "SLSArray.hpp"
#include "SLSLog.hpp"

const int DEFAULT_MAX_DATA_SIZE = 4096;//about 5mbps*2sec

CSLSArray::CSLSArray()
{
    m_nDataSize     = DEFAULT_MAX_DATA_SIZE;
    m_nWritePos     = 0;
    m_nReadPos      = 0;
    m_nDataCount    = 0;
    m_arrayData     = new uint8_t[m_nDataSize];

}

CSLSArray::~CSLSArray()
{
	CSLSLock lock(&m_mutex);
    if (m_arrayData != NULL) {
        delete[] m_arrayData;
        m_arrayData = NULL;
    }
}

int CSLSArray::count()
{
    CSLSLock lock(&m_mutex);
    return m_nDataCount;
}

//please call this function before get and put,
//if not, the read data will be make confusion.
void CSLSArray::setSize(int n)
{
    CSLSLock lock(&m_mutex);
    delete[] m_arrayData ;
    m_nDataSize      = n;
    m_nWritePos      = 0;
    m_nReadPos       = 0;
    m_nDataCount     = 0;
    m_arrayData      = new uint8_t[m_nDataSize];
}

void CSLSArray::clear()
{
    m_nWritePos      = 0;
    m_nReadPos       = 0;
    m_nDataCount     = 0;
}

int CSLSArray::put(const uint8_t * data, int len)
{
    if (NULL == data || len <= 0) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSArray::put, failed, data=%p, len=%d.",
                this, data, len);
        return SLS_ERROR;
    }

    CSLSLock lock(&m_mutex);

    int nRemainder = m_nDataSize - m_nDataCount;
    if (len > nRemainder) {
    	//need expand data buff
        //ext at least DEFAULT_MAX_DATA_SIZE each time.
        int   ext_len  = m_nDataSize + (DEFAULT_MAX_DATA_SIZE>=len?DEFAULT_MAX_DATA_SIZE:len);//m_nDataCount + len;
        sls_log(SLS_LOG_INFO, "[%p]CSLSArray::put, len=%d is bigger than nRemainder=%d, ext m_nDataSize=%d to ext_len=%d.",
                this, data, len, m_nDataSize, ext_len);

        uint8_t *ext_data = new uint8_t[ext_len];
        int re = get_inline(ext_data, m_nDataCount);
        memcpy(ext_data + re, data, len);
        delete[] m_arrayData;
        m_arrayData      = ext_data;
        m_nDataSize      = ext_len;
        m_nWritePos      = 0;
        m_nReadPos       = 0;
        m_nDataCount     = re + len;
        return len;
    }


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

    //no consider int wrapround;
    m_nDataCount += len;
    sls_log(SLS_LOG_TRACE, "[%p]CSLSArray::put, len=%d, m_nWritePos=%d, m_nDataCount=%d, m_nDataSize=%d.",
            this, len, m_nWritePos, m_nDataCount, m_nDataSize);
    return len;
}

int CSLSArray::get(uint8_t *data, int size)
{
    CSLSLock lock(&m_mutex);
	return get_inline(data, size);
}

int CSLSArray::get_inline(uint8_t *data, int size)
{
    if (NULL == m_arrayData) {
        sls_log(SLS_LOG_INFO, "[%p]CSLSArray::get, failed, m_arrayData is NULL.", this);
        return SLS_ERROR;
    }

    if (0 == m_nDataCount) {
        sls_log(SLS_LOG_TRACE, "[%p]CSLSArray::get, no new data.", this);
        return SLS_OK;
    }
    sls_log(SLS_LOG_TRACE, "[%p]CSLSArray::get, m_nReadPos=%d, m_nWritePos=%d, m_nDataCount=%d, m_nDataSize=%d.",
            this, m_nReadPos, m_nWritePos, m_nDataCount, m_nDataSize);

    int ready_data_len = 0;
    int copy_data_len  = 0;
    if (m_nReadPos < m_nWritePos) {
        //read pos is behind in the write pos
        ready_data_len = m_nWritePos - m_nReadPos;
        copy_data_len = ready_data_len <= size ? ready_data_len : size;
        //sls_log(SLS_LOG_TRACE, "[%p]CSLSArray::get, read pos is behind in the write pos, copy_data_len=%d, ready_data_len=%d, size=%d.",
        //		this, copy_data_len, ready_data_len, size);
        memcpy(data, m_arrayData + m_nReadPos, copy_data_len);
        m_nReadPos   += copy_data_len;
        m_nDataCount -= copy_data_len;
    } else {
        ready_data_len = m_nDataSize - m_nReadPos + m_nWritePos;
        copy_data_len = ready_data_len <= size ? ready_data_len : size;
        //sls_log(SLS_LOG_TRACE, "[%p]CSLSArray::get, read pos is before of the write pos, copy_data_len=%d, ready_data_len=%d, size=%d.",
        //		this, copy_data_len, ready_data_len, size);
        if (m_nDataSize - m_nReadPos >= copy_data_len) {
            //no wrap round
            memcpy(data, m_arrayData + m_nReadPos, copy_data_len);
            m_nReadPos   += copy_data_len;
        } else {
            memcpy(data, m_arrayData + m_nReadPos, m_nDataSize - m_nReadPos);
            //wrap around
            memcpy(data + (m_nDataSize - m_nReadPos), m_arrayData, copy_data_len - (m_nDataSize - m_nReadPos));
            m_nReadPos = copy_data_len - (m_nDataSize - m_nReadPos);
        }
        m_nDataCount -= copy_data_len;
    }
    if (m_nReadPos == m_nDataSize)
    	m_nReadPos = 0;

    if (m_nReadPos > m_nDataSize) {
        sls_log(SLS_LOG_WARNING, "[%p]CSLSArray::get, m_nReadPos=%d, but m_nDataSize=%d.",
        		this, m_nReadPos, m_nDataSize);
    	m_nReadPos = 0;
    }
    sls_log(SLS_LOG_TRACE, "[%p]CSLSArray::get, copy_data_lens=%d.",
    		this, copy_data_len);
    return copy_data_len;
}



