/*
 * drivers/mtd/nand/nand_util.c
 *
 * Copyright (C) 2006 by Weiss-Electronic GmbH.
 * All rights reserved.
 *
 * @author:	Guido Classen <clagix@gmail.com>
 * @descr:	NAND Flash support
 * @references: borrowed heavily from Linux mtd-utils code:
 *		flash_eraseall.c by Arcom Control System Ltd
 *		nandwrite.c by Steven J. Hill (sjhill@realitydiluted.com)
 *			       and Thomas Gleixner (tglx@linutronix.de)
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Copyright 2010 Freescale Semiconductor
 * The portions of this file whose copyright is held by Freescale and which
 * are not considered a derived work of GPL v2-only code may be distributed
 * and/or modified under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 */

#include <common.h>
#include <command.h>
#include <watchdog.h>
#include <malloc.h>
#include <div64.h>

#include <asm/errno.h>
#include <linux/err.h>
#include <linux/mtd/mtd.h>
#include <nand.h>
#include <jffs2/jffs2.h>
#ifdef NAND_DEBUG
#define aml_nand_debug(a...) printk(a)
#else
#define aml_nand_debug(a...) 
#endif
typedef struct erase_info erase_info_t;
typedef struct mtd_info	  mtd_info_t;

/* support only for native endian JFFS2 */
#define cpu_to_je16(x) (x)
#define cpu_to_je32(x) (x)

/*****************************************************************************/
static int nand_block_bad_scrub(struct mtd_info *mtd, loff_t ofs, int getchip)
{
	return 0;
}

/**
 * nand_erase_opts: - erase NAND flash with support for various options
 *		      (jffs2 formating)
 *
 * @param meminfo	NAND device to erase
 * @param opts		options,  @see struct nand_erase_options
 * @return		0 in case of success
 *
 * This code is ported from flash_eraseall.c from Linux mtd utils by
 * Arcom Control System Ltd.
 */
int nand_erase_opts(nand_info_t *meminfo, const nand_erase_options_t *opts)
{
	struct jffs2_unknown_node cleanmarker;
	erase_info_t erase;
	unsigned long erase_length, erased_length; /* in blocks */
	int bbtest = 1;
	int result;
	int percent_complete = -1;
	int (*nand_block_bad_old)(struct mtd_info *, loff_t, int) = NULL;
	const char *mtd_device = meminfo->name;
	struct mtd_oob_ops oob_opts;
	struct nand_chip *chip = meminfo->priv;

	if ((opts->offset & (meminfo->writesize - 1)) != 0) {
		printf("Attempt to erase non page aligned data\n");
		return -1;
	}

	memset(&erase, 0, sizeof(erase));
	memset(&oob_opts, 0, sizeof(oob_opts));

	erase.mtd = meminfo;
	erase.len  = meminfo->erasesize;
	erase.addr = opts->offset;
	erase_length = lldiv(opts->length + meminfo->erasesize - 1,
			     meminfo->erasesize);

	cleanmarker.magic = cpu_to_je16 (JFFS2_MAGIC_BITMASK);
	cleanmarker.nodetype = cpu_to_je16 (JFFS2_NODETYPE_CLEANMARKER);
	cleanmarker.totlen = cpu_to_je32(8);

	/* scrub option allows to erase badblock. To prevent internal
	 * check from erase() method, set block check method to dummy
	 * and disable bad block table while erasing.
	 */
	if (opts->scrub) {
		struct nand_chip *priv_nand = meminfo->priv;

		nand_block_bad_old = priv_nand->block_bad;
		priv_nand->block_bad = nand_block_bad_scrub;
		/* we don't need the bad block table anymore...
		 * after scrub, there are no bad blocks left!
		 */
		if (priv_nand->bbt) {
			kfree(priv_nand->bbt);
		}
		priv_nand->bbt = NULL;
	}

	for (erased_length = 0;
	     erased_length < erase_length;
	     erase.addr += meminfo->erasesize) {

		WATCHDOG_RESET ();

		if (!opts->scrub && bbtest) {
			int ret = meminfo->block_isbad(meminfo, erase.addr);
			if (ret > 0) {
				if (!opts->quiet)
					printf("\rSkipping bad block at  "
					       "0x%08llx                 "
					       "                         \n",
					       erase.addr);

				if (!opts->spread)
					erased_length++;

				continue;

			} else if (ret < 0) {
				printf("\n%s: MTD get bad block failed: %d\n",
				       mtd_device,
				       ret);
				return -1;
			}
		}

		erased_length++;

		result = meminfo->erase(meminfo, &erase);
		if (result != 0) {
			printf("\n%s: MTD Erase failure: %d\n",
			       mtd_device, result);
			continue;
		}

		/* format for JFFS2 ? */
		if (opts->jffs2 && chip->ecc.layout->oobavail >= 8) {
			chip->ops.ooblen = 8;
			chip->ops.datbuf = NULL;
			chip->ops.oobbuf = (uint8_t *)&cleanmarker;
			chip->ops.ooboffs = 0;
			chip->ops.mode = MTD_OOB_AUTO;

			result = meminfo->write_oob(meminfo,
			                            erase.addr,
			                            &chip->ops);
			if (result != 0) {
				printf("\n%s: MTD writeoob failure: %d\n",
				       mtd_device, result);
				continue;
			}
		}

		if (!opts->quiet) {
			unsigned long long n = erased_length * 100ULL;
			int percent;

			do_div(n, erase_length);
			percent = (int)n;

			/* output progress message only at whole percent
			 * steps to reduce the number of messages printed
			 * on (slow) serial consoles
			 */
			if (percent != percent_complete) {
				percent_complete = percent;

				printf("\rErasing at 0x%llx -- %3d%% complete.",
				       erase.addr, percent);

				if (opts->jffs2 && result == 0)
					printf(" Cleanmarker written at 0x%llx.",
					       erase.addr);
			}
		}
	}
	if (!opts->quiet)
		printf("\n");

	if (nand_block_bad_old) {
		struct nand_chip *priv_nand = meminfo->priv;

		priv_nand->block_bad = nand_block_bad_old;
		priv_nand->scan_bbt(meminfo);
	}

	return 0;
}

#ifdef CONFIG_CMD_NAND_LOCK_UNLOCK

/******************************************************************************
 * Support for locking / unlocking operations of some NAND devices
 *****************************************************************************/

#define NAND_CMD_LOCK		0x2a
#define NAND_CMD_LOCK_TIGHT	0x2c
#define NAND_CMD_UNLOCK1	0x23
#define NAND_CMD_UNLOCK2	0x24
#define NAND_CMD_LOCK_STATUS	0x7a

/**
 * nand_lock: Set all pages of NAND flash chip to the LOCK or LOCK-TIGHT
 *	      state
 *
 * @param mtd		nand mtd instance
 * @param tight		bring device in lock tight mode
 *
 * @return		0 on success, -1 in case of error
 *
 * The lock / lock-tight command only applies to the whole chip. To get some
 * parts of the chip lock and others unlocked use the following sequence:
 *
 * - Lock all pages of the chip using nand_lock(mtd, 0) (or the lockpre pin)
 * - Call nand_unlock() once for each consecutive area to be unlocked
 * - If desired: Bring the chip to the lock-tight state using nand_lock(mtd, 1)
 *
 *   If the device is in lock-tight state software can't change the
 *   current active lock/unlock state of all pages. nand_lock() / nand_unlock()
 *   calls will fail. It is only posible to leave lock-tight state by
 *   an hardware signal (low pulse on _WP pin) or by power down.
 */
int nand_lock(struct mtd_info *mtd, int tight)
{
	int ret = 0;
	int status;
	struct nand_chip *chip = mtd->priv;

	/* select the NAND device */
	chip->select_chip(mtd, 0);

	chip->cmdfunc(mtd,
		      (tight ? NAND_CMD_LOCK_TIGHT : NAND_CMD_LOCK),
		      -1, -1);

	/* call wait ready function */
	status = chip->waitfunc(mtd, chip);

	/* see if device thinks it succeeded */
	if (status & 0x01) {
		ret = -1;
	}

	/* de-select the NAND device */
	chip->select_chip(mtd, -1);
	return ret;
}

/**
 * nand_get_lock_status: - query current lock state from one page of NAND
 *			   flash
 *
 * @param mtd		nand mtd instance
 * @param offset	page address to query (muss be page aligned!)
 *
 * @return		-1 in case of error
 *			>0 lock status:
 *			  bitfield with the following combinations:
 *			  NAND_LOCK_STATUS_TIGHT: page in tight state
 *			  NAND_LOCK_STATUS_LOCK:  page locked
 *			  NAND_LOCK_STATUS_UNLOCK: page unlocked
 *
 */
int nand_get_lock_status(struct mtd_info *mtd, loff_t offset)
{
	int ret = 0;
	int chipnr;
	int page;
	struct nand_chip *chip = mtd->priv;

	/* select the NAND device */
	chipnr = (int)(offset >> chip->chip_shift);
	chip->select_chip(mtd, chipnr);


	if ((offset & (mtd->writesize - 1)) != 0) {
		printf ("nand_get_lock_status: "
			"Start address must be beginning of "
			"nand page!\n");
		ret = -1;
		goto out;
	}

	/* check the Lock Status */
	page = (int)(offset >> chip->page_shift);
	chip->cmdfunc(mtd, NAND_CMD_LOCK_STATUS, -1, page & chip->pagemask);

	ret = chip->read_byte(mtd) & (NAND_LOCK_STATUS_TIGHT
					  | NAND_LOCK_STATUS_LOCK
					  | NAND_LOCK_STATUS_UNLOCK);

 out:
	/* de-select the NAND device */
	chip->select_chip(mtd, -1);
	return ret;
}

/**
 * nand_unlock: - Unlock area of NAND pages
 *		  only one consecutive area can be unlocked at one time!
 *
 * @param mtd		nand mtd instance
 * @param start		start byte address
 * @param length	number of bytes to unlock (must be a multiple of
 *			page size nand->writesize)
 *
 * @return		0 on success, -1 in case of error
 */
int nand_unlock(struct mtd_info *mtd, ulong start, ulong length)
{
	int ret = 0;
	int chipnr;
	int status;
	int page;
	struct nand_chip *chip = mtd->priv;
	printf ("nand_unlock: start: %08x, length: %d!\n",
		(int)start, (int)length);

	/* select the NAND device */
	chipnr = (int)(start >> chip->chip_shift);
	chip->select_chip(mtd, chipnr);

	/* check the WP bit */
	chip->cmdfunc(mtd, NAND_CMD_STATUS, -1, -1);
	if (!(chip->read_byte(mtd) & NAND_STATUS_WP)) {
		printf ("nand_unlock: Device is write protected!\n");
		ret = -1;
		goto out;
	}

	if ((start & (mtd->erasesize - 1)) != 0) {
		printf ("nand_unlock: Start address must be beginning of "
			"nand block!\n");
		ret = -1;
		goto out;
	}

	if (length == 0 || (length & (mtd->erasesize - 1)) != 0) {
		printf ("nand_unlock: Length must be a multiple of nand block "
			"size %08x!\n", mtd->erasesize);
		ret = -1;
		goto out;
	}

	/*
	 * Set length so that the last address is set to the
	 * starting address of the last block
	 */
	length -= mtd->erasesize;

	/* submit address of first page to unlock */
	page = (int)(start >> chip->page_shift);
	chip->cmdfunc(mtd, NAND_CMD_UNLOCK1, -1, page & chip->pagemask);

	/* submit ADDRESS of LAST page to unlock */
	page += (int)(length >> chip->page_shift);
	chip->cmdfunc(mtd, NAND_CMD_UNLOCK2, -1, page & chip->pagemask);

	/* call wait ready function */
	status = chip->waitfunc(mtd, chip);
	/* see if device thinks it succeeded */
	if (status & 0x01) {
		/* there was an error */
		ret = -1;
		goto out;
	}

 out:
	/* de-select the NAND device */
	chip->select_chip(mtd, -1);
	return ret;
}
#endif

/**
 * check_skip_len
 *
 * Check if there are any bad blocks, and whether length including bad
 * blocks fits into device
 *
 * @param nand NAND device
 * @param offset offset in flash
 * @param length image length
 * @return 0 if the image fits and there are no bad blocks
 *         1 if the image fits, but there are bad blocks
 *        -1 if the image does not fit
 */
static int check_skip_len(nand_info_t *nand, loff_t offset, size_t length)
{
	size_t len_excl_bad = 0;
	int ret = 0;

	while (len_excl_bad < length) {
		size_t block_len, block_off;
		loff_t block_start;

		if (offset >= nand->size)
			return -1;

		block_start = offset & ~(loff_t)(nand->erasesize - 1);
		block_off = offset & (nand->erasesize - 1);
		block_len = nand->erasesize - block_off;

		if (!nand_block_isbad(nand, block_start))
			len_excl_bad += block_len;
		else
			ret = 1;

		offset += block_len;
	}

	return ret;
}

/**
 * nand_write_skip_bad:
 *
 * Write image to NAND flash.
 * Blocks that are marked bad are skipped and the is written to the next
 * block instead as long as the image is short enough to fit even after
 * skipping the bad blocks.
 *
 * @param nand  	NAND device
 * @param offset	offset in flash
 * @param length	buffer length
 * @param buffer        buffer to read from
 * @param withoob	whether write with yaffs format
 * @return		0 in case of success
 */
int nand_write_skip_bad(nand_info_t *nand, loff_t offset, size_t *length,
			u_char *buffer, int withoob)
{
	int rval = 0, blocksize;
	size_t left_to_write = *length;
	u_char *p_buffer = buffer;
	int need_skip;

#ifdef CONFIG_CMD_NAND_YAFFS
	if (withoob) {
		int pages;
		pages = nand->erasesize / nand->writesize;
		blocksize = (pages * nand->oobsize) + nand->erasesize;
		if (*length % (nand->writesize + nand->oobsize)) {
			printf ("Attempt to write incomplete page"
				" in yaffs mode\n");
			return -EINVAL;
		}
	} else
#endif
	{
		blocksize = nand->erasesize;
	}

	/*
	 * nand_write() handles unaligned, partial page writes.
	 *
	 * We allow length to be unaligned, for convenience in
	 * using the $filesize variable.
	 *
	 * However, starting at an unaligned offset makes the
	 * semantics of bad block skipping ambiguous (really,
	 * you should only start a block skipping access at a
	 * partition boundary).  So don't try to handle that.
	 */
	if ((offset & (nand->writesize - 1)) != 0) {
		printf ("Attempt to write non page aligned data\n");
		*length = 0;
		return -EINVAL;
	}

	need_skip = check_skip_len(nand, offset, *length);
	if (need_skip < 0) {
		printf ("Attempt to write outside the flash area\n");
		*length = 0;
		return -EINVAL;
	}

	if (!need_skip) {
		rval = nand_write (nand, offset, length, buffer);
		if (rval == 0)
			return 0;

		*length = 0;
		printf ("NAND write to offset %llx failed %d\n",
			offset, rval);
		return rval;
	}

	while (left_to_write > 0) {
		size_t block_offset = offset & (nand->erasesize - 1);
		size_t write_size;

		WATCHDOG_RESET ();

		if (nand_block_isbad (nand, offset & ~(nand->erasesize - 1))) {
			printf ("Skip bad block 0x%08llx\n",
				offset & ~(nand->erasesize - 1));
			offset += nand->erasesize - block_offset;
			continue;
		}

		if (left_to_write < (blocksize - block_offset))
			write_size = left_to_write;
		else
			write_size = blocksize - block_offset;

#ifdef CONFIG_CMD_NAND_YAFFS
		if (withoob) {
			int page, pages;
			size_t pagesize = nand->writesize;
			size_t pagesize_oob = pagesize + nand->oobsize;
			struct mtd_oob_ops ops;

			ops.len = pagesize;
			ops.ooblen = nand->oobsize;
			ops.mode = MTD_OOB_AUTO;
			ops.ooboffs = 0;

			pages = write_size / pagesize_oob;
			for (page = 0; page < pages; page++) {
				ops.datbuf = p_buffer;
				ops.oobbuf = ops.datbuf + pagesize;

				rval = nand->write_oob(nand, offset, &ops);
				if (!rval)
					break;

				offset += pagesize;
				p_buffer += pagesize_oob;
			}
		}
		else
#endif
		{
			rval = nand_write (nand, offset, &write_size, p_buffer);
			offset += write_size;
			p_buffer += write_size;
		}

		if (rval != 0) {
			printf ("NAND write to offset %llx failed %d\n",
				offset, rval);
			*length -= left_to_write;
			return rval;
		}

		left_to_write -= write_size;
	}

	return 0;
}

/**
 * nand_read_skip_bad:
 *
 * Read image from NAND flash.
 * Blocks that are marked bad are skipped and the next block is readen
 * instead as long as the image is short enough to fit even after skipping the
 * bad blocks.
 *
 * @param nand NAND device
 * @param offset offset in flash
 * @param length buffer length, on return holds remaining bytes to read
 * @param buffer buffer to write to
 * @return 0 in case of success
 */
int nand_read_skip_bad(nand_info_t *nand, loff_t offset, size_t *length,
		       u_char *buffer)
{
	int rval;
	size_t left_to_read = *length;
	u_char *p_buffer = buffer;
	int need_skip;

	if ((offset & (nand->writesize - 1)) != 0) {
		printf ("Attempt to read non page aligned data\n");
		*length = 0;
		return -EINVAL;
	}

	need_skip = check_skip_len(nand, offset, *length);
	if (need_skip < 0) {
		printf ("Attempt to read outside the flash area\n");
		*length = 0;
		return -EINVAL;
	}

	if (!need_skip) {
		rval = nand_read (nand, offset, length, buffer);
		if (!rval || rval == -EUCLEAN)
			return 0;

		*length = 0;
		printf ("NAND read from offset %llx failed %d\n",
			offset, rval);
		return rval;
	}

	while (left_to_read > 0) {
		size_t block_offset = offset & (nand->erasesize - 1);
		size_t read_length;

		WATCHDOG_RESET ();

		if (nand_block_isbad (nand, offset & ~(nand->erasesize - 1))) {
			printf ("Skipping bad block 0x%08llx\n",
				offset & ~(nand->erasesize - 1));
			offset += nand->erasesize - block_offset;
			continue;
		}

		if (left_to_read < (nand->erasesize - block_offset))
			read_length = left_to_read;
		else
			read_length = nand->erasesize - block_offset;

		rval = nand_read (nand, offset, &read_length, p_buffer);
		if (rval && rval != -EUCLEAN) {
			printf ("NAND read from offset %llx failed %d\n",
				offset, rval);
			*length -= left_to_read;
			return rval;
		}

		left_to_read -= read_length;
		offset       += read_length;
		p_buffer     += read_length;
	}

	return 0;
}

#include <asm/arch/nand.h>
#if CONFIG_NAND_AML_M3 || CONFIG_NAND_AML_A3
#include <asm/arch/romboot.h>
#define SKIP_HOLE 1
//const char default_boot_name[]="nandboot";
int romboot_nand_write(nand_info_t *nand, loff_t offset, size_t * plen,
			u_char *buf, int protect_flag)
{
	unsigned int area_size,i,cur,area_loop;
	unsigned ret=0,err_flag=0;
	unsigned int w_size,total_size;
	unsigned w_offset = 0;
	size_t len=*plen;
	u_char * in_buf=(u_char*)buf;
	unsigned char * rom_inter_buff=NULL;

	nand_erase_options_t opts;
	struct mtd_oob_ops ops=	{.retlen=0 ,
							.mode=MTD_OOB_PLACE};
	u16    oob[]={0xaa55,0xaa55,0xaa55,0xaa55};	
	struct mtd_info * mtd=get_mtd_device_nm(NAND_BOOT_NAME);
	struct aml_nand_chip *aml_chip = mtd_to_nand_chip(mtd);
	
	if (IS_ERR(mtd)) {
		printk("NO nand romboot device , pls config it\n");
		return  1;	
	}	

#if 0
		if(offset!=0) 
		{
			printk("Wrong addr begin\n");
			return	1;
		}
#endif


	if(mtd->writesize<=512)
	{
		area_size=mtd->erasesize;
		w_size=512;
		ops.ooblen=2;
		total_size=512*1024;
	}
	else
	{
		area_size=mtd->writesize*256;
		w_size=3*512;
		total_size=mtd->writesize*1024;
		ops.ooblen=8;
		if(len>w_size*255)
		{
			printk("Wrong Length bigger than 255*3*512 ,sub it \n");
			len=w_size*255;
		}
//		if(len%w_size)
//		{
//			printk("Wrong Length NOt 3*512 times \n");
//			return  1;	
//		}
	}

	if(protect_flag)
	{
		printk("ERASE  BOOT DEVICE\n");
		printk("INPUT y for sure this \n");
		if (getc() != 'y') {
			printk("ABORT \n");
			return 1 ;
		}
	}

	opts.offset = 0;
	opts.length =total_size;
	opts.jffs2=0;
	opts.scrub=0;
	opts.quiet= 0;
	ret = nand_erase_opts(mtd, &opts);
	printf(" erase %s\n", ret ? "ERROR" : "OK");
	if(ret)
		return 1;	
	
	ops.retlen=0;
	ops.oobbuf=(uint8_t*)&oob[0];
//	ops.len=w_size;
	ops.len=mtd->writesize;

	unsigned short_mode=0,pg_num=0;
	unsigned pages_in_block = mtd->erasesize/mtd->writesize;
	struct nand_chip * chip=(struct nand_chip *)(mtd->priv);
	struct aml_nand_platform * plat_info = aml_chip->platform;


	if(plat_info->short_pgsz!=0)
	{
		short_mode=1;
		pg_num=w_size/plat_info->short_pgsz;
			
	}else
	{
		BUG();			// 1024 mode 1536
		pg_num=w_size/((aml_chip->bch_mode&NAND_ECC_BCH8_512)?512:1024);
	}

	rom_inter_buff=(unsigned char *)malloc(mtd->writesize);
	if(rom_inter_buff==NULL)
		BUG();

	unsigned int ext=N2M_NORAN|((aml_chip->bch_mode&NAND_ECC_OPTIONS_MASK)<<14)|(short_mode<<13)|((plat_info->short_pgsz>>3)<<6)|pg_num;	
	//bit 23-25
	memset(rom_inter_buff,0xff,mtd->writesize);
	memset(rom_inter_buff,0xbb,w_size);		//quer bad share 
	memcpy((void*)rom_inter_buff, (void*)&ext,sizeof(unsigned));
	memcpy((void*)((unsigned)rom_inter_buff+sizeof(unsigned)), (void*)&pages_in_block,sizeof(unsigned)); //add block size 
    ops.datbuf=rom_inter_buff;
    printk("ext=%x\n",ext);
    w_offset=0;
	for(area_loop=0;area_loop<4;area_loop++)
	{
		if(err_flag&(1<<area_loop))
		    continue;
		ret=mtd->write_oob(mtd,area_loop*area_size+w_offset,&ops);						//that is a recursion , need 384 , but write 1536 , use bch 60 no ran 	
		if(ret)
		{
			err_flag|=1<<area_loop;
			printk("mtd->writeoob err at area %d first pg %lld \n",area_loop,area_loop*area_size);	
		}
    }
    if(err_flag==0xf)
    {
        printk("%s!%d:All of four backup areas are bad",__func__,__LINE__);	
        return -1;
    }
	for(cur=0,w_offset=mtd->writesize;cur<(len)&&w_offset<area_size;cur+=w_size,w_offset += mtd->writesize)
	{
		ops.datbuf=(uint8_t*)&in_buf[cur];
        aml_nand_debug("w_offset=%08x\n",w_offset);
		for(area_loop=0;area_loop<4;area_loop++)
    	{
    		if(err_flag&(1<<area_loop))
    		    continue;
    		ret=mtd->write_oob(mtd,area_loop*area_size+w_offset,&ops);						//that is a recursion , need 384 , but write 1536 , use bch 60 no ran 	
    		if(ret)
    		{
    			err_flag|=1<<area_loop;
    			aml_nand_debug("mtd->writeoob err at area %d first offset %lld \n",area_loop,area_loop*area_size+w_offset);	
    		}
        }	
        if(err_flag==0xf)
        {
            printk("%s!%d:All of four backup areas are bad",__func__,__LINE__);	
            return -1;
        }	
		
	}	
	free(rom_inter_buff);
	return 0;
}
int romboot_nand_read(nand_info_t *nand, loff_t offset, size_t * plen,
			u_char *buf)
{
	unsigned int i,cur;
	unsigned ret=0,err_flag=0;
	unsigned int w_size;
	unsigned r_offset = 0;
	size_t len=*plen;
	unsigned  * rom_inter_buff=NULL;

	nand_erase_options_t opts;
	struct mtd_oob_ops ops=	{.retlen=0 ,
							.mode=MTD_OOB_PLACE};
    unsigned short oob[8];							
	struct mtd_info * mtd=get_mtd_device_nm(NAND_BOOT_NAME);
	if (IS_ERR(mtd)) {
		printk("NO nand romboot device , pls config it\n");
		return  1;	
	}	

#if 0
	if(offset!=0) 
	{
		printk("Wrong addr begin\n");
		return  1;
	}
#endif
    printk("The writesize=%x,erasesize=%x\n",mtd->writesize,mtd->erasesize);
	rom_inter_buff = (unsigned char *)malloc(mtd->writesize);
	if(rom_inter_buff==NULL)
		BUG();

	if(mtd->writesize<=512)
	{
		w_size=512;
	}
	else
	{
		w_size=3*512;
	}	
	
	ops.retlen=0;
	ops.oobbuf= &oob[0];
	ops.ooblen=8;
	ops.len=mtd->writesize;
	ops.datbuf=rom_inter_buff;
	
	int area;
	unsigned  mem[4];
	unsigned  info[4];
	err_flag=0;
	for(area=0;area<4;area++)
	{
	    mem[area]=((unsigned)buf)+0x100000*area;
	    info[area]=((unsigned)buf)+0x100000*area+0x100000-0x100000/256;
	    ret=mtd->read_oob(mtd,area*(mtd->writesize*256),&ops);
	    
	    printk("area %x read to %08x info is in %08x , ecc mode %08x ,oob=%04x,",area,mem[area],info[area],*rom_inter_buff,oob[0]);
	    if(ret&&ret!=-EUCLEAN)
		{
			err_flag|=(1<<area);
			aml_nand_debug("mtd->read err at  %x %d",area*(mtd->writesize*256),ret);	
		}
		printk("\n");
	        
	}
	unsigned  databuf,oobbuf;
	for(area=0;area<4;area++)
	{
	    for(r_offset=(area*256*mtd->writesize+mtd->writesize),databuf=mem[area],oobbuf=info[area];
	        r_offset<((area+1)*256*mtd->writesize);
	        databuf+=w_size,oobbuf+=w_size/(384/2),r_offset+=mtd->writesize
	        )
	    {
	        aml_nand_debug("read %x to %x",r_offset,databuf);	
	        ret=mtd->read_oob(mtd, r_offset,&ops);
	        if(ret&&ret!=-EUCLEAN)
    		{
    			err_flag|=(1<<area);
    			aml_nand_debug("mtd->read err at  %x %d mem_addr=%x",r_offset,ret,databuf);	
    		}
    		aml_nand_debug("\n");
	        memcpy(databuf,rom_inter_buff,w_size);
	        memcpy(oobbuf,&oob[0],w_size/(384/2));
	    }
	}
	

	if((err_flag&0xf)==0xf)
	{
		printk("romboot_nand_read  err happen , for safe you can use cmd : nand device 1 ; nand erase ; nand device 0  \n");
		free(rom_inter_buff);
		return 1;
	}	
	free(rom_inter_buff);
	return 0;
}

#else
int romboot_nand_write(nand_info_t *nand, loff_t offset, size_t * plen,
			u_char *buf, int protect_flag)
{
	unsigned int area_size,i,cur;
	unsigned ret=0,err_flag=0;
	unsigned int w_size,total_size;
	size_t len=*plen;
	u_char * p=(u_char*)buf;
	unsigned char *tmp_buf;
	nand_erase_options_t opts;

	struct mtd_oob_ops ops=	{.retlen=0 ,
							.mode=MTD_OOB_PLACE};
	u16    oob[]={0x55aa,0x55aa,0x55aa};
	struct mtd_info * mtd = nand;//get_mtd_device_nm(default_boot_name);

	tmp_buf = kzalloc((mtd->writesize), GFP_KERNEL);
	if (!tmp_buf)
		return 1;

	if (IS_ERR(mtd)) {
		printk("NO nand romboot device , pls config it\n");
		printk("ABORT \n");
		return  1;	
	}	

	if(offset!=0) 
	{
		printk("Wrong addr begin\n");
		printk("ABORT \n");
		return  1;
	}

	if(mtd!=nand)
	{	
		printk("AUTO SWITCH TO BOOT DEVICE\n");
	}
	
	if(mtd->writesize<=512)
	{
		area_size=mtd->erasesize;
		w_size=512;
		ops.ooblen=2;
		total_size=512*1024;
	}else
	{
		area_size=mtd->writesize*256;
		w_size=3*512;
		total_size=mtd->writesize*1024;
		ops.ooblen=6;
		if(len>w_size*256)
		{
			printk("Wrong Length bigger\n");
			printk("ABORT \n");
			return  1;
		}
		if(len%w_size)
		{
		
			printk("Wrong Length NOt 3*512 \n");
			printk("ABORT \n");
			return  1;	
		}
	}

	if(protect_flag)
	{
		printk("^^^^^^^MUST ERASE THIS BOOT DEVICE FIRST^^^^^^^^^\n");
		printk("INPUT y for sure this \n");
		if (getc() != 'y') {
			printk("ABORT \n");
			return 1 ;
		}
	}

	opts.offset = 0;
	opts.length =total_size;
	opts.jffs2=0;
	opts.quiet  = 0;
	opts.scrub  = 0;
	ret = nand_erase_opts(nand, &opts);
	printf(" erase %s\n", ret ? "ERROR" : "OK");
	if(ret)
		return 1;	
	
	ops.retlen=0;
	ops.oobbuf=(uint8_t*)&oob[0];
	ops.len=mtd->writesize;

	
	for(cur=0,i=0;cur<len;i++)
	{
			memset(tmp_buf, 0xff, mtd->writesize);
			memcpy(tmp_buf, (uint8_t*)&p[cur], 3*512);
			ops.datbuf=tmp_buf;//(uint8_t*)&p[cur];
			ret=mtd->write_oob(mtd,offset+i*(mtd->writesize),&ops);
			if(ret)
			{
				err_flag=1;
				printk("mtd->writeoob err at 0Aoff %lld \n",offset+i*(mtd->writesize));	
			}
			ret=mtd->write_oob(mtd,offset+area_size+i*(mtd->writesize),&ops);
			if(ret)
			{
				err_flag=1;
				printk("mtd->writeoob err at 1Aoff %lld \n",offset+area_size+i*(mtd->writesize));	
			}
			ret=mtd->write_oob(mtd,offset+2*area_size+i*(mtd->writesize),&ops);
			if(ret)
			{
				err_flag=1;
				printk("mtd->writeoob err at 2Aoff %lld \n",offset+2*area_size+i*(mtd->writesize));	
			}
			ret=mtd->write_oob(mtd,offset+3*area_size+i*(mtd->writesize),&ops);
			if(ret)
			{
				err_flag=1;
				printk("mtd->writeoob err at 3Aoff %lld \n",offset+3*area_size+i*(mtd->writesize));	
			}	

			cur	+=3*512;
	}

	if(err_flag)
	{
		printk("romboot_nand_write  err happen , for safe you can use cmd : nand device 1 ; nand erase ; nand device 0  \n");
		return 1;
	}	

	return 0;
}

int romboot_nand_read(nand_info_t *nand, loff_t offset, size_t * plen,
			u_char *buf)
{
	int area_size,i,cur,temp,j,k,ret=0;
	int w_size,total_size;
	size_t len=*plen;
	u_char * p=(u_char*)buf;
	struct mtd_info * mtd=get_mtd_device_nm(NAND_BOOT_NAME);
	struct mtd_oob_ops ops={.retlen=0 ,
							.mode=MTD_OOB_PLACE};
	u16    oob[]={0,0,0};
	ops.retlen=0;
	ops.oobbuf=(uint8_t*)&oob[0];
	if(mtd->writesize<=512)
	{
		area_size=mtd->erasesize;
		w_size=512;
		ops.ooblen=2;
		total_size=512*1024;
	}else{
		area_size=mtd->writesize*256;
		w_size=3*512;
		total_size=mtd->writesize*1024;
		ops.ooblen=6;
		if(len>w_size*256)
		{
			printk("Wrong Length\n");
			BUG();
		}
	}
	ops.len=w_size;
	for(i=0,cur=0;i<total_size&&cur<len;i+=area_size)
	{
		temp=cur;
		for(j=0;j<area_size;j+=mtd->erasesize)
		{
			for(k=0;k<mtd->erasesize;k+=mtd->writesize)
			{
				ops.datbuf=&p[temp];
				ret=mtd->read_oob(mtd,i+j+k,&ops);
				if(ret)
					break;
				temp+=w_size;
				if(oob[0]!=0x55aa){
					ret=1;
					break;
				}
				if(ops.ooblen==6&&oob[1]!=0x55aa&&oob[2]!=0x55aa)
				{
					ret=1;
					break;
				}
			}
			if(ret)
				break;



		}
		if(ret)
			continue;
		cur=temp;
	}
	if(i>total_size&&cur<len)
	{
		printk("Error \n");
		return -1;
	}
	return 0;
}




#endif