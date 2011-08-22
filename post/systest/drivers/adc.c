#include <common.h>
#include <asm/saradc.h>

#ifdef CONFIG_SARADC

#define SYSTEST_CASES_NUM 10

#ifdef ENABLE_FONT_RESOURCE
extern vidinfo_t panel_info;
#endif
extern char systest_info_line[];
extern struct adc_device aml_adc_devices;

#define IS_KEY(adc_val, value, tolerance) ((adc_val >= (value-tolerance))&&(adc_val <= (value+tolerance)))?1:0

//====================================================================
int adc_systest_init()
{
#ifdef ENABLE_FONT_RESOURCE
	InitFont();	
#endif
	return 1;
}
//====================================================================
static void display_adc_title(char *s, int pos)
{
#ifdef ENABLE_FONT_RESOURCE
	int font_height=GetCharHeight();
	
	int x_cur = 40;
	int y_cur = 20+font_height*SYSTEST_CASES_NUM;
	
	if(s == NULL)
		return;
		
	DrawRect(x_cur, y_cur+font_height*pos, panel_info.vl_col-x_cur,font_height,DISPLAY_BLACK_COLOR);
	AsciiPrintf((uchar*)s, x_cur, y_cur+font_height*pos, DISPLAY_WHITE_COLOR);		
#endif	
}

//====================================================================
static void display_adc_info()
{
#ifdef ENABLE_FONT_RESOURCE	
	int i;
	adckey_info_t *aml_adckey_info = NULL;
	struct adc_info *ptest = aml_adc_mid_device.paml_adc_info;
	int num = aml_adc_mid_device.dev_num;
	
	for(i=0; i<num; i++){
		if(ptest->adc_type == ADC_KEY){
			aml_adckey_info = (adckey_info_t*)ptest->adc_data;
			sprintf(systest_info_line, "key name: %s,  chan: %d, value: %d, tolerance: %d \n", 
													aml_adckey_info->key, ptest->chan, aml_adckey_info->value, aml_adckey_info->tolerance);
			systest_log(systest_info_line, SYSTEST_INFO_L3);
		}
		else{
			sprintf(systest_info_line, "name: %s,  chan: %d \n", ptest->tint, ptest->chan);
			systest_log(systest_info_line, SYSTEST_INFO_L3);
		}
		ptest++;
	}	
#endif	
}

//====================================================================
static int adc_sub_test(struct adc_info* ptest, int pos)
{
	int adc_val;
	int countdown = 10*1000*10;//10 second
	adckey_info_t *aml_adckey_info = NULL;

	if(ptest == NULL){
		sprintf(systest_info_line, "%s:%d: ADC: test fail:  %s\n", __FILE__, __LINE__, "not found adc test case.");
		systest_log(systest_info_line, SYSTEST_INFO_L2);
		return -1;
	}
	display_adc_title(ptest->tint, pos);
	saradc_enable();

	if(ADC_KEY == ptest->adc_type) {
		while(countdown > 0) {
			udelay(100);
			adc_val = get_adc_sample(ptest->chan);
			aml_adckey_info = (adckey_info_t*)ptest->adc_data;
			if(IS_KEY(adc_val, aml_adckey_info->value, aml_adckey_info->tolerance)) {
				sprintf(systest_info_line, "ADC key: %s: test pass.\n", aml_adckey_info->key);
				systest_log(systest_info_line, SYSTEST_INFO_L2);
				return 1;
			}
			countdown -= 1; 
		}
		sprintf(systest_info_line, "%s:%d: ADC:%s: test fail: value is 0x%03x\n", __FILE__, __LINE__, aml_adckey_info->key, adc_val);
		systest_log(systest_info_line, SYSTEST_INFO_L2);
		return -1;
	}	
	else {
		adc_val = get_adc_sample(ptest->chan);
		sprintf(systest_info_line, "%s:%d: SARADC[Chan: %d]: 0x%03x\n", __FILE__, __LINE__, ptest->chan, adc_val);
		systest_log(systest_info_line, SYSTEST_INFO_L2);
		return 1;
	}
}

//====================================================================
int adc_systest(int argc, char *argv[])
{
	struct adc_info  test;
	struct adckey_info key;
	int i=0;
	int ret;
	char token[20];
	int num = aml_adc_devices.dev_num;
	
	switch(argc){
		case 0:
		case 1:   // systest adc all/chan
				ret = 0;
				for(i=0; i<num; i++){
					if(adc_sub_test((aml_adc_devices.adc_device_info)+i, i) < 0)
						ret = -1;
				}
				break;					
		
		default:
			systest_log("adc: test parameters number error.\n", SYSTEST_INFO_L1);
			ret = -1;;
			break;
	}
	
	if(ret < 0)
		systest_log("adc: test fail. \n", SYSTEST_INFO_L1);				
	else
		systest_log("adc: test pass. \n", SYSTEST_INFO_L1);
		
	return 0;	
}


#endif
