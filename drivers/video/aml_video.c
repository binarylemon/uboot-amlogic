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
                &default_color_format_array[COLOR_INDEX_24_RGB],
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
	u32 fb_addr, display_width, display_height, display_bpp;
	char *layer_str;
	fb_addr = simple_strtoul (getenv ("fb_addr"), NULL, 16);
	display_width = simple_strtoul (getenv ("display_width"), NULL, 10);
	display_height = simple_strtoul (getenv ("display_height"), NULL, 10);
	display_bpp = simple_strtoul (getenv ("display_bpp"), NULL, 10);
	layer_str = getenv ("display_layer");
	
	
	/* fill in Graphic Device */
	aml_gdev.frameAdrs = fb_addr;
	aml_gdev.winSizeX = display_width;
	aml_gdev.winSizeY = display_height;
	aml_gdev.gdfBytesPP = display_bpp/8;

	switch(display_bpp)
	{
		case 8:
			aml_gdev.gdfIndex = GDF__8BIT_INDEX;	//or GDF__8BIT_332RGB
			break;
		case 16:
			aml_gdev.gdfIndex = GDF_16BIT_565RGB;
			break;	
		case 24:
			aml_gdev.gdfIndex = GDF_24BIT_888RGB;
			break;
		case 32:
			aml_gdev.gdfIndex = GDF_32BIT_X888RGB;
			break;
		default:
			printf("ERROR:env display_bpp invalid! display_bpp is %d\n", display_bpp);
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
		printf("ERROR:env display_layer invalid! display_layer is %s\n", display_bpp);
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

