#include <common.h>
#include <aml_rtc.h>
#include <rtc.h>

extern char systest_info_line[];

struct rtc_time pattern1 ={
	.tm_year = 111,
	.tm_mon = 2,
	.tm_mday = 23,
	.tm_hour = 13,
	.tm_min = 15,
	.tm_sec = 3,
};

//===============================================================
static void write_rtc(struct rtc_time *pattern)
{
	struct rtc_time tm;
	tm.tm_year = pattern->tm_year;
	tm.tm_mon = pattern->tm_mon;
	tm.tm_mday = pattern->tm_mday;						
	tm.tm_hour = pattern->tm_hour;						
	tm.tm_min = pattern->tm_min;
	tm.tm_sec = pattern->tm_sec;	 
	aml_rtc_write_time(&tm); 
	sprintf(systest_info_line, "RTC: set time: %04d-%02d-%02d %02d:%02d:%02d\n",
	        	tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);
	systest_log(systest_info_line, SYSTEST_INFO_L2);
}
//===============================================================
static int read_rtc(struct rtc_time* pattern)
{
	struct rtc_time tm;
	aml_rtc_read_time(&tm);
	sprintf(systest_info_line, "RTC: get time: %04d-%02d-%02d %02d:%02d:%02d\n", 
			tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);
	systest_log(systest_info_line, SYSTEST_INFO_L2);	

	if((tm.tm_year != pattern->tm_year) || (tm.tm_mon != pattern->tm_mon) || (tm.tm_mday != pattern->tm_mday)
		|| (tm.tm_hour != pattern->tm_hour) || (tm.tm_min != pattern->tm_min) 
		|| (tm.tm_sec < 0) || (tm.tm_sec > 60)) 
		return -1;	
	
	else	
		return 1;		
	
}
//===============================================================
static int rtc_sub_test()
{
	int i, ret;
	write_rtc(&pattern1);	
	ret = 0;
	for(i=0; i<4;i++){
		if(read_rtc(&pattern1) < 0){
			sprintf(systest_info_line, "%s:%d: RTC read time[%d] fail. \n", __FILE__, __LINE__, i);
			systest_log(systest_info_line, SYSTEST_INFO_L2);
			ret = -1;
		}		
	}
	if(ret < 0)
		return -1;
	else
		return 1;
}
//===============================================================
int rtc_systest(int argc, char *argv[])
{
	if(argc < 2){
		if(rtc_sub_test() < 0)
			systest_log("rtc: test fail. \n", SYSTEST_INFO_L1);			
		else
			systest_log("rtc: test pass. \n", SYSTEST_INFO_L1);
		return 1;		
	}
	else{
			systest_log("rtc: test fail: parameter number error.\n", SYSTEST_INFO_L1);		
	}
	return 0;
}
