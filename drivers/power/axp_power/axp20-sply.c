#include <common.h>
#include <div64.h>
#include "axp-sply.h"

#define DBG_AXP_PSY 1
#if  DBG_AXP_PSY
#define DBG_PSY_MSG(format,args...)   printf("[AXP]"format,##args)
#else
#define DBG_PSY_MSG(format,args...)   do {} while (0)
#endif

#define BATTERYCAP				3400							//battery capability

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

	axp_read(POWER20_DATA_BUFFER5, &val);
	DBG_PSY_MSG("base_cap = axp_read:%d\n",val);

	if((val & 0x80) >> 7)
		return (int) (val & 0x7F);
	else
		return 0;
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



int axp_charger_get_charging_percent()
{

	int rdc = 0,Cur_CoulombCounter = 0,base_cap = 0,bat_cap = 0, rest_vol;
	uint8_t val,v[2];

	axp_read(POWER20_DATA_BUFFER5, &val);
	DBG_PSY_MSG("base_cap = axp_read:%d\n",val);

	if((val & 0x80) >> 7)
	{

		Cur_CoulombCounter = axp_get_coulomb();

		DBG_PSY_MSG("%s->%d: charger->rest_vol > 100\n",__FUNCTION__,__LINE__);
		
		base_cap = axp_get_basecap();
		DBG_PSY_MSG("base_cap = axp_get_basecap(charger):%d\n",base_cap);
		bat_cap = BATTERYCAP;
		rest_vol = 100 * (base_cap * BATTERYCAP / 100 + Cur_CoulombCounter + BATTERYCAP/200) / BATTERYCAP;
		DBG_PSY_MSG("(val & 0x80) >> 7 = 1,rest_vol = :%d\n",rest_vol);
		if(rest_vol < 0)
		{
			rest_vol = 0;
		}
		if(rest_vol > 100)
		{
			rest_vol =100;
		}
		if(axp_charger_get_charging_status() && (rest_vol ==100))
		{
			rest_vol = 99;
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
    return rest_vol;
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



static int do_get_batcap (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int precent = axp_charger_get_charging_percent();
	printf("Battery CAP: %d\n", precent);
	setenv("battery_cap", "50");
	return 0;
}


U_BOOT_CMD(
	get_batcap,	1,	0,	do_get_batcap,
	"get battery capability",
	"/N\n"
	"This command will get battery capability\n"
	"capability will set to 'battery_cap'\n"
);

