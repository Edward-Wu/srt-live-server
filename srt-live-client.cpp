
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
#include "SLSClient.hpp"

/*
 * ctrl + c controller
 */
static bool b_exit = 0;
static void ctrl_c_handler(int s){
    printf("\ncaught signal %d, exit.\n",s);
    b_exit = true;
}


/**
 * usage information
 */
#define SLS_MAJOR_VERSION "1"
#define SLS_MIN_VERSION "1"
#define SLS_TEST_VERSION "x"
static void usage()
{
    printf("-------------------------------------------------\n");
    printf("           srt-live-client \n");
    printf("                    v%s.%s.%s \n", SLS_MAJOR_VERSION, SLS_MIN_VERSION, SLS_TEST_VERSION);
    printf("-------------------------------------------------\n");
    printf("    -r srt_url [-o out_file_name] [-c worker_count] \n");
    printf("    -r srt_url -i ts_file_name \n");
    printf("    \n");
}

struct sls_opt_client_t {
    char input_ts_file[1024];
    char srt_url[1024];
    char out_file_name[1024];
    char ts_file_name[1024];
    int  worker_count;
    bool loop;
//  int xxx;                  //-x example
};


int main(int argc, char* argv[])
{
    struct sigaction    sigIntHandler;
    sls_opt_client_t    sls_opt;

    int ret = SLS_OK;
    usage();

    //parse cmd line
    if (argc < 3) {
        sls_log(SLS_LOG_INFO, "srt live client, no enough parameters, EXIT!");
    	return SLS_OK;
    }

    //parset argv
    memset(&sls_opt, 0, sizeof(sls_opt));
    int i = 1;
	while (i < argc) {
	    sls_remove_marks(argv[i]);
		if (strcmp("-r", argv[i]) == 0) {
			i ++;
	        sls_remove_marks(argv[i]);
			strcpy(sls_opt.srt_url, argv[i++]);
		} else if  (strcmp("-i", argv[i]) == 0) {
			i ++;
	        sls_remove_marks(argv[i]);
			strcpy(sls_opt.ts_file_name, argv[i++]);
		} else if  (strcmp("-o", argv[i]) == 0) {
			i ++;
	        sls_remove_marks(argv[i]);
			strcpy(sls_opt.out_file_name, argv[i++]);
		} else if  (strcmp("-c", argv[i]) == 0) {
			i ++;
	        sls_remove_marks(argv[i]);
			sls_opt.worker_count = atoi(argv[i++]);
		} else if  (strcmp("-l", argv[i]) == 0) {
			i ++;
	        sls_remove_marks(argv[i]);
			sls_opt.loop = atoi(argv[i++]);
		} else {
	        sls_log(SLS_LOG_INFO, "srt live client, wrong parameter '%s', EXIT!", argv[i]);
	    	return SLS_OK;
		}
	}
	CSLSClient sls_client;
	if (strlen(sls_opt.ts_file_name) > 0) {
		if (SLS_OK != sls_client.push(sls_opt.srt_url, sls_opt.ts_file_name, sls_opt.loop)) {
		    sls_log(SLS_LOG_INFO, "sls_client.push failed, EXIT!");
			return SLS_ERROR;
		}
	} else {
		if (SLS_OK != sls_client.play(sls_opt.srt_url, sls_opt.out_file_name)) {
			sls_log(SLS_LOG_INFO, "sls_client.play failed, EXIT!");
			return SLS_ERROR;
		}
		for(i = 1; i < sls_opt.worker_count; i++) {
	        pid_t fpid; //fpid表示fork函数返回的值
	        fpid=fork();
	        if (fpid < 0){
	            printf("srt live client, error in fork!");
	        }else if (fpid == 0) {
	            printf("srt live client, i am the child process, my process id is %d/n",getpid());
	            break;
	        }else {
	            printf("srt live client, i am the parent process, my process id is %d/n",getpid());
	        }
	    }
	}


    //ctrl + c to exit
    sigIntHandler.sa_handler = ctrl_c_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, 0);

    sls_log(SLS_LOG_INFO, "\nsrt live client is running...");
	while(!b_exit)
	{
		//printf log info
		int64_t kb = sls_client.get_bitrate() ;
	    printf("\rsrt live client, cur bitrate=%lld(kb)", kb);

		int ret = sls_client.handler();
		if ( ret > 0) {
			continue;
		} else if (0 == ret) {
			msleep(1);
		} else {
			break;
		}
	}

    sls_log(SLS_LOG_INFO, "exit, stop srt live client...");
    sls_client.close();
    sls_log(SLS_LOG_INFO, "exit, bye bye!");

    return 0;
}
