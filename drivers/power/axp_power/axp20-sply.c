#include <common.h>
#include <div64.h>
#include "axp-sply.h"
#include <asm/setup.h>

#ifdef CONFIG_UBOOT_BATTERY_PARAMETERS
#include <amlogic/battery_parameter.h>
#endif

static int axp_debug = 0;

#define DBG_PSY_MSG(format,args...)   if(axp_debug) printf("[AXP]"format,##args)

#define ABS(x)				((x) >0 ? (x) : -(x) )

static int axp_get_freq(void)
{
	int  ret = 25;
	uint8_t  temp;
	axp_read(AXP20_ADC_CONTROL3, &temp);
	temp &= 0xc0;
	switch(temp >> 6){
		case 0:	ret = 25; break;
		case 1:	ret = 50; break;
		case 2:	ret = 100;break;
		case 3:	ret = 200;break;
		default:break;
	}
	return ret;
}

static inline void axp_read_adc(struct axp_adc_res *adc)
{
	uint8_t tmp[8];
	axp_reads(AXP20_VACH_RES,8,tmp);
	adc->vac_res = ((uint16_t) tmp[0] << 4 )| (tmp[1] & 0x0f);
	adc->iac_res = ((uint16_t) tmp[2] << 4 )| (tmp[3] & 0x0f);
	adc->vusb_res = ((uint16_t) tmp[4] << 4 )| (tmp[5] & 0x0f);
	adc->iusb_res = ((uint16_t) tmp[6] << 4 )| (tmp[7] & 0x0f);
	axp_reads(AXP20_VBATH_RES,6,tmp);
	adc->vbat_res = ((uint16_t) tmp[0] << 4 )| (tmp[1] & 0x0f);

	adc->ichar_res = ((uint16_t) tmp[2] << 4 )| (tmp[3] & 0x0f);

	adc->idischar_res = ((uint16_t) tmp[4] << 5 )| (tmp[5] & 0x1f);
}

static inline int axp_ibat_to_mA(uint16_t reg)
{
	return (reg) * 500 / 1000;
}

int axp_charger_is_ac_online(void)
{
	uint8_t val;
	axp_read(AXP20_CHARGE_STATUS, &val);
	if(val & ((1<<7) | (1<<5)))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int axp_charger_get_charging_status(void)
{
	uint8_t val;
	axp_read(POWER20_MODE_CHGSTATUS, &val);
	if(val & (1<<6))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static int axp_get_basecap()
{
	uint8_t val;

	axp_read(POWER20_DATA_BUFFER4, &val);
	DBG_PSY_MSG("base_cap = axp_read:%d\n",val);

	if((val & 0x80) >> 7)
		return (int) (0 - (val & 0x7F));
	else
		return (int) (val & 0x7F);
}

static void axp_set_basecap(int base_cap)
{
	uint8_t val;
	if(base_cap >= 0)
		val = base_cap & 0x7F;
	else
		val = ABS(base_cap) | 0x80;
	DBG_PSY_MSG("axp_set_basecap = %d\n", val);
	axp_write(POWER20_DATA_BUFFER4, val);
}

/* 得到开路电压 */
static int axp_get_ocv()
{
	int battery_ocv;
	uint8_t v[2];
	axp_reads(AXP_OCV_BUFFER0, 2, v);
	battery_ocv = ((v[0] << 4) + (v[1] & 0x0f)) * 11 /10;
	DBG_PSY_MSG("battery_ocv = %d\n", battery_ocv);
	return battery_ocv;
}



static int axp_get_coulomb(void)
{
	uint8_t  temp[8];
	int64_t  rValue1,rValue2,rValue;
	int Cur_CoulombCounter_tmp,m;


	axp_reads(POWER20_BAT_CHGCOULOMB3,8, temp);
	rValue1 = ((temp[0] << 24) + (temp[1] << 16) + (temp[2] << 8) + temp[3]);
	rValue2 = ((temp[4] << 24) + (temp[5] << 16) + (temp[6] << 8) + temp[7]);

	DBG_PSY_MSG("%s->%d -     CHARGINGOULB:[0]=0x%x,[1]=0x%x,[2]=0x%x,[3]=0x%x\n",__FUNCTION__,__LINE__,temp[0],temp[1],temp[2],temp[3]);
	DBG_PSY_MSG("%s->%d - DISCHARGINGCLOUB:[4]=0x%x,[5]=0x%x,[6]=0x%x,[7]=0x%x\n",__FUNCTION__,__LINE__,temp[4],temp[5],temp[6],temp[7]);

	rValue = (ABS(rValue1 - rValue2)) * 4369;
	m = axp_get_freq() * 480;
	do_div(rValue,m);
	if(rValue1 >= rValue2)
		Cur_CoulombCounter_tmp = (int)rValue;
	else
		Cur_CoulombCounter_tmp = (int)(0 - rValue);
	
	DBG_PSY_MSG("Cur_CoulombCounter_tmp = %d\n",Cur_CoulombCounter_tmp);
	return Cur_CoulombCounter_tmp;	//unit mAh
}





/* add OCV measure by wch */
//#define CONFIG_AXP_USE_OCV
#if defined(CONFIG_AXP_USE_OCV)
#define AXP_CAP_REG 	0xB9
#define AXP_CAP_NUM	10
static int ocv_initCnt = 1;
/***********************************************
  *
  * init_yes=0: 读取电量值
  * init_yes=1: 初始化电量值
  *
 ***********************************************/
static int get_ocv_batteryCap(int init_yes)
{
	int   battery_ocv, charging_status, is_ac_online;
  	unsigned int   i, capTotal=0;
  	unsigned char    capValue, capTmp;
  	static unsigned char  axp_cap_Value=0, capValue_Save=0;
	static unsigned char  cap_buffer[AXP_CAP_NUM], cap_index=0;

	//读状态
	battery_ocv      = axp_get_ocv();
	charging_status = axp_charger_get_charging_status();
	is_ac_online      = axp_charger_is_ac_online();

  	//初始电量值
	if(init_yes)										
	{
		//求2次相邻电量平均值
		memset(cap_buffer, 0, AXP_CAP_NUM);
		axp_read(AXP_CAP_REG, &capValue);
		capTmp = capValue;
		while(1)
		{
			mdelay(100);
			axp_read(AXP_CAP_REG, &capValue);
			if(ABS(capTmp-capValue) > 5)
				capTmp = capValue;
			else
			{
				capValue = (capTmp+capValue) /2;
				break;
			}
		}
		
		//存入10 个相同的电量值,求平均值
		memset(cap_buffer, capValue, AXP_CAP_NUM);
		capTotal	= AXP_CAP_NUM*capValue;
		capValue = capTotal /AXP_CAP_NUM;
		if(!(capValue & (1<<7)))						//OCV 正常工作
		{
			capValue &= 0x7f;
			if(capValue >= 100)
				capValue = 100;				
			if((battery_ocv >= 4090) && (charging_status == 0) && is_ac_online)
				capValue = 100;
						
			axp_cap_Value = capValue;		
		}
		else											//OCV 不正常工作
		{ 
			cap_index = 0;
			capTotal   = 0;
			printf("OCV Measurement not normal work.\n");
			return -1;
		}

		cap_index = 0;
		capTotal   = 0;
		printf("##################init:axp actual cap percent capValue=%d%, capValue_Save=%d%, axp_cap_Value=%d%\n",capValue,capValue_Save,axp_cap_Value);

		return (int)axp_cap_Value;						//返回初始化后的电量值
	}


	//读取电量值
	if(!init_yes)
	{
		//求2次相邻电量平均值
		axp_read(AXP_CAP_REG, &capValue);
		capTmp = capValue;
		while(1)
		{
			mdelay(100);
			axp_read(AXP_CAP_REG, &capValue);
			if(ABS(capTmp-capValue) > 5)
				capTmp = capValue;
			else
			{
				capValue = (capTmp+capValue) /2;
				break;
			}
		}

		//判断OCV是否正常工作
		if(!(capValue & (1<<7)))						//OCV 正常工作
		{
			capValue &= 0x7f;
			if(capValue >= 100)
				capValue = 100;			
			if((battery_ocv >= 4090) && (charging_status == 0) && is_ac_online)
				capValue = 100;			
		}
		else											//OCV 不正常工作
		{ 
			printf("OCV Measurement not normal work.\n");
			return -1;
		}	

		//求10 次电量平均值
		cap_buffer[cap_index] = capValue;
		for(i=0; i<AXP_CAP_NUM; i++)
			capTotal  += cap_buffer[i];
		capValue = capTotal /AXP_CAP_NUM;
		cap_index ++;
		if(cap_index >= AXP_CAP_NUM)
			cap_index = 0;

		//拔插电源处理
		if(is_ac_online )//充电状态
		{
			printf("\nAC online, actual cap percent: %d%\n",capValue);
			printf("axp_cap_Value=%d%\n",axp_cap_Value);
			if(ABS(axp_cap_Value-capValue)<3)			//本次和上一次电量值相差3% 内则进行比较
			{
				if(capValue < axp_cap_Value)
					capValue = axp_cap_Value;
				else
					axp_cap_Value = capValue;	
			}
			else										//否则缓慢增加电量值
			{
				if(axp_cap_Value < capValue)
				{
					axp_cap_Value ++;
					if(axp_cap_Value>=100)
						axp_cap_Value = 100;
				}
				capValue = axp_cap_Value;	
			}	
		}
		else			//放电状态
		{
			printf("\nAC not online, actual cap percent: %d%\n",capValue);
			printf("axp_cap_Value=%d%\n",axp_cap_Value);
			if(ABS(axp_cap_Value-capValue)<3)			//本次和上一次电量值相差3% 内则进行比较
			{
				if(capValue > axp_cap_Value)
					capValue = axp_cap_Value;
				else
					axp_cap_Value = capValue;
			}
			else										//否则缓慢减小电量值
			{
				if(axp_cap_Value > capValue)			
				{
					axp_cap_Value --;
					if(axp_cap_Value<=0)
						axp_cap_Value = 0;
				}	
				capValue = axp_cap_Value;	
			}	
		}

		printf("battery_ocv=%d,charging_status=%d,is_ac_online=%d\n",battery_ocv,charging_status,is_ac_online);	
		printf("OCV Measurement normal work,percent: %d%\n\n",capValue);

		capValue_Save = capValue;
		return (int)capValue;							//返回电量值
	}
}
#endif


int axp_charger_get_charging_percent()
{

	int rdc = 0,Cur_CoulombCounter = 0,base_cap = 0,bat_cap = 0, rest_vol,
		battery_ocv, charging_status, is_ac_online, icharging;
	uint8_t val;
	struct axp_adc_res axp_adc;

#ifdef CONFIG_UBOOT_BATTERY_PARAMETERS
    int i;
    int ocv = 0;
    int ocv_diff, percent_diff, ocv_diff2;
    int charge_status = axp_charger_get_charging_status();
    static int ocv_full  = 0;
    static int ocv_empty = 0;

    if (get_battery_para_flag() == PARA_UNPARSED) {
        /*
         * this code runs earlier than get_battery_para(), 
         * we need to know battery parameters first.
         */
        if (parse_battery_parameters() > 0) {
            set_battery_para_flag(PARA_PARSE_SUCCESS);
            for (i = 0; i < 16; i++) {                  	// find out full & empty ocv in battery curve
                if (!ocv_empty && board_battery_para.pmu_bat_curve[i].discharge_percent > 0) {
                    ocv_empty = board_battery_para.pmu_bat_curve[i - 1].ocv;    
                }
                if (!ocv_full && board_battery_para.pmu_bat_curve[i].discharge_percent == 100) {
                    ocv_full = board_battery_para.pmu_bat_curve[i].ocv;    
                }
            }
        } else {
            set_battery_para_flag(PARA_PARSE_FAILED);
        }
    }
    if (get_battery_para_flag() == PARA_PARSE_SUCCESS) {
        for (i = 0; i < 8; i++) {                           // calculate average ocv
            ocv += axp_get_ocv(); 
            udelay(10000); 
        }
        ocv = ocv / 8;
        if (ocv >= ocv_full) {
            return 100;    
        } else if (ocv <= ocv_empty) {
            return 0;    
        }
        for (i = 0; i < 15; i++) {                          // find which range this ocv is in
            if (ocv >= board_battery_para.pmu_bat_curve[i].ocv &&
                ocv <  board_battery_para.pmu_bat_curve[i + 1].ocv) {
                break;
            }
        }
        if (charge_status) {                                // calculate capability of battery according curve
            percent_diff = board_battery_para.pmu_bat_curve[i + 1].charge_percent -
                           board_battery_para.pmu_bat_curve[i].charge_percent;
        } else {
            percent_diff = board_battery_para.pmu_bat_curve[i + 1].discharge_percent -
                           board_battery_para.pmu_bat_curve[i].discharge_percent;
        }
        ocv_diff  = board_battery_para.pmu_bat_curve[i + 1].ocv -
                    board_battery_para.pmu_bat_curve[i].ocv;
        ocv_diff2 = ocv - board_battery_para.pmu_bat_curve[i].ocv;
        rest_vol  = (percent_diff * ocv_diff2 + ocv_diff / 2)/ocv_diff;
        if (charge_status) {
            rest_vol += board_battery_para.pmu_bat_curve[i].charge_percent;
        } else {
            rest_vol += board_battery_para.pmu_bat_curve[i].discharge_percent;
        }
        if (rest_vol > 100) {
            rest_vol = 100;    
        } else if (rest_vol < 0) {
            rest_vol = 0;    
        }
        return rest_vol;
    }
#endif      /* CONFIG_UBOOT_BATTERY_PARAMETERS */

	axp_read_adc(&axp_adc);
	battery_ocv = axp_get_ocv();
	charging_status = axp_charger_get_charging_status();
	is_ac_online = axp_charger_is_ac_online();
	icharging = ABS(axp_ibat_to_mA(axp_adc.ichar_res)-axp_ibat_to_mA(axp_adc.idischar_res));
	
	axp_read(POWER20_DATA_BUFFERB, &val);
	DBG_PSY_MSG("base_cap = axp_read:%d\n",val);

	DBG_PSY_MSG("icharging = %d\n", icharging);

#ifndef 	CONFIG_AXP_USE_OCV
	if((val & 0x80) >> 7)
	{

		Cur_CoulombCounter = axp_get_coulomb();

		DBG_PSY_MSG("%s->%d: charger->rest_vol > 100\n",__FUNCTION__,__LINE__);
		
		base_cap = axp_get_basecap();
		DBG_PSY_MSG("base_cap = axp_get_basecap(charger):%d\n",base_cap);
		bat_cap = BATTERYCAP;
		rest_vol = 100 * (base_cap * BATTERYCAP / 100 + Cur_CoulombCounter + BATTERYCAP/200) / BATTERYCAP;
		DBG_PSY_MSG("(val & 0x80) >> 7 = 1,rest_vol = :%d\n",rest_vol);

		if((battery_ocv >= 4090) && (rest_vol < 100) && (charging_status == 0) && is_ac_online)
		{
			DBG_PSY_MSG("((battery_ocv >= 4090) && (rest_vol < 100) && (charging_status == 0) && is_ac_online)\n");
			base_cap = 100 - (rest_vol - base_cap);
			axp_set_basecap(base_cap);
			rest_vol = 100;
		}

		if((rest_vol > 99) && charging_status)
		{
			DBG_PSY_MSG("((rest_vol > 99) && charging_status)\n");
			base_cap = 99 - (rest_vol - base_cap);
			axp_set_basecap(base_cap);
			rest_vol = 99;
		}

		if((rest_vol < 100) && (icharging < 280) && charging_status && (battery_ocv >= 4150))
		{
			DBG_PSY_MSG("((rest_vol < 100) && (icharging < 280) && charging_status && (battery_ocv >= 4150))\n");
			rest_vol++;
			base_cap++;
			axp_set_basecap(base_cap);
		}

		if((rest_vol > 0) && (battery_ocv < 3550))
		{
			DBG_PSY_MSG("((rest_vol > 0) && (battery_ocv < 3550))\n");
			base_cap = 0 - (rest_vol - base_cap);
			axp_set_basecap(base_cap);
			rest_vol = 0;
		}

		if((rest_vol < 1) && (battery_ocv > 3650))
		{
			DBG_PSY_MSG("((rest_vol < 1) && (battery_ocv > 3650))\n");
			base_cap = 1 - (rest_vol - base_cap);
			axp_set_basecap(base_cap);
			rest_vol = 1;
		}
	}

	else
	{
		axp_read(0xb9, &val);
		rest_vol = val & 0x7f ;
		DBG_PSY_MSG("(val & 0x80) >> 7 = 0,rest_vol = :%d\n",rest_vol);
		if(rest_vol>=100) {
			rest_vol = 100;
		}
	}

#else  //#define CONFIG_AXP_USE_OCV	
	if(ocv_initCnt==1)		//初始化
	{
		rest_vol = get_ocv_batteryCap(1);
		ocv_initCnt *= 0;
		printf("ocv init...\n");
	}
	else if(ocv_initCnt==0)	//读取电量值
	{
		rest_vol = get_ocv_batteryCap(0);
		printf("use ocv mode...\n");
	}	
#endif

    return rest_vol;
}


void axp_set_charging_current(int current)
{
	uint8_t reg_val = 0;
	axp_read(POWER20_CHARGE1, &reg_val);
	if(current == 0)
	{
		reg_val &= 0x7f;
		axp_write(POWER20_CHARGE1, reg_val);
		printf("%s: set charge current to %d Reg value %x!\n",__FUNCTION__, current, reg_val);		
	}
	else if((current<300)||(current>1800))
	{
		printf("%s: value(%dmA) is outside the allowable range of 300-1800mA!\n",
			__FUNCTION__, current);
	}
	else
	{
		reg_val &= 0xf0;
		reg_val |= ((current-300)/100);
		axp_write(POWER20_CHARGE1, reg_val);
		printf("%s: set charge current to %d Reg value %x!\n",__FUNCTION__, current, reg_val);		
	}
}

int axp_charger_set_usbcur_limit(int usbcur_limit)
{
    uint8_t val;

	axp_read(AXP20_CHARGE_VBUS, &val);

	switch(usbcur_limit)
	{
		case 0:
			val |= 0x3;
			break;
		case 100:
			val |= 0x2;
			break;
		case 500:
			val |= 0x1;
			break;
		case 900:
			val |= 0x0;
			break;
		default:
			printf("usbcur_limit=%d, not in 0,100,500,900. please check!\n");
			return -1;
			break;
	}
	axp_write(AXP20_CHARGE_VBUS, val);
	
    return 0;
}


//unit is mV
int set_dcdc2(u32 val)
{
	char reg_val = 0;
	if((val<700)||(val>2275))
	{
		printf("%s: value(%dmV) is outside the allowable range of 700-2275mV!\n",
			__FUNCTION__, val);
	}
	reg_val = (val-700)/25;
	;
	if(axp_write(POWER20_DC2OUT_VOL, reg_val))
	{
		printf("axp_write(): Failed!\n");
		return -1;
	}
	if(axp_read(POWER20_DC2OUT_VOL, &reg_val))
	{
		printf("axp_read(): Failed!\n");
		return -1;
	}
	printf("POWER20_DC2OUT_VOL is set to 0x%02x\n", reg_val);
	return 0;
}

int set_dcdc3(u32 val)
{
	char reg_val = 0;
	if((val<700)||(val>3500))
	{
		printf("%s: value(%dmV) is outside the allowable range of 700-2275mV!\n",
			__FUNCTION__, val);
	}
	reg_val = (val-700)/25;
	;
	if(axp_write(POWER20_DC3OUT_VOL, reg_val))
	{
		printf("axp_write(): Failed!\n");
		return -1;
	}
	if(axp_read(POWER20_DC3OUT_VOL, &reg_val))
	{
		printf("axp_read(): Failed!\n");
		return -1;
	}
	printf("POWER20_DC3OUT_VOL is set to 0x%02x\n", reg_val);
	return 0;
}



static int do_set_axp_debug (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	axp_debug = simple_strtol(argv[1], NULL, 10);
	
	printf("axp_debug: %d\n", axp_debug);
	return 0;
}


U_BOOT_CMD(
	set_axp_debug,	2,	0,	do_set_axp_debug,
	"set axp debug",
	"/N\n"
	"set axp debug <level>\n"
	"0-7\n"
);

#ifdef CONFIG_UBOOT_BATTERY_PARAMETER_TEST
#warning >>>>>>>>>>>>>>> CONFIG_UBOOT_BATTERY_PARAMETER_TEST
////////////////////////////////////////////////////////////////////////
//     This code is add for battery curve calibrate                   //
////////////////////////////////////////////////////////////////////////
// charecter attribute
#define     NoneAttribute       0   
#define     BoldFace            1   
#define     UnderLine           4   
#define     Flicker             5   
#define     ReverseDisplay      7   
#define     Hide                8   
#define     Un_BoldFace         22  
#define     Un_Flicker          25  
#define     Un_ReverseDisplay   27  

// background color
#define     Back_Black          40 
#define     Back_Red            41 
#define     Back_Yellow         43 
#define     Back_Blue           44 
#define     Back_Purple         45 
#define     Back_BottleGreen    46 
#define     Back_White          47

// front color
#define     Font_Black          30
#define     Font_Red            31
#define     Font_Green          42
#define     Font_Yellow         43
#define     Font_Blue           44
#define     Font_Purple         45
#define     Font_BottleGreen    46
#define     Font_White          47

void terminal_print(int x, int y, char *str)
{
    char   buff[200] = {};
    if (y == 35) {
        sprintf(buff,
                "\033[%d;%dH\033[0;%d;%dm%s\033[0m",
                y, x, Back_Black, Font_Black, 
                "                                                     "
                "                                                     ");
        printf(buff);
        sprintf(buff,
                "\033[%d;%dH\033[1;%d;%dm%s\033[0m",
                y, x, Back_Black, Font_Red, str);
    } else {
        sprintf(buff,
                "\033[%d;%dH\033[0;%d;%dm%s\033[0m",
                y, x, Back_Black, Font_Green, str);
    }
    printf(buff);
}

int axp_battery_calibrate_init(void)
{
    uint8_t val;
    axp_read(0x01, &val);
    if (!(val & 0x20)) {
        terminal_print(0, 35, "ERROR, NO battery is connected to system\n");
        return 0;
    }
    axp_read(0x84, &val);
    val &= ~0xc0;
    val |= 0x80;
    axp_write(0x84, val);                       // set ADC sample rate to 100KHz
    axp_write(0x82, 0xff);                      // open all ADC
    axp_write(0x31, 0x03);                      // shutdown when battery voltage < 2.9V
    axp_write(0x33, 0xc8);                      // set charge current to 1.1A
    axp_write(0xb8, 0x20);                      // clear coulomb counter
    axp_write(0xb8, 0x80);                      // start coulomb counter
    return 1;
}

int32_t coulomb = 0;
int32_t ocv  = 0;
int32_t ibat = 0;
int32_t rdc_r  = 0;
int32_t vbat_i = 0;

int axp_calculate_rdc(void)
{
    struct axp_adc_res axp_adc;
    char    buf[100];
    int32_t i_lo, i_hi;
    int32_t v_lo, v_hi;
    int32_t rdc_cal = 0;

    if (ocv > 4000) {                           // don't calculate rdc when ocv is too high
        return 0;
    }
    axp_write(0x33, 0xc1);                      // set charge current to 400mA 
    udelay(500000);
    axp_read_adc(&axp_adc);
    i_lo = ABS(axp_ibat_to_mA(axp_adc.ichar_res)-axp_ibat_to_mA(axp_adc.idischar_res)); 
    v_lo = (axp_adc.vbat_res * 1100) / 1000;
    axp_write(0x33, 0xc9);                      // set charge current to 1.2A
    udelay(500000);
    axp_read_adc(&axp_adc);
    i_hi = ABS(axp_ibat_to_mA(axp_adc.ichar_res)-axp_ibat_to_mA(axp_adc.idischar_res)); 
    v_hi = (axp_adc.vbat_res * 1100) / 1000;
    rdc_cal = (v_hi - v_lo) * 1000 / (i_hi - i_lo);
    sprintf(buf, "i_lo:%4d, i_hi:%4d, u_lo:%4d, u_hi:%4d, rdc:%4d\n", i_lo, i_hi, v_lo, v_hi, rdc_cal);
    terminal_print(0, 36, buf);
    if (rdc_cal < 0 || rdc_cal >= 300) {        // usually RDC will not greater than 300 mhom
        return 0;
    }
    return rdc_cal;
}

int axp_update_calibrate(int charge)
{
    uint8_t val[2];
    struct axp_adc_res axp_adc;
    uint32_t tmp0, tmp1;

    coulomb = axp_get_coulomb(); 
    ocv = axp_get_ocv(); 
    axp_read_adc(&axp_adc);
    ibat = ABS(axp_ibat_to_mA(axp_adc.ichar_res)-axp_ibat_to_mA(axp_adc.idischar_res)); 
    tmp0 = axp_adc.vbat_res;
    vbat_i = (tmp0 * 1100) / 1000;

    axp_reads(0xBA, 2, val);
    tmp1 = ((val[0] & 0x1F) << 8) | val[1];
    rdc_r = (tmp1  * 10742) / 10000;
    if (charge) {
        return axp_calculate_rdc();
    }
    return 0;
}

static struct energy_array {
    int     ocv;                            // mV
    int     coulomb;                        // mAh read from axp202
    int     coulomb_p;                      // mAh @ 3700mV
    int64_t energy;                         // mV * mAh
    int     updated_flag;           
};

static struct energy_array battery_energy_charge[16] = {
    {3132, 0, 0, 0, 0},                     // pmu_bat_para1
    {3273, 0, 0, 0, 0},                     // pmu_bat_para2
    {3414, 0, 0, 0, 0},                     // pmu_bat_para3
    {3555, 0, 0, 0, 0},                     // pmu_bat_para4
    {3625, 0, 0, 0, 0},                     // pmu_bat_para5
    {3660, 0, 0, 0, 0},                     // pmu_bat_para6
    {3696, 0, 0, 0, 0},                     // pmu_bat_para7
    {3731, 0, 0, 0, 0},                     // pmu_bat_para8
    {3766, 0, 0, 0, 0},                     // pmu_bat_para9
    {3801, 0, 0, 0, 0},                     // pmu_bat_para10
    {3836, 0, 0, 0, 0},                     // pmu_bat_para11
    {3872, 0, 0, 0, 0},                     // pmu_bat_para12
    {3942, 0, 0, 0, 0},                     // pmu_bat_para13
    {4012, 0, 0, 0, 0},                     // pmu_bat_para14
    {4083, 0, 0, 0, 0},                     // pmu_bat_para15
    {4153, 0, 0, 0, 0}                      // pmu_bat_para16   
};

static struct energy_array battery_energy_discharge[16] = {
    {3132, 0, 0, 0, 0},                     // pmu_bat_para1
    {3273, 0, 0, 0, 0},                     // pmu_bat_para2
    {3414, 0, 0, 0, 0},                     // pmu_bat_para3
    {3555, 0, 0, 0, 0},                     // pmu_bat_para4
    {3625, 0, 0, 0, 0},                     // pmu_bat_para5
    {3660, 0, 0, 0, 0},                     // pmu_bat_para6
    {3696, 0, 0, 0, 0},                     // pmu_bat_para7
    {3731, 0, 0, 0, 0},                     // pmu_bat_para8
    {3766, 0, 0, 0, 0},                     // pmu_bat_para9
    {3801, 0, 0, 0, 0},                     // pmu_bat_para10
    {3836, 0, 0, 0, 0},                     // pmu_bat_para11
    {3872, 0, 0, 0, 0},                     // pmu_bat_para12
    {3942, 0, 0, 0, 0},                     // pmu_bat_para13
    {4012, 0, 0, 0, 0},                     // pmu_bat_para14
    {4083, 0, 0, 0, 0},                     // pmu_bat_para15
    {4153, 0, 0, 0, 0}                      // pmu_bat_para16   
};

static int32_t ocv_array[4] = {};

static int32_t update_ocv(int32_t ocv)
{
    int32_t i = 0;
    int32_t total = ocv * 4;

    for (i = 0; i < 3; i++) {
        total += ocv_array[i + 1] * (i+1);
        ocv_array[i] = ocv_array[i+1];
    }
    ocv_array[3] = ocv;
    return total / 10;
}

static void inline update_energy_charge(int ocv, int energy, int coulomb, int coulomb_p) 
{
    int i = 0;    
    char    buf[100] = {};
    for (i = 0; i < 16; i++) {
        if (ocv >= battery_energy_charge[i].ocv && !battery_energy_charge[i].updated_flag) {
            battery_energy_charge[i].energy = energy;
            battery_energy_charge[i].coulomb = coulomb;
            battery_energy_charge[i].coulomb_p = coulomb_p;
            battery_energy_charge[i].updated_flag = 1;
          //sprintf(buf, "update energy %9lld for %4d mV, index:%2d\n", 
          //        battery_energy_charge[i].energy, battery_energy_charge[i].ocv, i);
          //terminal_print(0, 35, buf);
            sprintf(buf, "%2d,  %4d,  %9lld,  %4d,  %4d,\n", 
                    i,
                    battery_energy_charge[i].ocv, 
                    battery_energy_charge[i].energy, 
                    battery_energy_charge[i].coulomb,
                    battery_energy_charge[i].coulomb_p);
            terminal_print(0, 12 + i, buf);
        }
    }
}

static void inline update_energy_discharge(int ocv, int energy, int coulomb, int coulomb_p) 
{
    int i = 0;
    char    buf[100] = {};
    for (i = 0; i < 16; i++) {
        if (ocv < battery_energy_discharge[i].ocv && !battery_energy_discharge[i].updated_flag) {
            battery_energy_discharge[i].energy = energy;
            battery_energy_discharge[i].coulomb = coulomb;
            battery_energy_discharge[i].coulomb_p = coulomb_p;
            battery_energy_discharge[i].updated_flag = 1;
          //sprintf(buf, "update energy %9lld for %4d mV, index:%2d\n", 
          //       battery_energy_discharge[i].energy, battery_energy_discharge[i].ocv, i);
          //terminal_print(0, 35, buf);
            sprintf(buf, "%2d,  %4d,  %9lld,  %4d,  %4d,\n", 
                    i,
                    battery_energy_discharge[i].ocv, 
                    battery_energy_discharge[i].energy, 
                    battery_energy_discharge[i].coulomb,
                    battery_energy_discharge[i].coulomb_p);
            terminal_print(60, 12 + i, buf);
        }
    }
}

void  ClearScreen(void)                                 // screen clear for terminal
{
    char    buff[15] = {};
    int     length=0;
                
    sprintf(buff, "\033[2J\033[0m");
    while (buff[length] != '\0') { 
        length ++;
    }
    printf(buff);
}

void axp_set_rdc(int rdc)
{
    uint32_t rdc_tmp = (rdc * 10000 + 5371) / 10742;
    char    buf[100];

    axp_set_bits(0xB9, 0x80);                           // stop
    axp_clr_bits(0xBA, 0x80);
    axp_write(0xBB, rdc_tmp & 0xff);
    axp_write(0xBA, (rdc_tmp >> 8) & 0x1F);
    axp_clr_bits(0xB9, 0x80);                           // start
}

extern void set_backlight_level(unsigned level);

int axp_battery_calibrate(void)
{
    int64_t energy_c = 0;
    int64_t energy_p = 0;
    int     prev_coulomb = 0;
    int     prev_ocv  = 0;
    int     prev_ibat = 0;
    int     key;
    int     ibat_cnt = 0;
    int     i;
    int64_t energy_top, energy_visible;
    int     base, offset, range_charge, percent, range_discharge;
    char    buf[200] = {};
    int     size;
    int     rdc_average = 0, rdc_total = 0, rdc_cnt = 0, rdc_update_flag = 0;
    int     ocv_0 = 2;
    int     rdc_tmp = 0;

    ClearScreen();
    terminal_print(0,  7, "=============================== WARNING ================================\n");
    terminal_print(0,  8, "Battery calibrate will take several hours to finish. Before calibrate,  \n");
    terminal_print(0,  9, "make sure you have discharge battery with voltage between to 3.0V ~ 3.05V.\n");
    terminal_print(0, 10, "during test, you can press key 'Q' to quit this process.\n");
    terminal_print(0, 11, "'R' = run calibration, 'Q' = quit. Your Choise:\n");

    while (1) {
        if (tstc()) {
            key = getc();
            if (key == 'r' || key == 'R') {
                break;
            }
            if (key == 'q' || key == 'Q') {
                goto out;
            }
        }
        udelay(10000);
    } 

    /*
     * Note: If you observed rdc readed from register had large 
     * different with rdc calculated(more than +-15mohm), you should
     * reset rdc with calculated value and redo this test.
     */
    prev_ocv = axp_get_ocv(); 
    axp_read(0x00, buf);
    if ((buf[0] & 0x50) && prev_ocv < 3800) {
        terminal_print(0, 13, "Calibrate RDC now...\n");
        for (i = 0; i < 10; i++) {
            rdc_tmp = axp_calculate_rdc();
            if (rdc_tmp) {
                rdc_total += rdc_tmp;
                rdc_cnt++;
            }
        }
        if (rdc_cnt) {
            rdc_average = rdc_total / rdc_cnt;
            sprintf(buf, "RDC set to %d mohm\n", rdc_average);
            terminal_print(0, 35, buf);
            axp_set_rdc(rdc_average);           // update your calulated RDC here
        } else {
            terminal_print(0, 35, "WRONG with rdc calculate, we stop this test now!!!\n");     
            goto out; 
        }
    }

    if (!axp_battery_calibrate_init()) {
        goto out;
    }
    ClearScreen(); 
    terminal_print(0, 1, "'Q' = quit, 'S' = Skip this step\n");
    terminal_print(0, 4, "coulomb     energy_c    ibat   prev_ibat    ocv"
                         "     prev_ocv    coulomb_p   vbat    rdc\n");
    axp_update_calibrate(0); 
    prev_coulomb = coulomb;
    prev_ocv = ocv;
    prev_ibat = ibat;
    for (i = 0; i < 4; i++) {
        ocv_array[i] = prev_ocv;    
    }
    while (1) {
        if (tstc()) {
            key = getc();
            if (key == 'Q' || key == 'q') {
                terminal_print(0, 35, "You have aborted calibrate manually\n");
                goto out;
            }
            if (key == 'S' || key == 's') {
                terminal_print(0, 35, "Skip charging calibrate\n");
                break;
            }
        }
        rdc_tmp = axp_update_calibrate(1);
        if (rdc_tmp) {
            rdc_total += rdc_tmp; 
            rdc_cnt++;
        }
        if (ocv > 3520 && !rdc_update_flag) {
            if (rdc_cnt) {
                rdc_average = rdc_total / rdc_cnt; 
                axp_set_rdc(rdc_average); 
                sprintf(buf, "RDC set to %d mohm, rdc_total:%d, cnt:%d\n", rdc_average, rdc_total, rdc_cnt);
                terminal_print(0, 35, buf);
                rdc_update_flag = 1;
            } else {
                terminal_print(0, 35, "WRONG with rdc calculate, we stop this test now!!!\n");     
                goto out; 
            }
        }
        energy_c += (coulomb - prev_coulomb) * ((ocv + prev_ocv) / 2);
        energy_p = energy_c;
        do_div(energy_p, 3700);
        update_energy_charge(update_ocv(ocv), energy_c, coulomb, energy_p);
        size  = sprintf(buf, 
                        "%4d,   %12lld,   %4d,       %4d,  %4d,        %4d,",
                        coulomb, energy_c, ibat, prev_ibat, ocv, prev_ocv);
        size += sprintf(buf + size,
                        "        %4d,  %4d,  %4d\n",
                        (int32_t)energy_p, vbat_i, rdc_r);
        buf[size] = '\0';
        terminal_print(0, 5, buf);
        prev_coulomb = coulomb;
        prev_ocv = ocv;
        prev_ibat = ibat;
        udelay(1000000);
        if (ibat == 0) {                        // charging finished
            ibat_cnt++;
            if (ibat_cnt > 50) {
                break;
            }
        }
    }
    size = sprintf(buf, "During charge, rdc_total=%d, rdc_cnt:%d, rdc_average:%d\n", 
                   rdc_total, rdc_cnt, rdc_total / rdc_cnt);
    terminal_print(0, 36, buf);

    energy_top = energy_c;
    terminal_print(0, 10, "============= RESULT FOR CHARGE ================\n");
    terminal_print(0, 11, "i,    ocv,     energy,     c,   c_e,   off,    %%\n");
    offset = battery_energy_charge[15].coulomb_p - battery_energy_charge[2].coulomb_p;
    i = (battery_energy_charge[3].coulomb_p - battery_energy_charge[2].coulomb_p) * 100;
    if ((i / offset) >= 3) {
        ocv_0 = 2; 
        terminal_print(0, 35, "We set zero reference ocv to 3414mV\n");
    } else {
        ocv_0 = 3;    
        terminal_print(0, 35, "We set zero reference ocv to 3555mV\n");
    }
    base = battery_energy_charge[ocv_0].coulomb_p;
    range_charge = battery_energy_charge[15].coulomb_p - base;
    for (i = 0; i < 16; i++) {
        energy_p = battery_energy_charge[i].energy * 100;
        if (i <= ocv_0) {
            offset  = 0;
            percent = 0;
        } else {
            offset = battery_energy_charge[i].coulomb_p - base;
            percent = 100 * (offset + range_charge / 200) / range_charge;
        }
        size = sprintf(buf, "%2d,  %4d,  %9lld,  %4d,  %4d,  %4d,  %3d\n", 
                       i,
                       battery_energy_charge[i].ocv, 
                       battery_energy_charge[i].energy, 
                       battery_energy_charge[i].coulomb,
                       battery_energy_charge[i].coulomb_p,
                       offset,
                       percent);
        buf[size] = '\0';
        terminal_print(0, 12 + i, buf);
    }
    energy_p = energy_top;
    do_div(energy_p, 3700);
    size = sprintf(buf, "Total charge energy:%9lld(mV * mAh) = %5dmAh@3700mV\n", 
                   energy_top, (int32_t)energy_p);
    buf[size] = '\0';
    terminal_print(0, 30, buf);
    size = sprintf(buf, "Energy visible:%5dmAh@3700mV, percent:%2d\n", 
                   range_charge, range_charge * 100 / (int32_t)energy_p);
    buf[size] = '\0';
    terminal_print(0, 31, buf);

    /*
     * test for discharge
     */
    terminal_print(0, 35, "Please unplug DC power, then press 'R' to contine\n");
    while (1) {
        if (tstc()) {
            key = getc();
            if (key == 'r' || key == 'R') {
                break;
            }
            if (key == 'q' || key == 'Q') {
                goto out;
            }
        }
        udelay(10000);
    } 

    terminal_print(0, 35, "do discharge calibration now, please don't plug DC power during test!\n");
    set_backlight_level(255);
    energy_c = 0;
    while (1) {
        if (tstc()) {
            key = getc();
            if (key == 'Q' || key == 'q') {
                terminal_print(0, 35, "You have aborted calibrate manually\n");
                goto out;
            }
            if (key == 'S' || key == 's') {
                terminal_print(0, 35, "Skip discharging calibrate\n");
                break;
            }
        }
        axp_update_calibrate(0); 
        energy_c += (prev_coulomb - coulomb) * ((ocv + prev_ocv) / 2);
        energy_p = energy_c;
        do_div(energy_p, 3700);
        update_energy_discharge(update_ocv(ocv), energy_c, coulomb, energy_p);
        size  = sprintf(buf, 
                        "%4d,   %12lld,   %4d,       %4d,  %4d,        %4d,",
                        coulomb, energy_c, ibat, prev_ibat, ocv, prev_ocv);
        size += sprintf(buf + size,
                        "        %4d,  %4d,  %4d\n",
                        (int32_t)energy_p, vbat_i, rdc_r);
        buf[size] = '\0';
        terminal_print(0, 5, buf);
        prev_coulomb = coulomb;
        prev_ocv = ocv;
        prev_ibat = ibat;
        udelay(1000000);
        if (ocv < 3350) {
            terminal_print(0, 35, "ocv is too low, we stop discharging test now!\n");
            break;
        }
    }

    energy_top = energy_c;
    terminal_print(60, 10, "============ RESULT FOR DISCHARGE ==============\n");
    terminal_print(60, 11, "i,    ocv,     energy,     c,   c_e,   off,    %%\n");
    base = battery_energy_discharge[15].coulomb_p;
    range_discharge = battery_energy_discharge[ocv_0].coulomb_p - base;
    for (i = 0; i < 16; i++) {
        energy_p = battery_energy_discharge[i].energy * 100;
        if (i < ocv_0) {
            offset  = 0;
            percent = 0;
        } else {
            offset = battery_energy_discharge[i].coulomb_p - base;
            percent = 100 - 100 * (offset + range_discharge / 200) / range_discharge;
        }
        size = sprintf(buf, "%2d,  %4d,  %9lld,  %4d,  %4d,  %4d,  %3d\n", 
                       i,
                       battery_energy_discharge[i].ocv, 
                       battery_energy_discharge[i].energy, 
                       battery_energy_discharge[i].coulomb,
                       battery_energy_discharge[i].coulomb_p,
                       offset,
                       percent);
        buf[size] = '\0';
        terminal_print(60, 12 + i, buf);
    }
    energy_p = energy_top;
    do_div(energy_p, 3700);
    size = sprintf(buf, "Energy visible:%5dmAh@3700mV\n", range_discharge);
    buf[size] = '\0';
    terminal_print(60, 30, buf);
    size = sprintf(buf, "Charging efficient:%d%%\n", (100 * (range_discharge + range_charge / 200)) / range_charge);
    buf[size] = '\0';
    terminal_print(60, 31, buf);

out:
    terminal_print(0, 38, "\n\n");
    return 1;
}
#endif

