#ifdef	CONFIG_SARADC_WAKEUP_FOR_ARC
static __inline__ void aml_set_reg32_bits(unsigned int _reg, const unsigned int _value, const unsigned int _start, const unsigned int _len)
{
	writel(( (readl((volatile void *)_reg) & ~((( 1L << (_len) )-1) << (_start))) | ((unsigned)((_value)&((1L<<(_len))-1)) << (_start))), (volatile void *)_reg );
}

static __inline__ unsigned int aml_get_reg32_bits(unsigned int _reg, const unsigned int _start, const unsigned int _len)
{
	return	( (readl((volatile void *)_reg) >> (_start)) & (( 1L << (_len) ) - 1) );
}

static __inline__ void aml_write_reg32( unsigned int _reg, const unsigned int _value)
{
	writel( _value,(volatile void *)_reg );
};

static __inline__ unsigned int aml_read_reg32(unsigned int _reg)
{
	return readl((volatile void *)_reg);
};


/**********************************************************************************/
enum {
	NO_AVG_MODE = 0,
	SIMPLE_AVG_MODE,
	MEDIAN_AVG_MODE,
};

enum {
	SAMPLE_NUM_1 = 0,
	SAMPLE_NUM_2,
	SAMPLE_NUM_4,
	SAMPLE_NUM_8
};

enum {
	BLOCK_DELAY_TB_1US = 0,	/* count 1us ticks */
	BLOCK_DELAY_TB_10US,		/* count 10us ticks */
	BLOCK_DELAY_TB_100US,		/* count 100us ticks */
	BLOCK_DELAY_TB_1MS,		/* count 1ms ticks */
};
/* ADC AINC sample mode */
enum {
	DIFF_MODE = 0,			/* differential mode */
	SINGLE_ENDED_MODE,		/* single ended */
};

enum {
	SAMPLE_DELAY_TB_1US = 0,	/* count 1us ticks */
	SAMPLE_DELAY_TB_10US,		/* count 10us ticks */
	SAMPLE_DELAY_TB_100US,	/* count 100us ticks */
	SAMPLE_DELAY_TB_1MS,		/* count 1ms ticks */
};

enum {
	INPUT_DELAY_TB_110NS = 0,		/* count 110ns ticks */
	INPUT_DELAY_TB_1US,			/* count 1us ticks */
	INPUT_DELAY_TB_10US,			/* count 10us ticks */
	INPUT_DELAY_TB_100US,			/* count 100us ticks */
};

//REG0
#define delta_busy()					aml_get_reg32_bits(P_AO_SAR_ADC_REG0, 30, 1)
#define avg_busy()						aml_get_reg32_bits(P_AO_SAR_ADC_REG0, 29, 1)
#define sample_busy()					aml_get_reg32_bits(P_AO_SAR_ADC_REG0, 28, 1)
#define get_fifo_cnt()					aml_get_reg32_bits(P_AO_SAR_ADC_REG0, 21, 5)
#define stop_sample()					aml_set_reg32_bits(P_AO_SAR_ADC_REG0, 1, 14, 1)
#define disable_chan1_delta()			aml_set_reg32_bits(P_AO_SAR_ADC_REG0, 0, 13, 1)
#define disable_chan0_delta()			aml_set_reg32_bits(P_AO_SAR_ADC_REG0, 0, 12, 1)
#define set_detect_irq_pol(pol)		aml_set_reg32_bits(P_AO_SAR_ADC_REG0, pol, 10, 1)
#define disable_detect_irq()			aml_set_reg32_bits(P_AO_SAR_ADC_REG0, 0, 9, 1)
#define disable_fifo_irq()				aml_set_reg32_bits(P_AO_SAR_ADC_REG0, 0, 3, 1)
#define start_sample()					aml_set_reg32_bits(P_AO_SAR_ADC_REG0, 1, 2, 1)
#define disable_continuous_sample()	aml_set_reg32_bits(P_AO_SAR_ADC_REG0, 0, 1, 1)
#define enable_sample_engine()		aml_set_reg32_bits(P_AO_SAR_ADC_REG0, 1, 0, 1)
#define disable_sample_engine()		aml_set_reg32_bits(P_AO_SAR_ADC_REG0, 0, 0, 1)
//REG3
#define set_chan_list(list, len)		aml_write_reg32(P_AO_SAR_ADC_CHAN_LIST, list | ((len-1)<<24))

#define set_avg_mode(ch, mode, num) do {\
	aml_set_reg32_bits(P_AO_SAR_ADC_AVG_CNTL, num, ch*2, 2);\
	aml_set_reg32_bits(P_AO_SAR_ADC_AVG_CNTL, mode, ch*2+16, 2);\
} while(0)

#define set_block_delay(delay, timebase) do {\
	aml_set_reg32_bits(P_AO_SAR_ADC_REG3, delay, 0, 8);\
	aml_set_reg32_bits(P_AO_SAR_ADC_REG3, timebase, 8, 2);\
} while(0)

/* Enable/disable ADC */
#define enable_adc()					aml_set_reg32_bits(P_AO_SAR_ADC_REG3, 1, 21, 1)   
#define disable_adc()					aml_set_reg32_bits(P_AO_SAR_ADC_REG3, 0, 21, 1)
#define disable_detect_pullup()		aml_set_reg32_bits(P_AO_SAR_ADC_REG3, 0, 22, 1)
#define set_sample_mode(mode)			aml_set_reg32_bits(P_AO_SAR_ADC_REG3, mode, 23, 1)
#define set_cal_voltage(sel)			aml_set_reg32_bits(P_AO_SAR_ADC_REG3, sel, 23, 3)
/* TEMPSEN_PD12, TEMPSEN_MODE */
#define set_tempsen(val)				aml_set_reg32_bits(P_AO_SAR_ADC_REG3, val, 28, 2)
// REG9
#define set_idle_sw(sw)				aml_set_reg32_bits(P_AO_SAR_ADC_DETECT_IDLE_SW, sw, 0, 7)
#define set_idle_mux(mux)				aml_set_reg32_bits(P_AO_SAR_ADC_DETECT_IDLE_SW, mux, 7, 3)
#define set_detect_sw(sw)				aml_set_reg32_bits(P_AO_SAR_ADC_DETECT_IDLE_SW, sw, 16, 7)
#define set_detect_mux( mux)			aml_set_reg32_bits(P_AO_SAR_ADC_DETECT_IDLE_SW, mux, 23, 3)
#define disable_detect_sw()			aml_set_reg32_bits(P_AO_SAR_ADC_DETECT_IDLE_SW, 0, 26, 1)
// REG10
#define enable_temp__()    			aml_set_reg32_bits(P_AO_SAR_ADC_DELTA_10, 1, 15, 1)
#define enable_temp()      			aml_set_reg32_bits(P_AO_SAR_ADC_DELTA_10, 1, 26, 1)
#define select_temp()       			aml_set_reg32_bits(P_AO_SAR_ADC_DELTA_10, 1, 27, 1)
// REG11
#define enable_bandgap()   			aml_set_reg32_bits(P_AO_SAR_ADC_REG11, 1, 13, 1)
/* The ADC clock is derived by dividing  */
#define set_clock_src(src) 			aml_set_reg32_bits(P_AO_SAR_CLK, src, 9, 2)
#define set_clock_divider(div) 		aml_set_reg32_bits(P_AO_SAR_CLK, div, 0, 8)
/* enable/disable  the SAR ADC clock */
#define enable_clock()					aml_set_reg32_bits(P_AO_SAR_CLK, 1, 8, 1)
//FIFO
#define get_fifo_sample()				aml_read_reg32(P_AO_SAR_ADC_FIFO_RD)

/* set_sample_delay() - set sample delay
 * For channels that acquire 2,4 or 8 samples, there is a delay between two samples */
#define set_sample_delay(delay, timebase) do {\
	aml_set_reg32_bits(P_AO_SAR_ADC_DELAY, delay, 0, 8);\
	aml_set_reg32_bits(P_AO_SAR_ADC_DELAY, timebase, 8, 2);\
} while(0)

#define set_input_delay(delay, timebase) do {\
	aml_set_reg32_bits(P_AO_SAR_ADC_DELAY, delay, 16, 8);\
	aml_set_reg32_bits(P_AO_SAR_ADC_DELAY, timebase, 24, 2);\
} while(0)

#define set_sample_sw(ch, sw) do {\
	if (ch < 2)\
		aml_set_reg32_bits(P_AO_SAR_ADC_CHAN_10_SW, sw, ch*16, 7);\
	else\
		aml_set_reg32_bits(P_AO_SAR_ADC_AUX_SW, sw, 0, 7);\
} while(0)

#define set_sample_mux(ch, mux) do {\
	if (ch < 2)\
		aml_set_reg32_bits(P_AO_SAR_ADC_CHAN_10_SW, mux, ch*16 + 7, 3);\
	else\
		aml_set_reg32_bits(P_AO_SAR_ADC_AUX_SW, mux, (ch-2) * 3 + 8, 3);\
} while(0)

#define XN_OFF		(0<<0)
#define YN_OFF		(0<<1)
#define YN_ON		(1<<1)
#define XP_OFF		(1<<2)
#define YP_OFF		(1<<3)

#define MODE_SEL(sel)		(sel<<4)
#define VREF_N_MUX(mux)	(mux<<5)
#define VREF_P_MUX(mux)	(mux<<6)

#define IDLE_SW		(XP_OFF | XN_OFF | YP_OFF | YN_OFF\
					| MODE_SEL(0) | VREF_N_MUX(0) | VREF_P_MUX(0))
#define DETECT_SW	(XP_OFF | XN_OFF | YP_OFF | YN_ON\
					| MODE_SEL(0) | VREF_N_MUX(0) | VREF_P_MUX(0))				
/**********************************************************************************/
enum{AML_ADC_CHAN_0 = 0, AML_ADC_CHAN_1, AML_ADC_CHAN_2, AML_ADC_CHAN_3,
	 AML_ADC_CHAN_4,	 AML_ADC_CHAN_5, AML_ADC_CHAN_6, AML_ADC_CHAN_7,
	 AML_ADC_SARADC_CHAN_NUM,
};

static unsigned char g_chan_mux[AML_ADC_SARADC_CHAN_NUM] = {0,1,2,3,4,5,6,7};

static int adc_key_value[] = {
		CONFIG_SARADC_POWER_UP_KEY_VAL1,
		CONFIG_SARADC_POWER_UP_KEY_VAL2,
};

static void saradc_enable(void)
{
	int i;
	
	enable_bandgap();
	set_clock_src(0); //0-xtal, 1-clk81
	//set adc clock as 1.28Mhz @sys=27MHz
	set_clock_divider(20);
	enable_clock();
	enable_adc();

	set_sample_mode(DIFF_MODE);
	set_tempsen(0);
	disable_fifo_irq();
	disable_continuous_sample();
	disable_chan0_delta();
	disable_chan1_delta();

	set_input_delay(10, INPUT_DELAY_TB_1US);
	set_sample_delay(10, SAMPLE_DELAY_TB_1US);
	set_block_delay(10, BLOCK_DELAY_TB_1US);
	
	// channels sampling mode setting
	for(i=0; i<AML_ADC_SARADC_CHAN_NUM; i++) {
		set_sample_sw(i, IDLE_SW);
		set_sample_mux(i, g_chan_mux[i]);
	}
	
	// idle mode setting
	set_idle_sw(IDLE_SW);
	set_idle_mux(g_chan_mux[AML_ADC_CHAN_0]);
	
	// detect mode setting
	set_detect_sw(DETECT_SW);
	set_detect_mux(g_chan_mux[AML_ADC_CHAN_0]);
	disable_detect_sw();
	disable_detect_pullup();
	set_detect_irq_pol(0);
	disable_detect_irq();
	set_cal_voltage(7);

	enable_sample_engine();

	select_temp();
	enable_temp();
	enable_temp__();
	udelay(1000);
}

static int saradc_disable(void)
{
	disable_adc();
	disable_sample_engine();
	
	return 0;
}

static int get_adc_sample(int chan)
{
	int count;
	int value = -1;
	int sum;
	
	set_chan_list(chan, 1);
	set_avg_mode(chan, NO_AVG_MODE, SAMPLE_NUM_8);
	set_sample_mux(chan, g_chan_mux[chan]);
	set_detect_mux(g_chan_mux[chan]);
	set_idle_mux(g_chan_mux[chan]); // for revb
	enable_sample_engine();
	start_sample();

	//count = aml_read_reg32(P_ISA_TIMERE); 
	count = 0;
	while (delta_busy() || sample_busy() || avg_busy()){
		if (++count > 10000){
			goto adc_sample_end;
			}
	}
	
    stop_sample();
    
    sum = 0;
    count = 0;
    value = get_fifo_sample();

	while (get_fifo_cnt()){
        value = get_fifo_sample() & 0x3ff;
		sum += value;
        count++;
	}	
	value =(count) ? (value) : (-1);

adc_sample_end:
	disable_sample_engine();
	
	return value;
}

int  adc_detect_key()
{
 	int value , i;
	int key_tolerance = CONFIG_SARADC_KEY_TOLERANCE;
	
	value = get_adc_sample(CONFIG_SARADC_CHANEL_CNT);
	if(value < 0 || value > 0x3ff)
		return -1;
	for(i = 0; i < (sizeof(adc_key_value) / sizeof(unsigned int)); i++){
		if((value >= adc_key_value[i] - key_tolerance)
			&&(value <= adc_key_value[i] + key_tolerance) ){	
			return 1;
		}
	}
	
	return 0;
}
#endif

