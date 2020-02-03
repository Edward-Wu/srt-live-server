
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


#ifndef _SLSPulllerManager_INCLUDE_
#define _SLSPulllerManager_INCLUDE_

#include <vector>
#include <string>

#include "SLSRelayManager.hpp"
#include "conf.hpp"

/**
 * CSLSPullerManager
 */
class CSLSPullerManager: public CSLSRelayManager
{
public :
	CSLSPullerManager();
    virtual ~CSLSPullerManager();

    virtual int start();
    virtual int add_reconnect_stream(char* relay_url);
    virtual int reconnect(int64_t cur_tm_ms);

protected:

    int connect_loop();
    virtual CSLSRelay *create_relay();
    virtual int set_relay_param(CSLSRelay *relay);
    int check_relay_param();

    int   m_cur_loop_index;

};


#endif
