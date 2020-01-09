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

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <strings.h>

#include "SLSLog.hpp"
#include "SLSLock.hpp"

CSLSLog* CSLSLog::m_pInstance = NULL;

CSLSLog::CSLSLog()
{
	m_level = SLS_LOG_INFO;
	m_log_file   = NULL;
    sprintf(log_filename, "");
}
CSLSLog::~CSLSLog()
{

}
int CSLSLog::create_instance()
{
    if (!m_pInstance) {
    	m_pInstance = new CSLSLog();
    	m_pInstance->m_level = SLS_LOG_ERROR;//default
		return 0;
    }
	return -1;
}
int CSLSLog::destory_instance()
{
	SAFE_DELETE(m_pInstance);
	return 0;
}

void CSLSLog::log(int level, const char *fmt, ...)
{
    if (!m_pInstance)
    	m_pInstance = new CSLSLog();
    if (level > m_pInstance->m_level)
        return;

	va_list vl;
    va_start(vl, fmt);
	m_pInstance->print_log(level, fmt, vl);
	va_end (vl);
}


void CSLSLog::print_log(int level, const char *fmt, va_list vl)
{
    CSLSLock lock(&m_mutex);
    char buf[4096] = {0};
    char buf_info[4096] = {0};
    char cur_time[32] = {0};
    int64_t cur_time_msec = sls_gettime()/1000;
    int64_t cur_time_sec = cur_time_msec/1000;
    cur_time_msec = cur_time_msec - cur_time_sec*1000;
    sls_gettime_fmt(cur_time, cur_time_sec, "%Y-%m-%d %H:%M:%S");
    vsnprintf (buf , 4095 , fmt , vl);
    //sprintf(buf_info, "%s %s: %s\n" , cur_time, LOG_LEVEL_NAME[level], buf);
    sprintf(buf_info, "%s:%03d %s %s: %s\n" , cur_time, cur_time_msec, APP_NAME, LOG_LEVEL_NAME[level], buf);
    printf(buf_info);

    if (m_log_file) {
        fwrite(buf_info, strlen(buf_info), 1, m_log_file);
    }
}

void CSLSLog::set_log_level(char * level)
{
    if (!m_pInstance)
    	m_pInstance = new CSLSLog();

    level = sls_strupper(level);//to upper
    int n = sizeof(LOG_LEVEL_NAME)/sizeof(char*);
    for (int i = 0; i < n; i ++) {
        if (strcmp(level, LOG_LEVEL_NAME[i]) == 0) {
            m_pInstance->m_level = i;
            printf("set log level='%s'.\n" , level, LOG_LEVEL_NAME[i]);
            return ;
        }
    }
    printf("!!!wrong log level '%s', set default '%s'.\n" , level, LOG_LEVEL_NAME[m_pInstance->m_level]);
}

void CSLSLog::set_log_file(char * file_name)
{
    if (!m_pInstance)
        m_pInstance = new CSLSLog();

    if (strlen(m_pInstance->log_filename) == 0) {
        sprintf(m_pInstance->log_filename, "%s", file_name);
        m_pInstance->m_log_file = fopen(m_pInstance->log_filename, "a");
    }


}

