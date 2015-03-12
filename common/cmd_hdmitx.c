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
#include <amlogic/hdmitx/hdmi.h>

static int do_hpd_detect(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
    int st = hdmitx_get_hpd_state();
    printf("%c\n", st ? '1' : '0');
    return st;
}

static unsigned char edid_raw_buf[256] = {0};
static void dump_edid_raw(unsigned char *buf)
{
    int i = 0;
    for ( i = 0; i < 8; i++ )
    {
        printf("%02x ", buf[i]);
    }
    printf("\n");
}

static int do_edid(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
    unsigned int tmp_addr = 0;
    unsigned char edid_addr = 0;
    unsigned char st = 0;

    memset(edid_raw_buf, 0, ARRAY_SIZE(edid_raw_buf));
    if (argc < 2)
        return cmd_usage(cmdtp);
    if (strcmp(argv[1], "read") == 0) {
        tmp_addr = simple_strtoul(argv[2], NULL, 16);
        if (tmp_addr > 0xff)
            return cmd_usage(cmdtp);
        edid_addr = tmp_addr;
        // read edid raw data
        // current only support read 1 byte edid data
        st = hdmitx_read_edid(&edid_raw_buf[edid_addr & 0xf8], edid_addr & 0xf8, 8);
        printf("edid[0x%02x]: 0x%02x\n", edid_addr, edid_raw_buf[edid_addr]);
        if (0)      // Debug only
            dump_edid_raw(&edid_raw_buf[edid_addr & 0xf8]);
        if (!st) {
            printf("edid read failed\n");
        }
    }
    return st;
}

static int do_rx_det(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
    unsigned char edid_addr = 0xf8;     // Fixed Address
    unsigned char st = 0;

    memset(edid_raw_buf, 0, ARRAY_SIZE(edid_raw_buf));

    // read edid raw data
    // current only support read 1 byte edid data
    st = hdmitx_read_edid(&edid_raw_buf[edid_addr & 0xf8], edid_addr & 0xf8, 8);
    if (1)      // Debug only
        dump_edid_raw(&edid_raw_buf[edid_addr & 0xf8]);
    if (st) {
#if 0
        // set fake value
        edid_raw_buf[250] = 0xfb;
        edid_raw_buf[251] = 0x0c;
        edid_raw_buf[252] = 0x01;
#endif
        if ((edid_raw_buf[250] == 0xfb) & (edid_raw_buf[251] == 0x0c)) {
            printf("RX is FBC\n");

            // set outputmode ENV
            switch (edid_raw_buf[252] & 0x0f) {
            case 0x0:
                run_command("setenv outputmode 1080p", 0);
                break;
            case 0x1:
                run_command("setenv outputmode 4k2k60hz420", 0);
                break;
            case 0x2:
                run_command("setenv outputmode 1080p", 0);
                break;
            default:
                run_command("setenv outputmode 1080p", 0);
                break;
            }

            // set RX 3D Info
            switch ((edid_raw_buf[252] >> 4) & 0x0f) {
            case 0x00:
                run_command("setenv rx_3d_info 0", 0);
                break;
            case 0x01:
                run_command("setenv rx_3d_info 1", 0);
                break;
            case 0x02:
                run_command("setenv rx_3d_info 2", 0);
                break;
            case 0x03:
                run_command("setenv rx_3d_info 3", 0);
                break;
            case 0x04:
                run_command("setenv rx_3d_info 4", 0);
                break;
            default:
                break;
            }

            switch (edid_raw_buf[253]) {
            case 0x1:
                run_command("setenv lcd_reverse 1", 0);
                break;
            case 0x2:
                // TODO
                break;
            default:
                break;
            }
        }
    } else {
        printf("edid read failed\n");
    }

    return st;
}

static cmd_tbl_t cmd_hdmi_sub[] = {
    U_BOOT_CMD_MKENT(hpd, 1, 1, do_hpd_detect, "", ""),
    U_BOOT_CMD_MKENT(edid, 3, 1, do_edid, "", ""),
    U_BOOT_CMD_MKENT(rx_det, 1, 1, do_rx_det, "", ""),
};

static int do_hdmitx(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *c;

	if (argc < 2)
		return cmd_usage(cmdtp);

	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_hdmi_sub[0], ARRAY_SIZE(cmd_hdmi_sub));

	if (c)
		return  c->cmd(cmdtp, flag, argc, argv);
	else
		return cmd_usage(cmdtp);
}

U_BOOT_CMD(hdmitx, CONFIG_SYS_MAXARGS, 1, do_hdmitx,
	"HDMITX sub-system",
    "hdmitx hpd\n"
    "    Detect hdmi rx plug-in\n"
	"hdmitx edid read ADDRESS\n"
    "    Read hdmi rx edid from ADDRESS(0x00~0xff)\n"
    "hdmitx rx_det\n"
    "    Auto detect if RX is FBC and set outputmode\n"
);
