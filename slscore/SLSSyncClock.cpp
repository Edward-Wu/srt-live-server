
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

#include "SLSSyncClock.hpp"
#include "common.hpp"

#define TM_JITTER 1000//ms

CSLSSyncClock::CSLSSyncClock()
{
    m_jitter = TM_JITTER;
	m_begin_ms_rts = -1;
	m_begin_ms_sys = -1;
}

CSLSSyncClock::~CSLSSyncClock()
{
}

int  CSLSSyncClock::wait(int64_t rts_tm_ms)
{
	if (-1 == m_begin_ms_rts) {
		m_begin_ms_rts = rts_tm_ms;
		m_begin_ms_sys = sls_gettime_ms();
		return SLS_OK;
	}
	int64_t cur_sys_ms = sls_gettime_ms();
	int64_t sys_passed = cur_sys_ms - m_begin_ms_sys;
	int64_t rts_passed = rts_tm_ms - m_begin_ms_rts;
	int64_t d = rts_passed - sys_passed;
	if (d >= m_jitter || d <= (-1 * m_jitter)) {
		//jitter
		m_begin_ms_rts = rts_tm_ms;
		m_begin_ms_sys = cur_sys_ms;
		return SLS_OK;
	}
    if (d > 0) {
    	//printf("rts_passed=%lld, sys_passed=%lld, d=%lld\n", rts_passed, sys_passed, d);
    	msleep(d);
    }
    return SLS_OK;
}

void CSLSSyncClock::set_jitter(int v)
{
	m_jitter = v;
}

