
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
#include <string.h>
#include <signal.h>
#include <unistd.h>

using namespace std;

#include "SLSLog.hpp"
#include "SLSManager.hpp"
#include "HttpClient.hpp"

/*
 * ctrl + c controller
 */
static bool b_exit = 0;
static void ctrl_c_handler(int s){
    printf("\ncaught signal %d, exit.\n",s);
    b_exit = true;
}


static bool b_reload = 0;
static void reload_handler(int s){
    printf("\ncaught signal %d, reload.\n",s);
    b_reload = true;
}

/**
 * usage information
 */
#define SLS_MAJOR_VERSION "1"
#define SLS_MIN_VERSION "4"
#define SLS_TEST_VERSION "x"
static void usage()
{
    printf("-------------------------------------------------\n");
    printf("           srt-live-server \n");
    printf("                    v%s.%s.%s \n", SLS_MAJOR_VERSION, SLS_MIN_VERSION, SLS_TEST_VERSION);
    printf("-------------------------------------------------\n");
    printf("    \n");
}

//add new parameter here
static sls_conf_cmd_t  conf_cmd_opt[] = {
    SLS_SET_OPT(string, c, conf_file_name, "conf file name", 1, 1023),
    SLS_SET_OPT(string, s, c_cmd,          "cmd: reload", 1, 1023),
    SLS_SET_OPT(string, l, log_level,      "log level: fatal/error/warning/info/debug/trace", 1, 1023),
//  SLS_SET_OPT(int, x, xxx,          "", 1, 100),//example
};

int main(int argc, char* argv[])
{
    struct sigaction    sigIntHandler;
    struct sigaction    sigHupHandler;
    sls_opt_t           sls_opt;

    CSLSManager             *sls_manager = NULL;
    std:list <CSLSManager*>  reload_manager_list;
    CHttpClient             *http_stat_client = new CHttpClient;

    int ret = SLS_OK;
    int l = sizeof(sockaddr_in);
    int64_t tm_begin_ms = 0;

    char stat_method[]        = "POST";
    sls_conf_srt_t * conf_srt = NULL;

    usage();

    //parse cmd line
    memset(&sls_opt, 0, sizeof(sls_opt));
    if (argc > 1) {
        //parset argv
    	int cmd_size = sizeof(conf_cmd_opt)/sizeof(sls_conf_cmd_t);
        ret = sls_parse_argv(argc, argv, &sls_opt, conf_cmd_opt, cmd_size);
        if (ret!= SLS_OK) {
            CSLSLog::destory_instance();
            return SLS_ERROR;
        }
    }

    //reload
    if (strcmp(sls_opt.c_cmd,  "") != 0) {
    	return sls_send_cmd(sls_opt.c_cmd);
    }

    //log level
    if (strlen(sls_opt.log_level) > 0) {
        sls_set_log_level(sls_opt.log_level);
    }

    //test erro info...
    //CSLSSrt::libsrt_print_error_info();

    //ctrl + c to exit
    sigIntHandler.sa_handler = ctrl_c_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, 0);

    //hup to reload
    sigHupHandler.sa_handler = reload_handler;
    sigemptyset(&sigHupHandler.sa_mask);
    sigHupHandler.sa_flags = 0;
    sigaction(SIGHUP, &sigHupHandler, 0);

    //init srt
    CSLSSrt::libsrt_init();

    //parse conf file
    if (strlen(sls_opt.conf_file_name) == 0) {
        sprintf(sls_opt.conf_file_name, "./sls.conf");
    }
    ret = sls_conf_open(sls_opt.conf_file_name);
    if (ret!= SLS_OK) {
        sls_log(SLS_LOG_INFO, "sls_conf_open failed, EXIT!");
        goto EXIT_PROC;
    }

    if (0 != sls_write_pid(getpid())) {
        sls_log(SLS_LOG_INFO, "sls_write_pid failed, EXIT!");
        goto EXIT_PROC;
    }
    //sls manager
    sls_log(SLS_LOG_INFO, "\nsrt live server is running...");

    sls_manager  = new CSLSManager;
    if (SLS_OK != sls_manager->start()) {
        sls_log(SLS_LOG_INFO, "sls_manager->start failed, EXIT!");
        goto EXIT_PROC;
    }

    conf_srt = (sls_conf_srt_t *)sls_conf_get_root_conf();
    if (strlen(conf_srt->stat_post_url) > 0)
        http_stat_client->open(conf_srt->stat_post_url, stat_method, conf_srt->stat_post_interval);

	while(!b_exit)
	{
		int64_t cur_tm_ms = sls_gettime_ms();
		ret = 0;
		if (sls_manager->is_single_thread()) {
			ret = sls_manager->single_thread_handler();
		}
		if (NULL != http_stat_client) {
			if (!http_stat_client->is_valid()) {
				if (SLS_OK == http_stat_client->check_repeat(cur_tm_ms)) {
					http_stat_client->reopen();
				}
			}
			ret = http_stat_client->handler();
			if (SLS_OK == http_stat_client->check_finished() ||
				SLS_OK == http_stat_client->check_timeout(cur_tm_ms)) {
				//http_stat_client->get_response_info();
				http_stat_client->close();
			}

		}

		msleep(10);

		/*for test reload...
		int64_t tm_cur = sls_gettime();
		int64_t d = tm_cur - tm;
		if ( d >= 10000000) {
			b_reload = !b_reload;
			tm = tm_cur;
			printf("\n\n\n\n");
		}
		//*/

		//check reloaded manager
		int reload_managers = reload_manager_list.size();
	    std::list<CSLSManager *>::iterator it;
	    std::list<CSLSManager *>::iterator it_erase;
	    for ( it = reload_manager_list.begin(); it != reload_manager_list.end();)
	    {
	    	CSLSManager * manager = *it;
    		it_erase = it;
    		it ++;
	    	if (NULL == manager) {
	    		continue;
	    	}
	    	if (SLS_OK == manager->check_invalid()) {
	            sls_log(SLS_LOG_INFO, "check reloaded manager, delete manager=%p ...", manager);
	            manager->stop();
	            delete manager;
	    		reload_manager_list.erase(it_erase);
	    	}
	    }

		if (b_reload) {
            //reload
    		b_reload = false;
	    	sls_log(SLS_LOG_INFO, "reload srt live server...");
		    ret = sls_manager->reload();
            if (ret != SLS_OK) {
                sls_log(SLS_LOG_INFO, "reload failed, sls_manager->reload failed.");
                continue;
            }
            reload_manager_list.push_back(sls_manager);
            sls_manager = NULL;
            sls_log(SLS_LOG_INFO, "reload, push old sls_manager to list.");

            sls_conf_close();
            ret = sls_conf_open(sls_opt.conf_file_name);
            if (ret != SLS_OK) {
                sls_log(SLS_LOG_INFO, "reload failed, read config file failed.");
                break;
            }
            sls_log(SLS_LOG_INFO, "reload config file ok.");
            sls_manager = new CSLSManager;
            if (SLS_OK != sls_manager->start()) {
                sls_log(SLS_LOG_INFO, "reload, failed, sls_manager->start, exit.");
                break;
            }
            if (strlen(conf_srt->stat_post_url) > 0)
                http_stat_client->open(conf_srt->stat_post_url, stat_method, conf_srt->stat_post_interval);
            sls_log(SLS_LOG_INFO, "reload successfully.");
		}
	}

EXIT_PROC:
    sls_log(SLS_LOG_INFO, "exit, stop srt live server...");

	//stop srt
    if (NULL != sls_manager) {
        sls_manager->stop();
        delete sls_manager;
        sls_manager = NULL;
        sls_log(SLS_LOG_INFO, "exit, release sls_manager ok.");
    }

    //release all reload manager
    sls_log(SLS_LOG_INFO, "exit, release reload_manager_list begin，count=%d.", reload_manager_list.size());
    std::list<CSLSManager *>::iterator it;
    for ( it = reload_manager_list.begin(); it != reload_manager_list.end(); it++)
    {
    	CSLSManager * manager = *it;
    	if (NULL == manager) {
    		continue;
    	}
    	manager->stop();
        delete manager;
    }
    sls_log(SLS_LOG_INFO, "exit, release reload_manager_list ok.");
    reload_manager_list.clear();

    sls_log(SLS_LOG_INFO, "exit, release http_stat_client.");
    //release http_stat_client
    if (NULL != http_stat_client) {
    	http_stat_client->close();
    	delete http_stat_client;
    	http_stat_client = NULL;
    }

    sls_log(SLS_LOG_INFO, "exit, uninit srt .");
    //uninit srt
    CSLSSrt::libsrt_uninit();

    sls_log(SLS_LOG_INFO, "exit, close conf.");
    sls_conf_close();
    CSLSLog::destory_instance();

    sls_remove_pid();

    sls_log(SLS_LOG_INFO, "exit, bye bye!");

    return 0;
}
