/*-----------------------------------------------------------------------------
 * aml_video.c -- Elvis Yu
 *-----------------------------------------------------------------------------
 */

#include <common.h>
#include <video_fb.h>
#include <asm/arch/osd.h>
#include <asm/arch/osd_hw.h>
#include <stdio_dev.h>
#include <bmp_layout.h>
#include <amlogic/aml_tv.h>
#include <amlogic/vinfo.h>
#include <malloc.h>


GraphicDevice aml_gdev;

//set default LCD_BPP
#ifndef LCD_BPP
#define LCD_BPP LCD_COLOR24
#endif

/*-----------------------------------------------------------------------------
 * osd_init --
 *-----------------------------------------------------------------------------
 */
static void osd_layer_init(GraphicDevice gdev, int layer)
{
//	printf("%s\n", __FUNCTION__);
	osd_init_hw();
#ifdef CONFIG_OSD_SCALE_ENABLE
	osd_setup(0,
                0,
                gdev.fb_width,
                gdev.fb_height,
                gdev.fb_width,
                gdev.fb_height * 2,
                0,
                0,
                gdev.fb_width- 1,
                gdev.fb_height- 1,
                gdev.frameAdrs,
                &default_color_format_array[gdev.gdfIndex],
                layer);
#else
	osd_setup(0,
                0,
                gdev.winSizeX,
                gdev.winSizeY,
                gdev.winSizeX,
                gdev.winSizeY * 2,
                0,
                0,
                gdev.winSizeX- 1,
                gdev.winSizeY- 1,
                gdev.frameAdrs,
                &default_color_format_array[gdev.gdfIndex],
                layer);
#endif
}

/*-----------------------------------------------------------------------------
 * osd_init --
 *-----------------------------------------------------------------------------
 */
static void video_layer_init(GraphicDevice gdev)
{
	return;
}


/*-----------------------------------------------------------------------------
 * video_hw_init --
 *-----------------------------------------------------------------------------
 */
void *video_hw_init (void)
{	
	u32 fb_addr, display_width, display_height, display_bpp, color_format_index, fg, bg;
	u32 fb_width, fb_height;
	char *layer_str;
	fb_addr = simple_strtoul (getenv ("fb_addr"), NULL, 16);
#ifdef CONFIG_OSD_SCALE_ENABLE
	fb_width = simple_strtoul (getenv ("fb_width"), NULL, 10 );
	fb_height = simple_strtoul (getenv ("fb_height"), NULL, 10);
#else
	fb_width = 0;
	fb_height = 0;
#endif
	display_width = simple_strtoul (getenv ("display_width"), NULL, 10);
	display_height = simple_strtoul (getenv ("display_height"), NULL, 10);
	display_bpp = simple_strtoul (getenv ("display_bpp"), NULL, 10);
	color_format_index = simple_strtoul (getenv ("display_color_format_index"), NULL, 10);
	layer_str = getenv ("display_layer");
	fg = simple_strtoul (getenv ("display_color_fg"), NULL, 10);
	bg = simple_strtoul (getenv ("display_color_bg"), NULL, 10);
	
	/* fill in Graphic Device */
	aml_gdev.frameAdrs = fb_addr;
	aml_gdev.fb_width = fb_width;
	aml_gdev.fb_height = fb_height;
	aml_gdev.winSizeX = display_width;
	aml_gdev.winSizeY = display_height;
	aml_gdev.gdfBytesPP = display_bpp/8;
	aml_gdev.fg = fg;
	aml_gdev.bg = bg;

	//different method with other video gdfIndex
	//if((color_format_index < ARRAY_SIZE(default_color_format_array)) && (default_color_format_array[color_format_index] != INVALID_BPP_ITEM))
	if((color_format_index < ARRAY_SIZE(default_color_format_array)) && (default_color_format_array[color_format_index].color_index != COLOR_INDEX_NULL))
	{
		aml_gdev.gdfIndex = color_format_index;
	}
	else
	{
		printf("ERROR:env color_format_index invalid! color_format_index is %d\n", color_format_index);
		return NULL;
	}

	if(strcmp(layer_str, "osd1") == 0)
	{
		osd_layer_init(aml_gdev, OSD1);
	}
	else if(strcmp(layer_str, "osd2") == 0)
	{
		osd_layer_init(aml_gdev, OSD2);
	}
	else if(strcmp(layer_str, "video") == 0)
	{
		video_layer_init(aml_gdev);
	}
	else
	{
		printf("ERROR:env display_layer invalid! display_layer is %d\n", display_bpp);
		return NULL;
	}

	return (void *) &aml_gdev;

}

/*-----------------------------------------------------------------------------
 * video_set_lut --
 *-----------------------------------------------------------------------------
 */
void video_set_lut (
	unsigned int index,           /* color number */
	unsigned char r,              /* red */
	unsigned char g,              /* green */
	unsigned char b               /* blue */
	)
{
	return;
}


int video_display_bitmap(ulong bmp_image, int x, int y)
{
    vidinfo_t * info = NULL;
#if defined CONFIG_VIDEO_AMLLCD
    extern vidinfo_t panel_info;
    info = & panel_info;
#endif
#if defined CONFIG_VIDEO_AMLTVOUT
    extern vidinfo_t tv_info;
    info = & tv_info;
#endif
#if(LCD_BPP ==LCD_COLOR8)	
	ushort *cmap_base = NULL;
	unsigned long byte_width;
#endif	
	ushort i, j;
	uchar *fb;
	bmp_image_t *bmp=(bmp_image_t *)bmp_image;
	uchar *bmap;
	ushort padded_line;
	unsigned long width, height;
#ifdef CONFIG_OSD_SCALE_ENABLE
	unsigned long pheight = aml_gdev.fb_height;
	unsigned long pwidth = aml_gdev.fb_width;
#else
	unsigned long pwidth = info->vl_col;
#endif
	unsigned colors, bpix, bmp_bpix;
	unsigned long compression;
	int lcd_line_length = (pwidth * NBITS (info->vl_bpix)) / 8;
	char *layer_str = NULL;
	int osd_index = -1;

	layer_str = getenv ("display_layer");
	if(strcmp(layer_str, "osd1") == 0)
	{
		osd_index = 0;
	}
	else if(strcmp(layer_str, "osd2") == 0)
	{
		osd_index = 1;
	}

	if (!((bmp->header.signature[0]=='B') &&
		(bmp->header.signature[1]=='M'))) {
		printf ("Error: no valid bmp image at %lx\n", bmp_image);
		return 1;
	}

	width = le32_to_cpu (bmp->header.width);
	height = le32_to_cpu (bmp->header.height);
	bmp_bpix = le16_to_cpu(bmp->header.bit_count);
	colors = 1 << bmp_bpix;
	compression = le32_to_cpu (bmp->header.compression);

	bpix = NBITS(info->vl_bpix);

#ifdef CONFIG_OSD_SCALE_ENABLE
	if((x == -1) &&(y == -1))
	{
		if((width > pwidth) || (height > pheight))
		{
			x = 0;
			y = 0;
		}
		else
		{
			x = (pwidth - width) / 2;
			y = (pheight - height) / 2;
		}
	}
#else
	if((x == -1) &&(y == -1))
	{
		if((width > info->vl_col) || (height > info->vl_row))
		{
			x = 0;
			y = 0;
		}
		else
		{
			x = (info->vl_col - width) / 2;
			y = (info->vl_row - height) / 2;
		}
	}
#endif

	if ((bpix != 1) && (bpix != 8) && (bpix != 16) && (bpix != 24) && (bpix != 32)) {
		printf ("Error: %d bit/pixel mode, but BMP has %d bit/pixel\n",
			bpix, bmp_bpix);
		return 1;
	}

	/* We support displaying 8bpp BMPs on 16bpp LCDs */
	if (bpix != bmp_bpix && (bmp_bpix != 8 || bpix != 16)) {
		printf ("Error: %d bit/pixel mode, but BMP has %d bit/pixel\n",
			bpix,
			le16_to_cpu(bmp->header.bit_count));
		return 1;
	}

	debug ("Display-bmp: %d x %d  with %d colors\n",
		(int)width, (int)height, (int)colors);


	/*
	 *  BMP format for Monochrome assumes that the state of a
	 * pixel is described on a per Bit basis, not per Byte.
	 *  So, in case of Monochrome BMP we should align widths
	 * on a byte boundary and convert them from Bit to Byte
	 * units.
	 *  Probably, PXA250 and MPC823 process 1bpp BMP images in
	 * their own ways, so make the converting to be MCC200
	 * specific.
	 */
	padded_line = (width&0x3) ? ((width&~0x3)+4) : (width);

#ifdef CONFIG_SPLASH_SCREEN_ALIGN
	if (x == BMP_ALIGN_CENTER)
		x = max(0, (pwidth - width) / 2);
	else if (x < 0)
		x = max(0, pwidth - width + x + 1);

	if (y == BMP_ALIGN_CENTER)
		y = max(0, (info->vl_row - height) / 2);
	else if (y < 0)
		y = max(0, info->vl_row - height + y + 1);
#endif /* CONFIG_SPLASH_SCREEN_ALIGN */

#ifdef CONFIG_OSD_SCALE_ENABLE
	if ((x + width)>pwidth)
		width = pwidth - x;
	if ((y + height)>pheight)
		height = pheight - y;
#else
	if ((x + width)>pwidth)
		width = pwidth - x;
	if ((y + height)>info->vl_row)
		height = info->vl_row - y;
#endif

#ifdef CONFIG_OSD_SCALE_ENABLE
        if(getenv("bmp_osd_control")) {
                osd_enable_hw(simple_strtoul(getenv("bmp_osd_control"), NULL, 10), osd_index);
        }
        else{
                osd_enable_hw(0, osd_index);//default to close osd in 'bmp display'
        }
#endif
	bmap = (uchar *)bmp + le32_to_cpu (bmp->header.data_offset);
	fb   = (uchar *) (info->vd_base +
		(y + height - 1) * lcd_line_length + x*(LCD_BPP/8));

	debug("fb=0x%08x; bmap=0x%08x, width=%d, height= %d, lcd_line_length=%d, padded_line=%d\n",
			fb, bmap, width, height, lcd_line_length, padded_line);
	switch (bmp_bpix) {
#if(LCD_BPP ==LCD_COLOR8)
	case 8:
		if (bpix != 16)
			byte_width = width;
		else
			byte_width = width * 2;

		for (i = 0; i < height; ++i) {
			for (j = 0; j < width; j++) {
				if (bpix != 16) {
					*(fb++) = *(bmap++);
				} else {
					*(uint16_t *)fb = cmap_base[*(bmap++)];
					fb += sizeof(uint16_t) / sizeof(*fb);
				}
			}
			bmap += (width - padded_line);
			fb   -= (byte_width + lcd_line_length);
		}
		break;
#endif /* LCD_BPP ==LCD_COLOR8 */

#if(LCD_BPP ==LCD_COLOR16)
	case 16:
		for (i = 0; i < height; ++i) {
			for (j = 0; j < width; j++) {

				*(fb++) = *(bmap++);
				*(fb++) = *(bmap++);
			}
			bmap += (padded_line - width) * 2;
			fb   -= (width * 2 + lcd_line_length);
		}
		break;
#endif /* LCD_BPP ==LCD_COLOR16 */

#if(LCD_BPP ==LCD_COLOR24)
	case 24:
		for (i = 0; i < height; ++i) {
			for (j = 0; j < width; j++) {

				*(fb++) = *(bmap++);
				*(fb++) = *(bmap++);
				*(fb++) = *(bmap++);
			}
			bmap += (padded_line - width);
			fb   -= (width * 3 + lcd_line_length);
		}
		break;
#endif /* LCD_BPP ==LCD_COLOR24 */
#if(LCD_BPP ==LCD_COLOR32)
		case 32:
			for (i = 0; i < height; ++i) {
				for (j = 0; j < width; j++) {

					*(fb++) = *(bmap++);
					*(fb++) = *(bmap++);
					*(fb++) = *(bmap++);
					*(fb++) = *(bmap++);
				}
				bmap += (padded_line - width);
				fb   -= (width * 4 + lcd_line_length);
			}
			break;
#endif /* LCD_BPP ==LCD_COLOR32 */

	default:
		printf("ERROR LCD_BPP is %d\n", LCD_BPP);
		return (-1);
	};
	flush_cache((unsigned long)info->vd_base, info->vl_col*info->vl_row*info->vl_bpix/8);

	return (0);
}

int my_atoi(const char *str){
    int result = 0;
    int signal = 1;
	
    if((*str >= '0' && *str<='9') || *str == '-' || *str == '+'){
        if(*str == '-' || *str == '+'){  
            if(*str=='-')
                signal = -1; 
            str++;
        } 
    }else 
		return 0;
	
    while(*str >= '0' && *str <= '9')
        result = result * 10 + (*str++ -'0');

	return signal * result;
}

#ifdef CONFIG_OSD_SCALE_ENABLE
int getenv_int(char *env, int def){
    if (getenv(env) == NULL) {
		//printf("bmp scale:  def=%d\n", def);
        return def;
    } else {
    	//printf("bmp scale:   %s=%s\n", env, getenv(env));
        return my_atoi(getenv(env));
    }
}

int* get_window_axis(void) {
    char *mode = getenv("outputmode");
    int* axis = (int *)malloc(4*4);

    if (strcmp(mode, "480i") == 0 || strcmp(mode, "480cvbs") == 0) {
        axis[0] = getenv_int("480i_x", 0);
        axis[1] = getenv_int("480i_y", 0);
        axis[2] = getenv_int("480i_w", 720);
        axis[3] = getenv_int("480i_h", 480);
    } else if (strcmp(mode, "480p") == 0) {
        axis[0] = getenv_int("480p_x", 0);
        axis[1] = getenv_int("480p_y", 0);
        axis[2] = getenv_int("480p_w", 720);
        axis[3] = getenv_int("480p_h", 480);
    } else if (strcmp(mode, "576i") == 0 || strcmp(mode, "576cvbs") == 0) {
        axis[0] = getenv_int("576i_x", 0);
        axis[1] = getenv_int("576i_y", 0);
        axis[2] = getenv_int("576i_w", 720);
        axis[3] = getenv_int("576i_h", 576);
    } else if (strcmp(mode, "576p") == 0) {
        axis[0] = getenv_int("576p_x", 0);
        axis[1] = getenv_int("576p_y", 0);
        axis[2] = getenv_int("576p_w", 720);
        axis[3] = getenv_int("576p_h", 576);
    } else if (strcmp(mode, "720p") == 0 || strcmp(mode, "720p50hz") == 0) {
        axis[0] = getenv_int("720p_x", 0);
        axis[1] = getenv_int("720p_y", 0);
        axis[2] = getenv_int("720p_w", 1280);
        axis[3] = getenv_int("720p_h", 720);
    } else if (strcmp(mode, "1080i") == 0 || strcmp(mode, "1080i50hz") == 0) { 
        axis[0] = getenv_int("1080i_x", 0);
        axis[1] = getenv_int("1080i_y", 0);
        axis[2] = getenv_int("1080i_w", 1920);
        axis[3] = getenv_int("1080i_h", 1080);
    } else if (strcmp(mode, "4k2k24hz") == 0) {
        axis[0] = getenv_int("4k2k24hz_x", 0);
        axis[1] = getenv_int("4k2k24hz_y", 0);
        axis[2] = getenv_int("4k2k24hz_w", 3840);
        axis[3] = getenv_int("4k2k24hz_h", 2160);
    } else if (strcmp(mode, "4k2k25hz") == 0) {
        axis[0] = getenv_int("4k2k25hz_x", 0);
        axis[1] = getenv_int("4k2k25hz_y", 0);
        axis[2] = getenv_int("4k2k25hz_w", 3840);
        axis[3] = getenv_int("4k2k25hz_h", 2160);
    } else if (strcmp(mode, "4k2k30hz") == 0) {
        axis[0] = getenv_int("4k2k30hz_x", 0);
        axis[1] = getenv_int("4k2k30hz_y", 0);
        axis[2] = getenv_int("4k2k30hz_w", 3840);
        axis[3] = getenv_int("4k2k30hz_h", 2160);
    }  else if (strcmp(mode, "4k2k50hz420") == 0) {
        axis[0] = getenv_int("4k2k420_x", 0);
        axis[1] = getenv_int("4k2k420_y", 0);
        axis[2] = getenv_int("4k2k420_w", 3840);
        axis[3] = getenv_int("4k2k420_h", 2160);
    } else if (strcmp(mode, "4k2k60hz420") == 0) {
        axis[0] = getenv_int("4k2k420_x", 0);
        axis[1] = getenv_int("4k2k420_y", 0);
        axis[2] = getenv_int("4k2k420_w", 3840);
        axis[3] = getenv_int("4k2k420_h", 2160);
    } else if (strcmp(mode, "4k2ksmpte") == 0) {
        axis[0] = getenv_int("4k2ksmpte_x", 0);
        axis[1] = getenv_int("4k2ksmpte_y", 0);
        axis[2] = getenv_int("4k2ksmpte_w", 4096);
        axis[3] = getenv_int("4k2ksmpte_h", 2160);
    } else if (strcmp(mode, "1080p") == 0 || strcmp(mode, "1080p50hz") == 0 || strcmp(mode, "1080p24hz") == 0) {
        axis[0] = getenv_int("1080p_x", 0);
        axis[1] = getenv_int("1080p_y", 0);
        axis[2] = getenv_int("1080p_w", 1920);
        axis[3] = getenv_int("1080p_h", 1080);
    } else if ((strcmp(mode, "768p50hz") == 0) || (strcmp(mode, "768p60hz") == 0) ){
        axis[0] = getenv_int("768p_x", 0);
        axis[1] = getenv_int("768p_y", 0);
        axis[2] = getenv_int("768p_width", 1366);
        axis[3] = getenv_int("768p_height", 768);
    } else {
        axis[0] = getenv_int("1080p_x", 0);
        axis[1] = getenv_int("1080p_y", 0);
        axis[2] = getenv_int("1080p_w", 1920);
        axis[3] = getenv_int("1080p_h", 1080);
    }
    //printf("bmp scale:  mode=%s , x=%d, y=%d, w=%d, h=%d\n", mode, axis[0], axis[1], axis[2], axis[3]);
    return axis;
}


int video_scale_bitmap(void)
{
//	printf("video_scale_bitmap src width is %d, height is %d, dst width is %d, dst height is %d\n",
//			aml_gdev.fb_width, aml_gdev.fb_height, aml_gdev.winSizeX, aml_gdev.winSizeY);
	char *layer_str = NULL;
	int osd_index = -1;
	layer_str = getenv ("display_layer");
	int *axis = get_window_axis();
	if(strcmp(layer_str, "osd1") == 0)
	{
		osd_index = 0;
	}
	else if(strcmp(layer_str, "osd2") == 0)
	{
		osd_index = 1;
	}
#ifdef CONFIG_OSD_SUPERSCALE_ENABLE
	if ((aml_gdev.fb_width*2 != aml_gdev.winSizeX) || (aml_gdev.fb_height*2 != aml_gdev.winSizeY)) {
		osd_enable_hw(1, osd_index);
		return (-1);
	}
	osd_free_scale_mode_hw(osd_index, 2);
#else
	osd_free_scale_mode_hw(osd_index, 1);
#endif
	osd_set_free_scale_axis_hw(osd_index, 0,0,aml_gdev.fb_width-1,aml_gdev.fb_height-1);
	osd_set_window_axis_hw(osd_index,axis[0],axis[1],axis[0]+axis[2]-1,axis[1]+axis[3]-1);
	free(axis);
	osd_free_scale_enable_hw(osd_index, 0x10001);
	osd_enable_hw(1, osd_index);
	return (1);
}
#endif

void reset_console(void)
{
    vidinfo_t * info = NULL;
#if defined CONFIG_VIDEO_AMLLCD
    extern vidinfo_t panel_info;
    info = & panel_info;
#endif
#if defined CONFIG_VIDEO_AMLTVOUT
    extern vidinfo_t tv_info;
    info = & tv_info;
#endif
	info->console_col = 0;
#ifdef CONFIG_LCD_INFO_BELOW_LOGO
	info->console_row = 7 + BMP_LOGO_HEIGHT / VIDEO_FONT_HEIGHT;
#else
	info->console_row = 1;	/* leave 1 blank line below logo */
#endif
}
