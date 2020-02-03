
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


#ifndef _SLSMapRelay_INCLUDE_
#define _SLSMapRelay_INCLUDE_

#include <map>
#include <string>

#include "SLSRelayManager.hpp"
#include "SLSLock.hpp"

class CSLSMapRelay
{
public:
	CSLSMapRelay();
    virtual ~CSLSMapRelay();

    CSLSRelayManager *add_relay_manager(const char *app_uplive, const char *stream_name);
    void clear();

    int add_relay_conf(std::string app_uplive, sls_conf_relay_t * cr);
    SLS_RELAY_INFO *get_relay_conf(std::string app_uplive);

private:
    CSLSRWLock          m_rwclock;
    std::map<std::string, CSLSRelayManager *>   m_map_relay_manager;        //stream_name: relay_manager

    std::map<std::string, SLS_RELAY_INFO *>     m_map_relay_info;        //uplive: relay_conf_info
};


#endif
