#include <config.h>
#include <common.h>
#include <asm/arch/io.h>
#include <command.h>
#include <malloc.h>
#include <amlogic/efuse.h>

int do_calinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char buf[EFUSE_BYTES];
#ifdef CONFIG_AML_MESON_8	
	memset(buf, 0, sizeof(buf));
	
	if(argc < 2){
		#define THERMAL_CAL_FLAG_OFF 503
		#define CVBS_FLAG_OFF 505
		printf("Module:                    Calibrated?\n");
		printf("---------------------------------------\n");
		memcpy(buf, efuse_dump(), EFUSE_BYTES);
		printf("thermal_sensor                ");
		if (buf[THERMAL_CAL_FLAG_OFF] & 1 << 7)
			printf("Yes\n");
		else
			printf("No\n");
		printf("cvbs                          ");
		if (buf[CVBS_FLAG_OFF] >> 6 ==  2)
			printf("Yes\n");
		else
			printf("No\n");
	}
#endif

	return 0;
	
}

U_BOOT_CMD(
	calinfo,	1,	1,	do_calinfo,
	"calinfo£ºprint the chip calibration info",
	"calinfo\n"
	"print the chip calibration info;\n"
);
