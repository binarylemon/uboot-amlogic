/*
 * (C) Copyright 2012
 * Amlogic. Inc. zongdong.jiao@amlogic.com
 *
 * This file is used to prefetch/varify/compare HDCP keys
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <malloc.h>
#include <asm/byteorder.h>

#define HDCP_KEY_SIZE                   308
#define HDCP_IP_KEY_SIZE                288
#define TX_HDCP_DKEY_OFFSET             0x400

#if 0
extern ssize_t uboot_key_init(void);
extern int nandkey_provider_register(void);
extern int key_set_version(char *device);
extern ssize_t uboot_key_read(char *keyname, char *keydata);
#endif
ssize_t uboot_key_get(char *device,char *key_name, char *key_data,int key_data_len,int ascii_flag);
extern unsigned long hdmi_hdcp_rd_reg(unsigned long addr);
extern void hdmi_hdcp_wr_reg(unsigned long addr, unsigned long data);

// store prefetch RAW data
static unsigned char hdcp_keys_prefetch[HDCP_KEY_SIZE] = { 0x00 };

static unsigned char hdcp_keys_reformat[HDCP_IP_KEY_SIZE] = { 0x00 };

// dump RAW data from the DEVICE prefetch
static int do_hdcp_dumprawdata(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
    int i;

    printf("dump hdcp raw data\n");
    for(i = 0; i < HDCP_KEY_SIZE; i++) {
        printf("%02x ", hdcp_keys_prefetch[i]);
        if(((i+1) & 0xf) == 0)
            printf("\n");
    }
    printf("\n");

    return 1;
}

// dump RAM data from the IP
static int do_hdcp_dumpipdata(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
    int i, j;

    printf("dump ip ram data\n");
    hdmi_hdcp_wr_reg(0x27, 0);
    for(i = 0, j = 0; i < HDCP_IP_KEY_SIZE - 3; i++) { // ignore 3 zeroes in reserved KSV
        printf("%02x ", (unsigned int)hdmi_hdcp_rd_reg(TX_HDCP_DKEY_OFFSET + j));
        j = ((i % 7) == 6) ? j + 2: j + 1;
        if(((i+1) & 0xf) == 0)
            printf("\n");
    }
    hdmi_hdcp_wr_reg(0x27, 1);
    printf("\n");
    return 1;
}

// compare the RAW data with the IP data
static int do_hdcp_compare(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
    int i, j;
    int idx_raw = 0;
    int idx_ram = 0;
    int val_raw = 0;
    int val_ram = 0;
    int err_cnt = 0;

    printf("comparing the prefetch data with IP data\n");
    hdmi_hdcp_wr_reg(0x27, 0);
    for(i = 0, j = 0; i < HDCP_IP_KEY_SIZE - 3; i++) {
        idx_raw = (i < 280 ? i + 8 : i - 280);
        idx_ram = TX_HDCP_DKEY_OFFSET + j;
        val_ram = hdmi_hdcp_rd_reg(idx_ram);
        val_raw = hdcp_keys_prefetch[idx_raw];
        j = ((i % 7) == 6) ? j + 2: j + 1;
        if(val_ram != val_raw) {
            printf("%03x %02x  --  %03x %02x\n", idx_raw, val_raw, j, val_ram);
            err_cnt ++;
        }
    }
    hdmi_hdcp_wr_reg(0x27, 1);

    printf("Error No: %d\n", err_cnt);
    return 1;
}

/* verify ksv, 20 ones and 20 zeroes*/
static int hdcp_ksv_valid(unsigned char * dat)
{
    int i, j, one_num = 0;
    for(i = 0; i < 5; i++){
        for(j=0;j<8;j++) {
            if((dat[i]>>j)&0x1) {
                one_num++;
            }
        }
    }
    return (one_num == 20);
}

static int do_hdcp_check(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
    int ret = hdcp_ksv_valid(hdcp_keys_prefetch);
    if(ret == 1)
        printf("AKSV valid\n");
    else
        printf("AKSV invalid\n");
    return ret;
}

// copy the fetched data into HDMI IP
static int init_hdcp_ram(unsigned char * dat, unsigned int pre_clear)
{
    int i, j;
    char value;
    void hdmi_hdcp_wr_reg(unsigned long addr, unsigned long data);
    unsigned int ram_addr;

    memset(hdcp_keys_reformat, 0, sizeof(hdcp_keys_reformat));
    // adjust the HDCP key's KSV & DPK position
    memcpy(&hdcp_keys_reformat[0], &hdcp_keys_prefetch[8], HDCP_IP_KEY_SIZE-8);   // DPK
    memcpy(&hdcp_keys_reformat[280], &hdcp_keys_prefetch[0], 5);   // KSV
    j = 0;
    for (i = 0; i < HDCP_IP_KEY_SIZE - 3; i++) {  // ignore 3 zeroes in reserved KSV
        value = hdcp_keys_reformat[i];
        ram_addr = TX_HDCP_DKEY_OFFSET+j;
        hdmi_hdcp_wr_reg(ram_addr, value);
        j = ((i % 7) == 6) ? j + 2: j + 1;
    }

    if(hdcp_ksv_valid(dat)) {
        printf("hdcp init done\n");
    }
    else {
        if(pre_clear == 0)
            printf("AKSV invalid, hdcp init failed\n");
        else
            printf("pre-clear hdmi ram\n");
    }

    return 1;
}

// Prefetch the HDCP keys data from nand, efuse, etc
static int do_hdcp_prefetch(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int ret=0, prefetch_flag = 0;

	if (argc == 1)
		return cmd_usage(cmdtp);

    init_hdcp_ram(hdcp_keys_prefetch, 1);

    ret = uboot_key_get(argv[1], "hdcp", hdcp_keys_prefetch, 308, 0);
    if(ret >= 0) {
        prefetch_flag = 1;;
    }
    else {
        printf("prefetch hdcp keys from %s failed\n", argv[1]);
    }
#if 0
        printf("FAKE prefect HDCP key\n");
        prefetch_flag = 1;
#endif
#if 0
        ret = uboot_key_get();
#endif

    if(prefetch_flag == 1) {
        init_hdcp_ram(hdcp_keys_prefetch, 0);
        memset(hdcp_keys_reformat, 0, sizeof(hdcp_keys_reformat));  // clear the buffer to prevent reveal keys
        ret = 1;
    }
	return 1;
}

static cmd_tbl_t cmd_hdcp_sub[] = {
	U_BOOT_CMD_MKENT(prefetch, 2, 1, do_hdcp_prefetch, "", ""),
	U_BOOT_CMD_MKENT(dumprawdata, 0, 1, do_hdcp_dumprawdata, "", ""),
	U_BOOT_CMD_MKENT(dumpipdata, 0, 1, do_hdcp_dumpipdata, "", ""),
	U_BOOT_CMD_MKENT(check, 0, 1, do_hdcp_check, "", ""),
	U_BOOT_CMD_MKENT(compare, 0, 1, do_hdcp_compare, "", ""),
};

static int do_hdcp(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *c;

	if (argc < 2)
		return cmd_usage(cmdtp);

	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_hdcp_sub[0], ARRAY_SIZE(cmd_hdcp_sub));

	if (c)
		return  c->cmd(cmdtp, flag, argc, argv);
	else
		return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	hdcp, 3, 0, do_hdcp,
	"HDCP sub-system",
    "prefetch [device] - prefetch hdcp keys from nand, efuse or others\n"
    "hdcp dumprawdata - dump the prefetch data\n"
    "hdcp dumpipdata - dump the ip ram data\n"
    "hdcp check - check KSV valid\n"
    "hdcp compare - compare RAW data with IP data"
);

