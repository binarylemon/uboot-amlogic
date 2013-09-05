//------------------------------------------------------------------------------
// Video
//------------------------------------------------------------------------------

//#define VIDEO_PATH_2      // If defined use video from path 2; if UN-defined used video from path 1.

#define VIC                 16                      // Video format identification code: 1920x1080p@59.94/60Hz
#define INTERLACE_MODE      0                       // 0=Progressive; 1=Interlace.
#define PIXEL_REPEAT_VENC   0                       // Pixel repeat factor seen in VENC
#define PIXEL_REPEAT_HDMI   0                       // Pixel repeat factor seen by HDMI TX

#define ACTIVE_PIXELS       (1920*(1+PIXEL_REPEAT_HDMI)) // Number of active pixels per line.
#define ACTIVE_LINES        (1080/(1+INTERLACE_MODE))    // Number of active lines per field.

#define LINES_F0            1125                    // Number of lines in the even field.
#define LINES_F1            1125                    // Number of lines in the odd field.

#define FRONT_PORCH         88                      // Number of pixels from DE Low to HSYNC high. 
#define HSYNC_PIXELS        44                      // Number of pixels of HSYNC pulse. 
#define BACK_PORCH          148                     // Number of pixels from HSYNC low to DE high.

#define EOF_LINES           4                       // HSYNC count between last line of active video and start of VSYNC 
                                                    // a.k.a. End of Field (EOF). In interlaced mode,
                                                    // HSYNC count will be eof_lines at the end of even field  
                                                    // and eof_lines+1 at the end of odd field.
#define VSYNC_LINES         5                       // HSYNC count of VSYNC assertion
                                                    // In interlaced mode VSYNC will be in-phase with HSYNC in the even field and 
                                                    // out-of-phase with HSYNC in the odd field.
#define SOF_LINES           36                      // HSYNC count between VSYNC de-assertion and first line of active video

#define HSYNC_POLARITY      1                       // HSYNC polarity invert 
#define VSYNC_POLARITY      1                       // HSYNC polarity invert

#define TOTAL_FRAMES        4                       // Number of frames to run in simulation

#define TX_INPUT_COLOR_DEPTH    1                   // Pixel bit width: 0=24-bit; 1=30-bit; 2=36-bit; 3=48-bit.
#define TX_OUTPUT_COLOR_DEPTH   0                   // Pixel bit width: 0=24-bit; 1=30-bit; 2=36-bit; 3=48-bit.
#define RX_INPUT_COLOR_DEPTH    0                   // Pixel bit width: 0=24-bit; 1=30-bit; 2=36-bit; 3=48-bit.
#define RX_OUTPUT_COLOR_DEPTH   1                   // Pixel bit width: 0=24-bit; 1=30-bit; 2=36-bit; 3=48-bit.

#define TX_INPUT_COLOR_FORMAT   1                   // Pixel format: 0=RGB444; 1=YCbCr444; 2=Rsrv; 3=YCbCr422.
#define TX_OUTPUT_COLOR_FORMAT  1                   // Pixel format: 0=RGB444; 1=YCbCr444; 2=Rsrv; 3=YCbCr422.
#define RX_INPUT_COLOR_FORMAT   1                   // Pixel format: 0=RGB444; 1=YCbCr444; 2=Rsrv; 3=YCbCr422.
#define RX_OUTPUT_COLOR_FORMAT  1                   // Pixel format: 0=RGB444; 1=YCbCr444; 2=Rsrv; 3=YCbCr422.

#define TX_INPUT_COLOR_RANGE    0                   // Pixel range: 0=16-235/240; 1=16-240; 2=1-254; 3=0-255.
#define TX_OUTPUT_COLOR_RANGE   0                   // Pixel range: 0=16-235/240; 1=16-240; 2=1-254; 3=0-255.
#define RX_INPUT_COLOR_RANGE    0                   // Pixel range: 0=16-235/240; 1=16-240; 2=1-254; 3=0-255.
#define RX_OUTPUT_COLOR_RANGE   0                   // Pixel range: 0=16-235/240; 1=16-240; 2=1-254; 3=0-255.

//------------------------------------------------------------------------------
// The following parameters are not to be modified
//------------------------------------------------------------------------------

#define TOTAL_PIXELS        (FRONT_PORCH+HSYNC_PIXELS+BACK_PORCH+ACTIVE_PIXELS) // Number of total pixels per line.
#define TOTAL_LINES         (LINES_F0+(LINES_F1*INTERLACE_MODE))                // Number of total lines per frame.
