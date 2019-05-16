/*
 * This file is part of SLS Live Server.
 *
 * SLS Live Server is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * SLS Live Server is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with SLS Live Server;
 * if not, please contact with the author: Edward.Wu(edward_email@126.com)
 */

#ifndef _SLSGroup_INCLUDE_
#define _SLSGroup_INCLUDE_

#include <map>

#include "SLSEpollThread.hpp"
#include "SLSRoleList.hpp"
#include "SLSRole.hpp"
/**
 * CSLSGroup , group of players, publishers and listener
 */
class CSLSGroup : public CSLSEpollThread
{
public :
	CSLSGroup();
    ~CSLSGroup();

    int start();
    int stop();

    void set_role_list(CSLSRoleList *list_role);
    void set_worker_connections(int n);
    void set_worker_number(int n);

    virtual int     handler();

protected:
    virtual void    clear();

private:
    CSLSRoleList           *   m_list_role;
    std::map<int, CSLSRole *>  m_map_role;

    void  check_invalid_sock();
    void  check_new_role();

    int m_worker_connections;
    int m_worker_number;

};


#endif
