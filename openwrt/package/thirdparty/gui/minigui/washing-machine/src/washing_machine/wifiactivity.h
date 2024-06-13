#ifndef WIFIACTIVITY_H
#define WIFIACTIVITY_H

#if WMG1
#include <wmg_debug.h>
#include <wifi_intf.h>
#endif

#define WMG1 0
typedef int aw_wifi_interface_t;

#define MAX_NUM 128
struct scan_info{
	char ssid[48];
	int level;
};
struct wifi_name{
	char wifi_name[48];
};
int scan_number;
int event;
int STATUS_WIFI_ON_OFF;
int wifi_flag;
int event_label;
const aw_wifi_interface_t *wifi_interface;
struct scan_info scan_info_arry[MAX_NUM];
struct wifi_name wifi_name_array[MAX_NUM];

#endif
