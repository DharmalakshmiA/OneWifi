//
//  main.cpp
//  wifi_telemetry
//
//  Created by Munshi, Soumya on 9/27/25.
//

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <errno.h>
#include "web.h"
#include "vector.h"
#include "sequence.h"
#include "cJSON.h"
#include "wifi_hal.h"
#include "wifi_util.h"
#include "qmgr.h"
#include "run_qmgr.h"

int run_web_server()
{
    web_t *web;
    server_arg_t arg;
    wifi_util_error_print(WIFI_CTRL,"%s:%d \n",__func__,__LINE__); 
    memset(&arg, 0, sizeof(server_arg_t));
    strcpy(arg.path, "/www/data");
    snprintf(arg.output_file, sizeof(arg.output_file), "%s/telemetry.json", arg.path);
    arg.threshold = 0.4;
    
    arg.sampling =  5;
    arg.reporting = 30;
    
    wifi_util_error_print(WIFI_CTRL,"%s:%d \n",__func__,__LINE__); 
	web = web_t::get_instance(arg.path);
    wifi_util_error_print(WIFI_CTRL,"%s:%d \n",__func__,__LINE__); 
    web->start();
    wifi_util_error_print(WIFI_CTRL,"%s:%d \n",__func__,__LINE__); 
    return 0;
}

int stop_web_server(const char *path)
 {
    wifi_util_error_print(WIFI_CTRL,"stoping web_server %s:%d \n",__func__,__LINE__);
    web_t *web;
    web = web_t::get_instance(path);   // always returns SAME instance
    wifi_util_error_print(WIFI_CTRL,"Got web instance\n");
    web->stop();
   wifi_util_error_print(WIFI_CTRL,"stopped web_server\n");
  }

int start_link_metrics()
{
    cJSON *out_obj;
    wifi_util_error_print(WIFI_CTRL,"%s:%d \n",__func__,__LINE__); 
     wifi_util_error_print(WIFI_CTRL,"started add_stats stats->\n"); 
    server_arg_t arg;
    memset(&arg, 0, sizeof(server_arg_t));
    strcpy(arg.path, "/www/data");
    snprintf(arg.output_file, sizeof(arg.output_file), "%s/telemetry.json", arg.path);
    arg.sampling =  5;
    arg.reporting = 30;
    arg.threshold = 0.4;
    
    qmgr_t *mgr;
    mgr = qmgr_t::get_instance(&arg);   // always returns SAME instance
    wifi_util_error_print(WIFI_CTRL,"%s:%d \n",__func__,__LINE__); 

    mgr->start_background_run(); 
    wifi_util_error_print(WIFI_CTRL,"%s:%d \n",__func__,__LINE__); 
    return 0;
}


int add_stats_metrics(stats_arg_t *stats)
{
    cJSON *out_obj;
    wifi_util_error_print(WIFI_CTRL,"%s:%d \n",__func__,__LINE__); 
    server_arg_t arg;
    memset(&arg, 0, sizeof(server_arg_t));
    strcpy(arg.path, "/www/data");
    snprintf(arg.output_file, sizeof(arg.output_file), "%s/telemetry.json", arg.path);
    arg.sampling =  5;
    arg.reporting = 30;
    arg.threshold = 0.4;
    wifi_util_error_print(WIFI_CTRL,"mac_address=%s per =%f, snr=%d and phy=%d\n",stats->mac_str,stats->per,stats->snr,stats->phy); 
    
    qmgr_t *mgr;
    mgr = qmgr_t::get_instance(&arg);   // always returns SAME instance

    mgr->init(stats,true);
    wifi_util_error_print(WIFI_CTRL,"Added the stats data->\n"); 
    return 0;
}

int remove_link_stats( stats_arg_t  *stats)
{
    cJSON *out_obj;
    printf("started %s:%d \n",__func__,__LINE__); 
    server_arg_t arg;
    memset(&arg, 0, sizeof(server_arg_t));
    snprintf(arg.output_file, sizeof(arg.output_file), "%s/telemetry.json", arg.path);
    arg.sampling =  5;
    arg.reporting = 30;
    arg.threshold = 0.4;
    
    qmgr_t *mgr;
    mgr = qmgr_t::get_instance(&arg);   // always returns SAME instance
	mgr->init(stats,false);
   wifi_util_error_print(WIFI_CTRL,"mac_str=%s %s:%d \n",stats->mac_str,__func__,__LINE__); 
    return 0;
}
