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

    int put(char *key, char *data, int len);
    int get(char *key, char *data, int len, SLSRecycleArrayID *read_id);

    bool is_exist(char *key);

private:
    std::map<std::string, CSLSRecycleArray *>    m_map_array;        //uplive_key_stream:data'
    CSLSRWLock          m_rwclock;


};


#endif
