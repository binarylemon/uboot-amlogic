/*
 * Command for Amlogic Custom.
 *
 * Copyright (C) 2013 Amlogic.
 * Cheng Wang <cheng.wang@amlogic.com>
 */

#include <common.h>
#include <command.h>

#define  USID_RANDOM_LENGTH_MAX           256
#define  MAC_RANDOM_LENGTH_MAX            12
#define  MAC_WIFI_RANDOM_LENGTH_MAX  12
#define  MAC_BT_RANDOM_LENGTH_MAX      12

enum random_type {
      NONE = -1,
      USID,
      MAC,
      MAC_WIFI,
      MAC_BT
};

static unsigned char random(int type)
{
     unsigned long long rand;
     unsigned char a;

     rand = get_timer(0) * 100000;
    
     // See "Numerical Recipes in C", second edition, p. 284
     rand = rand * 1664525L + 1013904223L;
     a = rand >> 24;                //not the lowest eight bit

     // Get '0~9, a ~ f, A ~ F' value
     if(a < 'A')
        a = a % 10 + 48;
     else if(a < 'F')
        a = a % 6 + 65;
     else if(a < 'a' || a > 'f')
        a = a % 6 + 97;

     if(type == USID) {
         return a;
     }
     else if(type >= MAC) {
         if(a>='A' && a<='F')
            a = a - 'A' + '0';
         else if(a>='a' && a<='f')
            a = a - 'a' + '0';
         return a;
     }
}


static void get_aml_usid_random(unsigned char *buf, size_t len)
{
     while (len--) {
	   *buf++ = random(USID);
         mdelay(1);
	}
}

static void get_aml_mac_random(unsigned char *buf, size_t len)
{
     while (len--) {
	   *buf++ = random(MAC);
         mdelay(1);
	}
}

static void get_aml_mac_wifi_random(unsigned char *buf, size_t len)
{
     while (len--) {
	   *buf++ = random(MAC_WIFI);
         mdelay(1);
	}
}

static void get_aml_mac_bt_random(unsigned char *buf, size_t len)
{
     while (len--) {
	   *buf++ = random(MAC_BT);
         mdelay(1);
	}
}


/*****************************
 *
 *     set_mac_bt_random
 *
 ****************************/
 static void set_mac_bt_random_help(void)
{
     printf(
         "command usage:\n"
         "  set_mac_bt_random\n"
         "  ---set 12 fixed length of numbers and the value is saved in mac_bt_random environment variable\n");
}

/**
 * command: set_mac_bt_random --set a mac_bt random number to environment variable
 * usage: set_mac_bt_random
 * if return 0 show sucessful, else failed.
 */
static int do_set_mac_bt_random (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
     size_t i = 0, j = 0;
     unsigned char random[MAC_BT_RANDOM_LENGTH_MAX];
     unsigned char random_str[MAC_BT_RANDOM_LENGTH_MAX*3];
     
     if(argc != 1 ||strncmp(argv[0], "set_mac_bt_random", strlen("set_mac_bt_random"))) {
         set_mac_bt_random_help();
         return -1;
     } 
  
     memset(random, 0, sizeof(random));
     memset(random_str, 0, sizeof(random_str));
     get_aml_mac_bt_random(random, MAC_BT_RANDOM_LENGTH_MAX);     // 0<= range <=0x9

#if 0
     for(i=0; i<MAC_BT_RANDOM_LENGTH_MAX; i++)
         printf("random[%d]=%c\n", i, random[i]);
#endif
   
     // save environment variable
     for(i=0; i<MAC_BT_RANDOM_LENGTH_MAX; i += 2) {
         sprintf(&random_str[j], "%c", random[i]);
         sprintf(&random_str[j+1], "%c", random[i+1]);
         if(i == MAC_BT_RANDOM_LENGTH_MAX-2)
            break;
         sprintf(&random_str[j+2], "%s", ":");
         j += 3;
     }

     setenv("mac_bt_random", random_str);
     
#if 1
     printf("set mac_bt_random=%s\n", getenv("mac_bt_random"));
#endif

	return 0;
}

U_BOOT_CMD(
	set_mac_bt_random,	2,	0,	do_set_mac_bt_random,
	"set a mac_bt random number to environment variable",
	"/N\n"
	"set_mac_bt_random\n"
	"This command will set mac_bt random number to mac_bt_random environment variable\n"
);



/*****************************
 *
 *     get_mac_bt_random
 *
 ****************************/
static void get_mac_bt_random_help(void)
{
     printf(
         "command usage:\n"
         "  get_mac_bt_random\n"
         "  ---get mac_bt random value\n");
}

/**
 * command: get_mac_bt_random --get mac_bt random environment variable value
 * usage: get_mac_bt_random
 * if return 0 show there is a mac_bt random number, else reurn 1 show there isn't.
 */
static inline int do_get_mac_bt_random (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
     char *mac_bt_random = NULL;
     
     if(argc != 1 || strncmp(argv[0], "get_mac_bt_random", strlen("get_mac_bt_random"))) {
         get_mac_bt_random_help();
         return -1;
     }

     mac_bt_random = getenv("mac_bt_random");
     if(mac_bt_random == NULL) { 
         printf("no mac_bt random\n");
         return 1;          // 1 is not exist   
     }
     else {
         printf("get mac_bt_random=%s\n", mac_bt_random);
         return 0;         // 0 is exist
     }
}

U_BOOT_CMD(
	get_mac_bt_random,	1,	0,	do_get_mac_bt_random,
	"get mac_bt_random environment variable value"
	"/N\n"	
	"get_mac_bt_random\n",	
	"This command will get mac_bt_random environment variable value'\n"
);



/*****************************
 *
 *     set_mac_wifi_random
 *
 ****************************/
 static void set_mac_wifi_random_help(void)
{
     printf(
         "command usage:\n"
         "  set_mac_wifi_random\n"
         "  ---set 12 fixed length of numbers and the value is saved in mac_wifi_random environment variable\n");
}

/**
 * command: set_mac_wifi_random --set a mac_wifi random number to environment variable
 * usage: set_mac_wifi_random
 * if return 0 show sucessful, else failed.
 */
static int do_set_mac_wifi_random (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
     size_t i = 0, j = 0;
     unsigned char random[MAC_WIFI_RANDOM_LENGTH_MAX];
     unsigned char random_str[MAC_WIFI_RANDOM_LENGTH_MAX*3];
     
     if(argc != 1 ||strncmp(argv[0], "set_mac_wifi_random", strlen("set_mac_wifi_random"))) {
         set_mac_wifi_random_help();
         return -1;
     } 
  
     memset(random, 0, sizeof(random));
     memset(random_str, 0, sizeof(random_str));
     get_aml_mac_wifi_random(random, MAC_WIFI_RANDOM_LENGTH_MAX);     // 0<= range <=0x9

#if 0
     for(i=0; i<MAC_WIFI_RANDOM_LENGTH_MAX; i++)
         printf("random[%d]=%c\n", i, random[i]);
#endif
   
     // save environment variable
     for(i=0; i<MAC_WIFI_RANDOM_LENGTH_MAX; i += 2) {
         sprintf(&random_str[j], "%c", random[i]);
         sprintf(&random_str[j+1], "%c", random[i+1]);
         if(i == MAC_WIFI_RANDOM_LENGTH_MAX-2)
            break;
         sprintf(&random_str[j+2], "%s", ":");
         j += 3;
     }

     setenv("mac_wifi_random", random_str);
     
#if 1
     printf("set mac_wifi_random=%s\n", getenv("mac_wifi_random"));
#endif

	return 0;
}

U_BOOT_CMD(
	set_mac_wifi_random,	2,	0,	do_set_mac_wifi_random,
	"set a mac_wifi random number to environment variable",
	"/N\n"
	"set_mac_wifi_random\n"
	"This command will set mac_wifi random number to mac_wifi_random environment variable\n"
);



/*****************************
 *
 *     get_mac_wifi_random
 *
 ****************************/
static void get_mac_wifi_random_help(void)
{
     printf(
         "command usage:\n"
         "  get_mac_wifi_random\n"
         "  ---get mac_wifi random value\n");
}

/**
 * command: get_mac_wifi_random --get mac_wifi random environment variable value
 * usage: get_mac_wifi_random
 * if return 0 show there is a mac_wifi random number, else reurn 1 show there isn't.
 */
static inline int do_get_mac_wifi_random (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
     char *mac_wifi_random = NULL;
     
     if(argc != 1 || strncmp(argv[0], "get_mac_wifi_random", strlen("get_mac_wifi_random"))) {
         get_mac_wifi_random_help();
         return -1;
     }

     mac_wifi_random = getenv("mac_wifi_random");
     if(mac_wifi_random == NULL) { 
         printf("no mac_wifi random\n");
         return 1;          // 1 is not exist   
     }
     else {
         printf("get mac_wifi_random=%s\n", mac_wifi_random);
         return 0;         // 0 is exist
     }
}

U_BOOT_CMD(
	get_mac_wifi_random,	1,	0,	do_get_mac_wifi_random,
	"get mac_wifi_random environment variable value"
	"/N\n"	
	"get_mac_wifi_random\n",	
	"This command will get mac_wifi_random environment variable value'\n"
);



/*****************************
 *
 *     set_mac_random
 *
 ****************************/
 static void set_mac_random_help(void)
{
     printf(
         "command usage:\n"
         "  set_mac_random\n"
         "  ---set 12 fixed length of numbers and the value is saved in mac_random environment variable\n");
}

/**
 * command: set_mac_random --set a mac random number to environment variable
 * usage: set_mac_random
 * if return 0 show sucessful, else failed.
 */
static int do_set_mac_random (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
     size_t i = 0, j = 0;
     unsigned char random[MAC_RANDOM_LENGTH_MAX];
     unsigned char random_str[MAC_RANDOM_LENGTH_MAX*3];
     
     if(argc != 1 ||strncmp(argv[0], "set_mac_random", strlen("set_mac_random"))) {
         set_mac_random_help();
         return -1;
     } 
  
     memset(random, 0, sizeof(random));
     memset(random_str, 0, sizeof(random_str));
     get_aml_mac_random(random, MAC_RANDOM_LENGTH_MAX);     // 0<= range <=0x9

#if 0
     for(i=0; i<MAC_RANDOM_LENGTH_MAX; i++)
         printf("random[%d]=%c\n", i, random[i]);
#endif
   
     // save environment variable
     for(i=0; i<MAC_RANDOM_LENGTH_MAX; i += 2) {
         sprintf(&random_str[j], "%c", random[i]);
         sprintf(&random_str[j+1], "%c", random[i+1]);
         if(i == MAC_RANDOM_LENGTH_MAX-2)
            break;
         sprintf(&random_str[j+2], "%s", ":");
         j += 3;
     }

     setenv("mac_random", random_str);
     
#if 1
     printf("set mac_random=%s\n", getenv("mac_random"));
#endif

	return 0;
}

U_BOOT_CMD(
	set_mac_random,	2,	0,	do_set_mac_random,
	"set a mac random number to environment variable",
	"/N\n"
	"set_mac_random\n"
	"This command will set mac random number to mac_random environment variable\n"
);



/*****************************
 *
 *     get_mac_random
 *
 ****************************/
static void get_mac_random_help(void)
{
     printf(
         "command usage:\n"
         "  get_mac_random\n"
         "  ---get mac random value\n");
}

/**
 * command: get_mac_random --get mac_random environment variable value
 * usage: get_mac_random
 * if return 0 show there is a mac random number, else reurn 1 show there isn't.
 */
static inline int do_get_mac_random (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
     char *mac_random = NULL;
     
     if(argc != 1 || strncmp(argv[0], "get_mac_random", strlen("get_mac_random"))) {
         get_mac_random_help();
         return -1;
     }

     mac_random = getenv("mac_random");
     if(mac_random == NULL) { 
         printf("no mac random\n");
         return 1;          // 1 is not exist   
     }
     else {
         printf("get mac_random=%s\n", mac_random);
         return 0;         // 0 is exist
     }
}

U_BOOT_CMD(
	get_mac_random,	1,	0,	do_get_mac_random,
	"get mac_random environment variable value"
	"/N\n"	
	"get_mac_random\n",	
	"This command will get mac_random environment variable value'\n"
);



/*****************************
 *
 *     set_usid_random
 *
 ****************************/
static void set_usid_random_help(void)
{
     printf(
         "command usage:\n"
         "  set_usid_random 16\n"
         "  ---set 16 random numbers and the value is saved in usid_random environment variable\n");
}

/**
 * command: set_usid_random --set a usid random number to environment variable
 * @length: the length of random number
 * usage: set_usid_random 16
 * if return 0 show sucessful, else failed.
 */
static int do_set_usid_random (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
     size_t i = 0, j = 0, len = 0;
     unsigned char buf[10];
     unsigned char random[USID_RANDOM_LENGTH_MAX];
     unsigned char random_str[USID_RANDOM_LENGTH_MAX*3];
     
     if(argc != 2 || strncmp(argv[0], "set_usid_random", strlen("set_usid_random"))) {
         set_usid_random_help();
         return -1;
     }  

     memset(buf, 0, sizeof(buf));     
     memset(random, 0, sizeof(random));
     memset(random_str, 0, sizeof(random_str));
     
     len = simple_strtoul(argv[1], NULL, 10);
     get_aml_usid_random(random, len);     // 0<= range <=0xf

#if 0
     for(i=0; i<len; i++)
         printf("random[%d]=%c\n", i, random[i]);
#endif
   
     // save environment variable
     for(i=0; i<len; i++) {
         sprintf(buf, "%c", random[i]);
         sprintf(&random_str[j], "%s", buf);
         j += strlen(buf);
         memset(buf, 0, sizeof(buf));
     }

     setenv("usid_random", random_str);
     
#if 1
     printf("set usid_random=%s\n", getenv("usid_random"));
#endif

	return 0;
}

U_BOOT_CMD(
	set_usid_random,	2,	0,	do_set_usid_random,
	"set a usid random number to environment variable",
	"/N\n"
	"set_usid_random <length>\n"
	"This command will set to specify the length of the random number to usid_random environment variable\n"
);



/*****************************
 *
 *     get_usid_random
 *
 ****************************/
static void get_usid_random_help(void)
{
     printf(
         "command usage:\n"
         "  get_usid_random\n"
         "  ---get usid random value\n");
}

/**
 * command: get_usid_random --get usid_random environment variable value
 * usage: get_usid_random
 * if return 0 show there is a usid random number, else reurn 1 show there isn't.
 */
static inline int do_get_usid_random (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
     char *usid_random = NULL;
     
     if(argc != 1 || strncmp(argv[0], "get_usid_random", strlen("get_usid_random"))) {
         get_usid_random_help();
         return -1;
     }

     usid_random = getenv("usid_random");
     if(usid_random == NULL) { 
         printf("no usid random\n");
         return 1;          // 1 is not exist   
     }
     else {
         printf("get usid_random=%s\n", usid_random);
         return 0;         // 0 is exist
     }
}

U_BOOT_CMD(
	get_usid_random,	1,	0,	do_get_usid_random,
	"get usid_random environment variable value"
	"/N\n"	
	"get_usid_random\n",	
	"This command will get usid_random environment variable value'\n"
);
