/*-----------------------------------------------------------------------------
 * aml_video.c -- Elvis Yu
 *-----------------------------------------------------------------------------
 */

#include <common.h>
#include <video_fb.h>
#include <asm/arch/osd.h>
#include <asm/arch/osd_hw.h>

GraphicDevice aml_gdev;

/*-----------------------------------------------------------------------------
 * osd_init --
 *-----------------------------------------------------------------------------
 */
static void osd_layer_init(GraphicDevice gdev, int layer)
{
	printf("%s\n", __FUNCTION__);
	osd_init_hw();
    osd_setup(0,
                0,
                gdev.winSizeX,
                gdev.winSizeY,
                gdev.winSizeX,
                gdev.winSizeY * 2,
                0,
                0,
                gdev.winSizeX - 1,
                gdev.winSizeY - 1,
                gdev.frameAdrs,
                &default_color_format_array[gdev.gdfIndex],
                layer);
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
	char *layer_str;
	fb_addr = simple_strtoul (getenv ("fb_addr"), NULL, 16);
	display_width = simple_strtoul (getenv ("display_width"), NULL, 10);
	display_height = simple_strtoul (getenv ("display_height"), NULL, 10);
	display_bpp = simple_strtoul (getenv ("display_bpp"), NULL, 10);
	color_format_index = simple_strtoul (getenv ("display_color_format_index"), NULL, 10);
	layer_str = getenv ("display_layer");
	fg = simple_strtoul (getenv ("display_color_fg"), NULL, 10);
	bg = simple_strtoul (getenv ("display_color_bg"), NULL, 10);
	
	/* fill in Graphic Device */
	aml_gdev.frameAdrs = fb_addr;
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

