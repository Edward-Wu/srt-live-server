
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


#ifndef _TSFileTimeReader_INCLUDE_
#define _TSFileTimeReader_INCLUDE_

#include <string.h>

#include "common.hpp"
#include "SLSArray.hpp"
#include "SLSSyncClock.hpp"

/**
 * CTSFileTimeReader
 */
class CTSFileTimeReader
{
public :
	CTSFileTimeReader();
    ~CTSFileTimeReader();

public :
    int  open(const char *ts_file_name, bool loop);
    int  close();
    int  get(uint8_t *data, int size, int64_t &tm_ms, bool& jitter);

    int64_t  generate_rts_file(const char  *ts_file_name);

private:
    char            m_file_name[URL_MAX_LEN];
    int             m_rts_fd;
    int             m_dts_pid;
    int64_t         m_dts;
    int64_t         m_pts;
    bool            m_loop;
    CSLSArray       m_array_data;
    int64_t         m_udp_duration;
    int64_t         m_readed_count;

};



#endif
