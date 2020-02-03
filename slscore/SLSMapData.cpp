
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


#include <errno.h>
#include <string.h>


#include "SLSMapData.hpp"
#include "SLSLog.hpp"


/**
 * CSLSMapData class implementation
 */

CSLSMapData::CSLSMapData()
{
}
CSLSMapData::~CSLSMapData()
{
    clear();
}

int CSLSMapData::add(char *key)
{
    int ret = SLS_OK;
    std::string strKey = std::string(key);

    CSLSLock lock(&m_rwclock, true);

    std::map<std::string, CSLSRecycleArray *>::iterator item;
    item = m_map_array.find(strKey);
    if (item != m_map_array.end()) {
        CSLSRecycleArray * array_data = item->second;
        if (array_data) {
        	sls_log(SLS_LOG_INFO, "[%p]CSLSMapData::add, failed, key=%s, array_data=%p, exist.",
                    this, key, array_data);
        	return ret;
        }
        //m_map_array.erase(item);
    }

    CSLSRecycleArray * data_array = new CSLSRecycleArray;
    //m_map_array.insert(make_pair(strKey, data_array));
    m_map_array[strKey] = data_array;
    sls_log(SLS_LOG_INFO, "[%p]CSLSMapData::add ok, key='%s'.",
            this, key);
    return ret;
}

int CSLSMapData::remove(char *key)
{
    int ret = SLS_ERROR;
    std::string strKey = std::string(key);

    CSLSLock lock(&m_rwclock, true);

    std::map<std::string, CSLSRecycleArray *>::iterator item;
    item = m_map_array.find(strKey);
    if (item != m_map_array.end()) {
        CSLSRecycleArray * array_data = item->second;
        sls_log(SLS_LOG_INFO, "[%p]CSLSMapData::remove, key='%s' delete array_data=%p.",
                this, key, array_data);
        if (array_data) {
            delete array_data;
        }
    	m_map_array.erase(item);
        return SLS_OK;
    }
    return ret;
}

bool CSLSMapData::is_exist(char *key)
{

    CSLSLock lock(&m_rwclock, true);
    std::string strKey = std::string(key);

    std::map<std::string, CSLSRecycleArray *>::iterator item;
    item = m_map_array.find(key);
    if (item != m_map_array.end()) {
    	CSLSRecycleArray * array_data = item->second;
    	if (array_data) {
            sls_log(SLS_LOG_TRACE, "[%p]CSLSMapData::is_exist, key=%s, exist.",
                this, key);
            return true;
    	} else {
            sls_log(SLS_LOG_TRACE, "[%p]CSLSMapData::is_exist, is_exist, key=%s, data_array is null.",
                this, key);
    	}
    } else {
        sls_log(SLS_LOG_TRACE, "[%p]CSLSMapData::add, is_exist, key=%s, not exist.",
            this, key);
    }
    return false;
}


int CSLSMapData::put(char *key, char *data, int len, int64_t *last_read_time)
{
    int ret = SLS_OK;

    CSLSLock lock(&m_rwclock, true);
    std::string strKey = std::string(key);

    std::map<std::string, CSLSRecycleArray *>::iterator item;
    item = m_map_array.find(key);
    if (item == m_map_array.end()) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSMapData::put, key=%s, not found data array.",
                this, key);
        return SLS_ERROR;
    }
    CSLSRecycleArray *array_data = item->second;
    if (NULL == array_data) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSMapData::get, key=%s, array_data is NULL.",
                this, key);
    }

    ret = array_data->put(data, len);
    if (ret != len) {
        sls_log(SLS_LOG_ERROR, "[%p]CSLSMapData::put, key=%s, array_data->put failed, len=%d, but ret=%d.",
                this, key, len, ret);
    }
    if (NULL != last_read_time) {
        *last_read_time = array_data->get_last_read_time();
    }
    return ret;
}

int CSLSMapData::get(char *key, char *data, int len, SLSRecycleArrayID *read_id)
{
    int ret = SLS_OK;

    CSLSLock lock(&m_rwclock, false);
    std::string strKey = std::string(key);

    std::map<std::string, CSLSRecycleArray *>::iterator item;
    item = m_map_array.find(strKey);
    if (item == m_map_array.end()) {
        sls_log(SLS_LOG_TRACE, "[%p]CSLSMapData::get, key=%s, not found data array,",
                this, key);
        return SLS_ERROR;
    }
    CSLSRecycleArray *array_data = item->second;
    if (NULL == array_data) {
        sls_log(SLS_LOG_WARNING, "[%p]CSLSMapData::get, key=%s, array_data is NULL.",
                this, key);
        return SLS_ERROR;
    }

    ret = array_data->get(data, len, read_id);
    return ret;
}

void CSLSMapData::clear()
{
    CSLSLock lock(&m_rwclock, true);
    std::map<std::string, CSLSRecycleArray *>::iterator it;
    for(it=m_map_array.begin(); it!=m_map_array.end(); ) {
        CSLSRecycleArray * array_data = it->second;
        if (array_data) {
            delete array_data;
        }
        it ++;
    }
    m_map_array.clear();
}



