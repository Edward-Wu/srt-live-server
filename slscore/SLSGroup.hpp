
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


#ifndef _SLSGroup_INCLUDE_
#define _SLSGroup_INCLUDE_

#include <map>

#include "SLSEpollThread.hpp"
#include "SLSRoleList.hpp"
#include "SLSRole.hpp"
#include "SLSMapRelay.hpp"

/**
 * CSLSGroup , group of players, publishers and listener
 */
class CSLSGroup : public CSLSEpollThread
{
public :
	CSLSGroup();
    ~CSLSGroup();

    int  start();
    int  stop();
    void reload();

    void set_role_list(CSLSRoleList *list_role);
    void set_worker_connections(int n);
    void set_worker_number(int n);

    virtual int     handler();

protected:
    virtual void    clear();

private:
    CSLSRoleList           *   m_list_role;
    std::map<int, CSLSRole *>  m_map_role;

    std::list<CSLSRelayManager *>  m_list_reconnect_relay_manager;


    void  idle_check();
    void  check_reconnect_relay();
    void  check_invalid_sock();
    void  check_new_role();

    int     m_worker_connections;
    int     m_worker_number;
    int64_t m_cur_time_microsec;
    bool    m_reload;

};


#endif
