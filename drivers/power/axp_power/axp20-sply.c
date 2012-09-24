#include <common.h>
#include <div64.h>
#include "axp-sply.h"

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


