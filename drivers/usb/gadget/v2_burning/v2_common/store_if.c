/*
 * \file        store_if.c
 * \brief       store interface for v2 burning
 *
 * \version     1.0.0
 * \date        2014/6/5
 * \author      Sam.Wu <yihui.wu@amlgic.com>
 *
 * Copyright (c) 2014 Amlogic Inc.. All Rights Reserved.
 *
 */
#include <common.h>
#include <linux/ctype.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <div64.h>
#include <linux/err.h>
#include <partition_table.h>

int device_boot_flag = NAND_BOOT_FLAG;
static const char *cmd_name = "nand";
extern const char*   _usbDownPartImgType;

#undef store_err
#undef store_msg
#undef store_dbg
#define store_err(fmt...)       printf("[store]L%d", __LINE__), printf(fmt)
#define store_msg(fmt...)       printf("[store]"fmt)
#define store_dbg(fmt...)

/***
upgrade_read_ops:

partition_name: env / logo / recovery /boot / system /cache /media

***/
int store_read_ops(unsigned char *partition_name,unsigned char * buf, uint64_t off, uint64_t size)
{
    unsigned char *name;
    uint64_t addr;
    char	str[128];
    int ret =0;
    const unsigned alignBits = 12;
    const unsigned Alginsz = 1U<<12;
    const unsigned AlignMask = Alginsz - 1;

    if (!buf) {
        store_err("upgrade: no buf!!");
        return -1;
    }

    name = partition_name;
    addr = (unsigned long)buf;

    if (size & AlignMask )size = ((size + AlignMask)>>alignBits) <<alignBits;
#ifdef UBIFS_IMG
    if (!strcmp(_usbDownPartImgType, "ubifs")) {
        sprintf(str, "%s  read 0x%llx %s 0x%llx", "ubi", addr, name, size);
        store_err("command:	%s", str);
    }
    else
#endif
    {// not ubi part
        sprintf(str, "%s  read %s 0x%llx  0x%llx  0x%llx", cmd_name, name, addr, off, size);
    }

    ret = run_command(str, 0);
    if (ret != 0) {
        store_err("nand cmd %s failed ",str);
        return -1;
    }

    return 0;
}

/***
upgrade_write_ops:

partition_name: env / logo / recovery /boot / system /cache /media

***/

int store_write_ops(unsigned char *partition_name,unsigned char * buf, uint64_t off, uint64_t size)
{
    unsigned char *name;
    uint64_t addr;
    char	str[128];
    int ret =0;
    const unsigned alignBits = 12;
    const unsigned Alginsz = 1U<<12;
    const unsigned AlignMask = Alginsz - 1;

    if (!buf) {
        store_err("upgrade: no buf!!");
        return -1;
    }

    name = partition_name;
    addr = (unsigned long)buf;

    if (size & AlignMask )size = ((size + AlignMask)>>alignBits) <<alignBits;
#ifdef UBIFS_IMG
    if (strcmp(_usbDownPartImgType, "ubifs")) {
        sprintf(str, "%s  write %s 0x%llx  0x%llx  0x%llx", cmd_name, name, addr, off, size);
    } else
#endif// #ifdef UBIFS_IMG
    {//ubi part
        sprintf(str, "%s  write 0x%llx %s 0x%llx  ", "ubi", addr, name, size);
    }

    store_dbg("command:	%s", str);
    ret = run_command(str, 0);
    if (ret != 0) {
        store_err("cmd [%s] failed ", str);
        return -1;
    }

    return 0;
}


/***
upgrade_write_ops:

partition_name: env / logo / recovery /boot / system /cache /media

***/

int store_get_partititon_size(unsigned char *partition_name, uint64_t *size)
{
    unsigned char *name;
    char	str[128];
    uint64_t addr;
    int ret=0;
    unsigned char * buf = malloc(4*sizeof(uint64_t));

    if (!buf) {
        store_err("store_get_partititon_size : malloc failed");
        return -1;
    }
    memset(buf,0x0,4*sizeof(uint64_t));
    store_dbg("4*sizeof(uint64_t) =%d",4*sizeof(uint64_t));
    addr = (unsigned long)size;
    name = partition_name;
    sprintf(str, "%s  size %s 0x%llx ",cmd_name, name, addr);
    store_dbg("command:	%s", str);
    ret = run_command(str, 0);
    if (ret != 0) {
        store_err("nand cmd %s size failed ",cmd_name);
        return -1;
    }



    if (buf) {
        kfree(buf);
    }
    return 0;
}


/***
upgrade_erase_ops:

partition_name: boot / data

flag = 0; indicate erase partition ;
flag = 1; indicate scurb whole nand;

***/
int store_erase_ops(unsigned char *par_name, uint64_t off, uint64_t size, unsigned char flag)
{
    const char *name;
    char	str[128];
    int ret=0;

    name = (const char*)par_name;
    if (!strcmp("boot", name)) {
        sprintf(str, "%s  rom_write 0x80400000 0 0x10000",cmd_name);
    }
    else{
        store_err("erase of not supported");
        return __LINE__;
    }
    store_dbg("command:	%s", str);
    ret = run_command(str, 0);
    if (ret != 0) {
        store_err("cmd [%s] failed", str);
        return -1;
    }

    return 0;
}

/***
bootloader:
***/
int store_boot_read(unsigned char * buf, uint64_t off, uint64_t size)
{
    uint64_t addr;
    char	str[128];
    int ret =0;

    if (!buf) {
        store_err("upgrade: no buf!!");
        return -1;
    }

    addr = (unsigned long)buf;
    store_dbg("store_boot_read: addr 0x%llx\n",addr);

    sprintf(str, "%s  rom_read 0x%llx  0x%llx  0x%llx",cmd_name, addr, off, size);
    store_dbg("command:	%s", str);
    ret = run_command(str, 0);
    if (ret != 0) {
        store_err("nand cmd %s  rom_read failed",cmd_name);
        return -1;
    }

    return 0;
}

int store_boot_write(unsigned char * buf,uint64_t off, uint64_t size)
{
    uint64_t addr;
    char	str[128];
    int ret =0;

    if (!buf) {
        store_err("upgrade: no buf!!");
        return -1;
    }

    addr = (unsigned long)buf;

    sprintf(str, "%s  rom_write 0x%llx  0x%llx  0x%llx",cmd_name, addr, off, size);
    store_dbg("command:	%s", str);
    ret = run_command(str, 0);
    if (ret != 0) {
        store_err("nand cmd %s rom_write failed",cmd_name);
        return -1;
    }

    return 0;

}


int store_init(unsigned  flag)
{
    char	str[128];
    int ret =0;
    store_dbg("flag : %d",flag);

    sprintf(str, "%s  init %d",cmd_name,flag);
    store_dbg("command:	%s", str);
    ret = run_command(str, 0);
    if (ret != 0) {
        store_err("cmd [%s] failed ", str);
        return -1;
    }
    switch (flag) {
        case 4:
            store_msg("disprotect secure key\n");
            run_command("nand secure", 0);
        case 3:
            store_msg("disprotect key\n");
            run_command("nand key", 0);
        case 2:
        case 1:
            {
                sprintf(str, "%s  rom_protect off; %s erase 0", cmd_name, cmd_name);
                ret = run_command(str, 0);
                if (ret != 0) {
                    store_err("cmd [%s] failed ",str);
                    return -1;
                }
                saveenv();
            }
            break;
        default:break;
    }

    if (flag > 0) {
        ret = run_command("nand init", 0);//re-init nand if bbt erased
    }

    return ret;
}

int store_exit(void)
{
    char    str[128];
    int ret =0;

    sprintf(str, "%s  exit",cmd_name);
    printf("command:	%s\n", str);
    ret = run_command(str, 0);
    if (ret != 0) {
        store_err("nand cmd %s exit failed",cmd_name);
        return -1;
    }

    return 0;

}

