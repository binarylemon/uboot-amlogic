#include <config.h>
#include <common.h>
#include <asm/arch/io.h>
#include <command.h>
#include <malloc.h>
#include <amlogic/gpio.h>

int do_gpio(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int pin;
	int i;
	for(i=0;i<argc;i++)
		printf("%s\n",argv[i]);
	printf("argc=%d\n",argc);
	pin=gpioname_to_pin(argv[1]);
	if(argc<3||argc>5)
		goto out;
	printf("pin=%d\n",pin);
	if(!strcmp(argv[2],"out"))
	{
		if(!strcmp(argv[3],"high"))
		{
			amlogic_gpio_direction_output(pin,1);
			if(argv[4]){
				if(!strcmp(argv[4],"pp"))
					amlogic_set_pull_up(pin,1);
				else if (!strcmp(argv[4],"pd"))
					amlogic_set_pull_up(pin,0);
				else
					goto out;
			}
		}
		else if(!strcmp(argv[3],"low"))
		{
			amlogic_gpio_direction_output(pin,0);
			if(argv[4]){
				if(!strcmp(argv[4],"pp"))
					amlogic_set_pull_up(pin,1);
				else if (!strcmp(argv[4],"pd"))
					amlogic_set_pull_up(pin,0);
				else
					goto out;
			}
		}
		else
			goto out;
	}
	else if(!strcmp(argv[2],"in"))
	{
		amlogic_gpio_direction_input(pin);
		if(argv[3]){
			if(!strcmp(argv[3],"pp"))
				amlogic_set_pull_up(pin,1);
			else if (!strcmp(argv[3],"pd"))
				amlogic_set_pull_up(pin,0);
			else
				goto out;
		}
	}
	else
		goto out;
	return 0;
out:
		printf ("Unknown operation\n");
		return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	gset,	5,	1,	do_gpio,
	"gpio commands",
	"GPIONAME out high/low [pp/pd]\n"
	"gset GPIONAME in [pp/pd]\n"
);

/****************************************************/

