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
#include <signal.h>
#include <unistd.h>

using namespace std;

#include "SLSLog.hpp"
#include "SLSManager.hpp"

/**
 * parse the argv
 */
#define SLS_SET_OPT(type, c, n, m, min, max)\
{ #c,\
  #m,\
  offsetof(sls_opt_t, n),\
  sls_conf_set_##type,\
  min,\
  max,\
  }

//1: add new parameter here
struct sls_opt_t {
    char conf_file_name[1024];//-c
    char c_cmd[256];          //-s
    char log_level[256];      //-l log level
//  int xxx;                  //-x example
};
//2: add new parameter here
static sls_conf_cmd_t  conf_cmd_opt[] = {
    SLS_SET_OPT(string, c, conf_file_name, "conf file name", 1, 1023),
    SLS_SET_OPT(string, s, c_cmd,          "cmd: stop/refresh", 1, 1023),
    SLS_SET_OPT(string, l, log_level,      "log level: fatal/error/warning/info/debug/trace", 1, 1023),
//  SLS_SET_OPT(int, x, xxx,          "", 1, 100),//example
};

void sls_remove_marks(char * s) {
    int len = strlen(s);
    if (len < 2)//pair
        return;

    if ((s[0] == '\'' && s[len-1] == '\'')
     || (s[0] == '"' && s[len-1] == '"')) {
        for(int i=0; i < len-2; i ++) {
            s[i] = s[i+1];
        }
        s[len-2] = 0x0;
    }
}

int sls_parese_argv(int argc, char * argv[], sls_opt_t * sls_opt)
{
    char opt_name[256] = {0};
    char opt_value[256] = {0};
    char temp[256] = {0};

    int ret  = SLS_OK;
    int i    = 1;//skip
    int len  = 0;

    //special for '-h'
    if (argc == 2) {
        strcpy(temp, argv[1]);
        sls_remove_marks(temp);
        if (strcmp(temp, "-h") == 0) {
            len = sizeof(conf_cmd_opt)/sizeof(sls_conf_cmd_t);
            printf("option help info:\n");
            for (i=0; i<len; i++) {
                printf("-%s, %s, range: %.0f-%.0f.\n",
                    conf_cmd_opt[i].name,
                    conf_cmd_opt[i].mark,
                    conf_cmd_opt[i].min,
                    conf_cmd_opt[i].max);
            }
        } else {
            printf("wrong parameter, '%s'.\n", argv[1]);
        }
        return SLS_ERROR;
    }
    while (i < argc) {
        strcpy(temp, argv[i]);
        len = strlen(temp);
        if (len ==0) {
            sls_log(SLS_LOG_ERROR, "wrong parameter, is ''.");
            ret = SLS_ERROR;
            return ret;
        }
        sls_remove_marks(temp);
        if (temp[0] != '-') {
            sls_log(SLS_LOG_ERROR, "wrong parameter '%s', the first character must be '-'.", opt_name);
            ret = SLS_ERROR;
            return ret;
        }
        strcpy(opt_name, temp + 1);

        sls_conf_cmd_t * it = sls_conf_find(opt_name, conf_cmd_opt, sizeof(conf_cmd_opt)/sizeof(sls_conf_cmd_t));
        if (!it) {
            sls_log(SLS_LOG_ERROR, "wrong parameter '%s'.", argv[i]);
            ret = SLS_ERROR;
            return ret;
        }
        i ++;
        strcpy(opt_value, argv[i++]);
        sls_remove_marks(opt_value);
        const char * r = it->set(opt_value, it, sls_opt);
        if (r != SLS_CONF_OK) {
            sls_log(SLS_LOG_ERROR, "parameter set failed, %s, name='%s', value='%s'.", r, opt_name, opt_value);
            ret = SLS_ERROR;
            return ret;
        }
    }
    return ret;
}
/*
 * ctrl + c controller
 */
static int b_exit = 0;
static void ctrl_c_handler(int s){
    printf("\ncaught signal %d, exit.\n",s);
    b_exit = 1;
}

/**
 * usage information
 */
#define SLS_MAJOR_VERSION "1"
#define SLS_MIN_VERSION "1"
static void usage()
{
    printf("-------------------------------------------------\n");
    printf("           srt-live-srver \n");
    printf("                    v%s.%s \n", SLS_MAJOR_VERSION, SLS_MIN_VERSION);
    printf("-------------------------------------------------\n");
    printf("    \n");
}


int main(int argc, char* argv[])
{
    struct sigaction    sigIntHandler;
    CSLSManager         sls_manager;
    sls_opt_t           sls_opt;

    int ret = SLS_OK;

    int l = sizeof(sockaddr_in);
    usage();

    //parse cmd line
    memset(&sls_opt, 0, sizeof(sls_opt));
    if (argc > 1) {
        //parset argv
        ret = sls_parese_argv(argc, argv, &sls_opt);
        if (ret!= SLS_OK) {
            CSLSLog::destory_instance();
            return SLS_ERROR;
        }
    }

    //log level
    if (strlen(sls_opt.log_level) > 0) {
        sls_set_log_level(sls_opt.log_level);
    }

    //parse conf file
    if (strlen(sls_opt.conf_file_name) == 0) {
        sprintf(sls_opt.conf_file_name, "./sls.conf");
    }
    ret = sls_conf_open(sls_opt.conf_file_name);
    if (ret!= SLS_OK) {
        CSLSLog::destory_instance();
        return SLS_ERROR;
    }

    //ctrl + c to exit
    sigIntHandler.sa_handler = ctrl_c_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, 0);


    //init srt
    CSLSSrt::libsrt_init();

    sls_log(SLS_LOG_INFO, "srt live server is running...\n");
    //sls manager
    if (SLS_OK == sls_manager.start()) {
        //int64_t tm = sls_gettime();
        while(!b_exit)
        {
            if (sls_manager.is_single_thread())
                sls_manager.single_thread_handler();
            else
                msleep(40);

            /* for debug
            int d = (sls_gettime() - tm)/1000000;
            if ( d > 15) //s
                b_exit = true;
            //*/
        }
    }

    sls_log(SLS_LOG_INFO, "stop srt live server...\n");
	//stop
    sls_manager.stop();
    //uninit srt
    CSLSSrt::libsrt_uninit();

    sls_conf_close();
    sls_log(SLS_LOG_INFO, "bye bye!\n");
    CSLSLog::destory_instance();

    return 0;
}
