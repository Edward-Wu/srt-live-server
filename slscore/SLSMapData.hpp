
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


#ifndef _SLSMapData_INCLUDE_
#define _SLSMapData_INCLUDE_

#include <map>
#include <string>

#include "SLSRecycleArray.hpp"
#include "SLSLock.hpp"

class CSLSMapData
{
public:
    CSLSMapData();
    virtual ~CSLSMapData();

    int  add(char *key);
    int  remove(char *key);
    void clear();

    int  put(char *key, char *data, int len, int64_t *last_read_time=NULL);
    int  get(char *key, char *data, int len, SLSRecycleArrayID *read_id, int aligned=0);

    bool is_exist(char *key);

    int  get_ts_info(char *key, char *data, int len);
private:
    std::map<std::string, CSLSRecycleArray *>    m_map_array;        //uplive_key_stream:data'
    std::map<std::string, ts_info *>             m_map_ts_info;      //uplive_key_stream:ts_info'
    CSLSRWLock          m_rwclock;

    int check_ts_info(char *data, int len, ts_info *ti);
};


#endif
