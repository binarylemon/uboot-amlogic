#include <asm/arch/mipi_dsi_reg.h>
#include <asm/arch/io.h>
#include <asm/arch/lcd_reg.h>
#include <amlogic/aml_lcd_extern.h>
#include "mipi_dsi_util.h"

//#define PRINT_DEBUG_INFO
#ifdef PRINT_DEBUG_INFO
#define DPRINT(...)		printf(__VA_ARGS__)
#else
#define DPRINT(...)
#endif

//===============================================================================
// Define MIPI DSI Default config
//===============================================================================
#define MIPI_DSI_VIRTUAL_CHAN_ID        0                       // Range [0,3]
#define MIPI_DSI_CMD_TRANS_TYPE         DCS_TRANS_LP            // Define DSI command transfer type: high speed or low power
#define MIPI_DSI_DCS_ACK_TYPE           MIPI_DSI_DCS_NO_ACK     // Define if DSI command need ack: req_ack or no_ack
#define MIPI_DSI_VIDEO_MODE_TYPE        BURST_MODE              // Applicable only to video mode. Define picture data transfer method: non-burst sync pulse; non-burst sync event; or burst.
#define MIPI_DSI_TEAR_SWITCH            MIPI_DCS_DISABLE_TEAR
//===============================================================================

#define RETRY_CNT       2000
#define WARNNING_CNT    100

static inline void print_mipi_cmd_status(int cnt)
{
    if (cnt <= WARNNING_CNT) {
        printf("cmd error: status=0x%04x, int0=0x%06x, int1=0x%06x\n", READ_LCD_REG(MIPI_DSI_DWC_CMD_PKT_STATUS_OS), READ_LCD_REG(MIPI_DSI_DWC_INT_ST0_OS), READ_LCD_REG(MIPI_DSI_DWC_INT_ST1_OS));
    }
}

static void print_info(DSI_Config_t *cfg)
{
    printf("================================================\n");
    printf("MIPI DSI Config\n");
    printf(" Lane Num:              %d\n", cfg->lane_num);
    printf(" Bit Rate min:          %dMHz\n", (cfg->bit_rate_min / 1000));
    printf(" Bit Rate max:          %dMHz\n", (cfg->bit_rate_max / 1000));
    printf(" Bit Rate:              %d.%03dMHz\n", (cfg->bit_rate / 1000000), (cfg->bit_rate % 1000000) / 1000);
    printf(" Pclk lanebyte factor:  %d\n", ((cfg->factor_numerator * 100 / cfg->factor_denominator) + 5) / 10);
    printf(" Operation mode:        %s\n", (cfg->operation_mode == OPERATION_COMMAND_MODE) ? "COMMAND":"VIDEO");
    printf(" Transfer control:       %d\n", cfg->transfer_ctrl);
    if(cfg->video_mode_type == NON_BURST_SYNC_PULSE) {
        printf(" Video mode type:       NON_BURST_SYNC_PULSE\n");
    }
    else if(cfg->video_mode_type == NON_BURST_SYNC_EVENT) {
        printf(" Video mode type:       NON_BURST_SYNC_EVENT\n");
    }
    else if(cfg->video_mode_type == BURST_MODE) {
        printf(" Video mode type:       BURST_MODE\n");
    }

    printf(" Venc format:           %d\n", cfg->venc_fmt);

    switch(cfg->dpi_data_format) {
        case COLOR_16BIT_CFG_1  :
            printf(" Data Format:           COLOR_16BIT_CFG_1\n");
            break;
        case COLOR_16BIT_CFG_2  :
            printf(" Data Format:           COLOR_16BIT_CFG_2\n");
            break;
        case COLOR_16BIT_CFG_3  :
            printf(" Data Format:           COLOR_16BIT_CFG_3\n");
            break;
        case COLOR_18BIT_CFG_1  :
            printf(" Data Format:           COLOR_18BIT_CFG_1\n");
            break;
        case COLOR_18BIT_CFG_2  :
            printf(" Data Format:           COLOR_18BIT_CFG_2\n");
            break;
        case COLOR_24BIT        :
            printf(" Data Format:           COLOR_24BIT\n");
            break;
        case COLOR_20BIT_LOOSE  :
            printf(" Data Format:           COLOR_20BIT_LOOSE\n");
            break;
        case COLOR_24_BIT_YCBCR :
            printf(" Data Format:           COLOR_24BIT_YCBCR\n");
            break;
        case COLOR_16BIT_YCBCR  :
            printf(" Data Format:           COLOR_16BIT_YCBCR\n");
            break;
        case COLOR_30BIT        :
            printf(" Data Format:           COLOR_30BIT\n");
            break;
        case COLOR_36BIT        :
            printf(" Data Format:           COLOR_36BIT\n");
            break;
        case COLOR_12BIT        :
            printf(" Data Format:           COLOR_12BIT\n");
            break;
        case COLOR_RGB_111      :
            printf(" Data Format:           COLOR_RGB_111\n");
            break;
        case COLOR_RGB_332      :
            printf(" Data Format:           COLOR_RGB332\n");
            break;
        case COLOR_RGB_444      :
            printf(" Data Format:           COLOR_RGB444\n");
            break;
        default            :
            printf(" Error: un-support Data Format, Please Check\n");
            break;
    }
    //printf(" POLARITY:              HIGH ACTIVE\n");
    printf(" Enable CRC/ECC/BTA\n");
    printf("================================================\n");
}

// -----------------------------------------------------------------------------
//                     Function: check_phy_st
// Check the status of the dphy: phylock and stopstateclklane, to decide if the DPHY is ready
// -----------------------------------------------------------------------------
static void check_phy_status(void)
{
    while((( READ_LCD_REG(MIPI_DSI_DWC_PHY_STATUS_OS ) >> BIT_PHY_LOCK) & 0x1) == 0){
        udelay(6);
    }
    while((( READ_LCD_REG(MIPI_DSI_DWC_PHY_STATUS_OS ) >> BIT_PHY_STOPSTATECLKLANE) & 0x1) == 0){
        DPRINT(" Waiting STOP STATE LANE\n");
        udelay(6);
    }
}

// -----------------------------------------------------------------------------
//                     Function: set_mipi_dcs
// Configure relative registers in command mode
// -----------------------------------------------------------------------------
static void set_mipi_dcs(int trans_type,        // 0: high speed, 1: low power
                         int req_ack,           // 1: request ack, 0: do not need ack
                         int tear_en)           // 1: enable tear ack, 0: disable tear ack
{
    WRITE_LCD_REG( MIPI_DSI_DWC_CMD_MODE_CFG_OS, (trans_type << BIT_MAX_RD_PKT_SIZE) | (trans_type << BIT_DCS_LW_TX)    |
                    (trans_type << BIT_DCS_SR_0P_TX)    | (trans_type << BIT_DCS_SW_1P_TX) |
                    (trans_type << BIT_DCS_SW_0P_TX)    | (trans_type << BIT_GEN_LW_TX)    |
                    (trans_type << BIT_GEN_SR_2P_TX)    | (trans_type << BIT_GEN_SR_1P_TX) |
                    (trans_type << BIT_GEN_SR_0P_TX)    | (trans_type << BIT_GEN_SW_2P_TX) |
                    (trans_type << BIT_GEN_SW_1P_TX)    | (trans_type << BIT_GEN_SW_0P_TX) |
                    (req_ack    << BIT_ACK_RQST_EN)     | (tear_en    << BIT_TEAR_FX_EN)  );

    if (tear_en == MIPI_DCS_ENABLE_TEAR) {
        // Enable Tear Interrupt if tear_en is valid
        WRITE_LCD_REG( MIPI_DSI_TOP_INTR_CNTL_STAT, (READ_LCD_REG(MIPI_DSI_TOP_INTR_CNTL_STAT) | (0x1<<BIT_EDPITE_INT_EN)) );
        // Enable Measure Vsync
        WRITE_LCD_REG( MIPI_DSI_TOP_MEAS_CNTL, (READ_LCD_REG(MIPI_DSI_TOP_MEAS_CNTL) | (0x1<<BIT_VSYNC_MEAS_EN | (0x1<<BIT_TE_MEAS_EN))));
    }
}
// -----------------------------------------------------------------------------
//                     Function: set_mipi_int
// Configure relative registers for mipi interrupt
// -----------------------------------------------------------------------------
static void set_mipi_int(void)
{
    WRITE_LCD_REG( MIPI_DSI_DWC_INT_MSK0_OS, 0);
    WRITE_LCD_REG( MIPI_DSI_DWC_INT_MSK1_OS, 0);
}

// ----------------------------------------------------------------------------
// Function: wait_bta_ack
// Poll to check if the BTA ack is finished
// ----------------------------------------------------------------------------
static void wait_bta_ack(void)
{
    unsigned int phy_status;

    // Check if phydirection is RX
    do {
        phy_status = READ_LCD_REG( MIPI_DSI_DWC_PHY_STATUS_OS );
    } while(((phy_status & 0x2) >> BIT_PHY_DIRECTION) == 0x0);

    // Check if phydirection is return to TX
    do {
        phy_status = READ_LCD_REG( MIPI_DSI_DWC_PHY_STATUS_OS );
    } while(((phy_status & 0x2) >> BIT_PHY_DIRECTION) == 0x1);
}

// ----------------------------------------------------------------------------
// Function: wait_cmd_fifo_empty
// Poll to check if the generic command fifo is empty
// ----------------------------------------------------------------------------
static void wait_cmd_fifo_empty(void)
{
    unsigned int cmd_status;
    int i= RETRY_CNT;

    do {
        i--;
        print_mipi_cmd_status(i);
        cmd_status = READ_LCD_REG(MIPI_DSI_DWC_CMD_PKT_STATUS_OS);
    } while((((cmd_status >> BIT_GEN_CMD_EMPTY) & 0x1) != 0x1) && (i!=0));
}

// ----------------------------------------------------------------------------
// Function: wait_for_generic_read_response
// Wait for generic read response
// ----------------------------------------------------------------------------
static unsigned int wait_for_generic_read_response(void)
{
    unsigned int timeout, phy_status, data_out;

    phy_status = READ_LCD_REG( MIPI_DSI_DWC_PHY_STATUS_OS );
    for(timeout=0; timeout<50; timeout++) {
        if(((phy_status & 0x40)>> BIT_PHY_RXULPSESC0LANE) == 0x0)
            break;
        phy_status = READ_LCD_REG( MIPI_DSI_DWC_PHY_STATUS_OS );
        udelay(1);
    }
    phy_status = READ_LCD_REG( MIPI_DSI_DWC_PHY_STATUS_OS );
    for(timeout=0; timeout<50; timeout++) {
        if(((phy_status & 0x40)>> BIT_PHY_RXULPSESC0LANE) == 0x1)
            break;
        phy_status = READ_LCD_REG( MIPI_DSI_DWC_PHY_STATUS_OS );
        udelay(1);
    }

    data_out = READ_LCD_REG( MIPI_DSI_DWC_GEN_PLD_DATA_OS );
    return data_out;
}

// ----------------------------------------------------------------------------
// Function: generic_if_wr
// Generic interface write, address has to be MIPI_DSI_DWC_GEN_PLD_DATA_OS and
// MIPI_DSI_DWC_GEN_HDR_OS, MIPI_DSI_DWC_GEN_VCID_OS
// ----------------------------------------------------------------------------
static unsigned int generic_if_wr(unsigned int address, unsigned int data_in)
{
    if(address != MIPI_DSI_DWC_GEN_HDR_OS && address != MIPI_DSI_DWC_GEN_PLD_DATA_OS) {
        DPRINT(" Error Address : 0x%x\n", address);
    }

    DPRINT("address 0x%x = 0x%08x\n", address, data_in);
    WRITE_LCD_REG(address, data_in);

    return 0;
}

// ----------------------------------------------------------------------------
//                           Function: generic_if_rd
// Generic interface read, address has to be MIPI_DSI_DWC_GEN_PLD_DATA_OS
// ----------------------------------------------------------------------------
static unsigned int generic_if_rd(unsigned int address)
{
    unsigned int data_out;

    if(address != MIPI_DSI_DWC_GEN_PLD_DATA_OS) {
        DPRINT(" Error Address : %x\n", address);
    }

    data_out = READ_DSI_REG(address);

    return data_out;
}

// ----------------------------------------------------------------------------
// Function: DCS_write_short_packet_0_para
// DCS Write Short Packet 0 Parameter with Generic Interface
// Supported DCS Command: DCS_ENTER_SLEEP_MODE/DCS_EXIT_IDLE_MODE/DCS_EXIT_INVERT_MODE
//                        DCS_EXIT_SLEEP_MODE/DCS_SET_DISPLAY_OFF/DCS_SET_DISPLAY_ON
//                        DCS_SET_TEAR_OFF/DCS_SOFT_RESET/DCS_NOP
// ----------------------------------------------------------------------------
static void DCS_write_short_packet_0_para(unsigned int data_type,
                                          unsigned int vc_id,
                                          unsigned int dcs_command,
                                          unsigned int req_ack)
{
    generic_if_wr(MIPI_DSI_DWC_GEN_HDR_OS, ((0x0 << BIT_GEN_WC_MSBYTE) |
                            (dcs_command << BIT_GEN_WC_LSBYTE) |
                            (vc_id << BIT_GEN_VC)              |
                            (data_type << BIT_GEN_DT)));
    if( req_ack == MIPI_DSI_DCS_REQ_ACK ) {
        wait_bta_ack();
    }
    else if( req_ack == MIPI_DSI_DCS_NO_ACK ) {
        wait_cmd_fifo_empty();
    }
}

// ----------------------------------------------------------------------------
// Function: DCS_write_short_packet_1_para
// DCS Write Short Packet 1 Parameter with Generic Interface
// Supported DCS Command: DCS_SET_ADDRESS_MODE/DCS_SET_GAMMA_CURVE/
//                        DCS_SET_PIXEL_FORMAT/DCS_SET_TEAR_ON
// ----------------------------------------------------------------------------
static void DCS_write_short_packet_1_para(unsigned int data_type,
                                          unsigned int vc_id,
                                          unsigned int dcs_command,
                                          unsigned int para,
                                          unsigned int req_ack)
{
    // DPRINT(" para is %x, dcs_command is %x\n", para, dcs_command);
    // DPRINT(" vc_id %x, data_type is %x\n", vc_id, data_type);
    generic_if_wr(MIPI_DSI_DWC_GEN_HDR_OS, ((para << BIT_GEN_WC_MSBYTE)        |
                            (dcs_command << BIT_GEN_WC_LSBYTE) |
                            (vc_id << BIT_GEN_VC)              |
                            (data_type << BIT_GEN_DT)));
    if( req_ack == MIPI_DSI_DCS_REQ_ACK ) {
        wait_bta_ack();
    }
    else if( req_ack == MIPI_DSI_DCS_NO_ACK ) {
        wait_cmd_fifo_empty();
    }
}

// ----------------------------------------------------------------------------
//                           Function: DCS_read_packet_no_para
// DCS Write Short Packet 1 Parameter with Generic Interface
// Supported DCS Command: DCS_SET_ADDRESS_MODE/DCS_SET_GAMMA_CURVE/
//                        DCS_SET_PIXEL_FORMAT/DCS_SET_TEAR_ON
// ----------------------------------------------------------------------------
static unsigned int DCS_read_packet_no_para(unsigned int data_type,
                                            unsigned int vc_id,
                                            unsigned int dcs_command)
{
    unsigned read_data;

    // DPRINT(" para is %x, dcs_command is %x\n", para, dcs_command);
    // DPRINT(" vc_id %x, data_type is %x\n", vc_id, data_type);
    generic_if_wr(MIPI_DSI_DWC_GEN_HDR_OS, ((0x0 << BIT_GEN_WC_MSBYTE)         |
                            (dcs_command << BIT_GEN_WC_LSBYTE) |
                            (vc_id << BIT_GEN_VC)              |
                            (data_type << BIT_GEN_DT)));

    read_data = wait_for_generic_read_response();

    return read_data;
}

// ----------------------------------------------------------------------------
//                           Function: DCS_long_write_packet
// DCS Long Write Packet
// Supported DCS Command: DCS_SET_COLUMN_ADDRESS/DCS_SET_PAGE_ADDRESS
//                        DCS_WRITE_MEMORY_START/DCS_WRITE_MEMORY_CONTINUE
// ----------------------------------------------------------------------------
static void DCS_long_write_packet(unsigned int data_type,    // DSI data type, such as DCS Long Write Packet
                                  unsigned int vc_id,        // Virtual Channel ID
                                  unsigned int dcs_command,  // DCS Command, such as set_column_address/set_page_address
                                  unsigned char* payload,    // Payload(including dcs_cmd+payload)
                                  unsigned int pld_size,     // Payload size, from LSB to MSB, include dcs_cmd+payload
                                  unsigned int req_ack)      // if need check ack for bta
{
    unsigned int payload_data=0, header_data;
    unsigned int cmd_status;
    unsigned int i;
    int j;

    // Write Payload Register First
    for(i=0; i<pld_size; i++) {
        if(i%4 == 0)    { payload_data = 0; }
        payload_data = payload_data | (payload[i] << 8*(i%4));

        if(i%4 == 3 || i == pld_size-1) {                                      // when last byte
            // Check the pld fifo status before write to it, do not need check every word
            if(i == pld_size/3 || i == pld_size/2) {
                j = RETRY_CNT;
                do {
                    j--;
                    print_mipi_cmd_status(j);
                    cmd_status = READ_LCD_REG( MIPI_DSI_DWC_CMD_PKT_STATUS_OS);
                } while((((cmd_status >> BIT_GEN_PLD_W_FULL) & 0x1) == 0x1) && (j>0));
            }
            if(dcs_command == DCS_WRITE_MEMORY_CONTINUE) {
                WRITE_LCD_REG( MIPI_DSI_DWC_GEN_PLD_DATA_OS, payload_data);               // Use direct memory write to save time when in WRITE_MEMORY_CONTINUE
                // (*(MIPI_DSI_DWC_GEN_PLD_DATA_OS|0xc1100000)) = payload_data;
                // *(volatile unsigned long *)((0xc1100000)|((MIPI_DSI_DWC_GEN_PLD_DATA_OS)<<2))=payload_data;
            }
            else {
                generic_if_wr(MIPI_DSI_DWC_GEN_PLD_DATA_OS, payload_data);
            }
        }
    }

    // Check cmd fifo status before write to it
    j = RETRY_CNT;
    do {
        j--;
        print_mipi_cmd_status(j);
        cmd_status = READ_LCD_REG( MIPI_DSI_DWC_CMD_PKT_STATUS_OS);
    } while((((cmd_status >> BIT_GEN_CMD_FULL) & 0x1) == 0x1) && (j>0));
    // Write Header Register
    header_data = (((((pld_size) & 0xff00) >> 8) << BIT_GEN_WC_MSBYTE) |
                    (((pld_size) & 0x00ff) << BIT_GEN_WC_LSBYTE)        |
                    (vc_id << BIT_GEN_VC)                               |
                    (data_type << BIT_GEN_DT));
    generic_if_wr(MIPI_DSI_DWC_GEN_HDR_OS, header_data);

    // Check BTA ACK
    if (req_ack == MIPI_DSI_DCS_REQ_ACK) {
        wait_bta_ack();
    }
    else if (req_ack == MIPI_DSI_DCS_NO_ACK) {
        wait_cmd_fifo_empty();
    }
}

static void dsi_phy_init(unsigned char lane_num)
{
    // enable phy clock.
    WRITE_DSI_REG(MIPI_DSI_PHY_CTRL,  0x1);          //enable DSI top clock.
    WRITE_DSI_REG(MIPI_DSI_PHY_CTRL,  0x1 |          //enable the DSI PLL clock .
                    (1 << 7 )  |   //enable pll clock which connected to  DDR clock path
                    (1 << 8 )  |   //enable the clock divider counter
                    (0 << 9 )  |   //enable the divider clock out
                    (0 << 10 ) |   //clock divider. 1: freq/4, 0: freq/2
                    (0 << 11 ) |   //1: select the mipi DDRCLKHS from clock divider, 0: from PLL clock
                    (0 << 12));    //enable the byte clock generateion.

    WRITE_DSI_REG(MIPI_DSI_PHY_CTRL,  0x1 |          //enable DSI top clock.
                    (1 << 7 )  |   //enable pll clock which connected to  DDR clock path
                    (1 << 8 )  |   //enable the clock divider counter
                    (1 << 9 )  |   //enable the divider clock out
                    (0 << 10 ) |   //clock divider. 1: freq/4, 0: freq/2
                    (0 << 11 ) |   //1: select the mipi DDRCLKHS from clock divider, 0: from PLL clock
                    (0 << 12));    //enable the byte clock generateion.

    WRITE_DSI_REG(MIPI_DSI_PHY_CTRL,  0x1 |          //enable DSI top clock.
                    (1 << 7 )  |   //enable pll clock which connected to  DDR clock path
                    (1 << 8 )  |   //enable the clock divider counter
                    (1 << 9 )  |   //enable the divider clock out
                    (0 << 10 ) |   //clock divider. 1: freq/4, 0: freq/2
                    (0 << 11 ) |   //1: select the mipi DDRCLKHS from clock divider, 0: from PLL clock
                    (1 << 12));    //enable the byte clock generateion.

    WRITE_DSI_REG_BITS(MIPI_DSI_PHY_CTRL,  1, 31, 1);
    WRITE_DSI_REG_BITS(MIPI_DSI_PHY_CTRL,  0, 31, 1);

    WRITE_DSI_REG(MIPI_DSI_CLK_TIM,  0x05210f08);//0x03211c08
    WRITE_DSI_REG(MIPI_DSI_CLK_TIM1, 0x8);//??
    WRITE_DSI_REG(MIPI_DSI_HS_TIM, 0x060f090d);//0x050f090d
    WRITE_DSI_REG(MIPI_DSI_LP_TIM, 0x4a370e0e);
    WRITE_DSI_REG(MIPI_DSI_ANA_UP_TIM, 0x0100); //?? //some number to reduce sim time.
    WRITE_DSI_REG(MIPI_DSI_INIT_TIM, 0x00d4); //0xe20   //30d4 -> d4 to reduce sim time.
    WRITE_DSI_REG(MIPI_DSI_WAKEUP_TIM, 0x48); //0x8d40  //1E848-> 48 to reduct sim time.
    WRITE_DSI_REG(MIPI_DSI_LPOK_TIM,  0x7C);   //wait for the LP analog ready.
    WRITE_DSI_REG(MIPI_DSI_ULPS_CHECK,  0x927C);   //1/3 of the tWAKEUP.
    WRITE_DSI_REG(MIPI_DSI_LP_WCHDOG,  0x1000);   // phy TURN watch dog.
    WRITE_DSI_REG(MIPI_DSI_TURN_WCHDOG,  0x1000);   // phy ESC command watch dog.

    // Powerup the analog circuit.
    switch (lane_num) {
        case 1:
            WRITE_DSI_REG(MIPI_DSI_CHAN_CTRL, 0x0e);
            break;
        case 2:
            WRITE_DSI_REG(MIPI_DSI_CHAN_CTRL, 0x0c);
            break;
        case 3:
            WRITE_DSI_REG(MIPI_DSI_CHAN_CTRL, 0x08);
            break;
        case 4:
        default:
            WRITE_DSI_REG(MIPI_DSI_CHAN_CTRL, 0);
            break;
    }
}

static void dsi_phy_config(Lcd_Config_t *pConf)
{
    DPRINT("%s\n", __func__);
    //Digital
    // Power up DSI
    WRITE_LCD_REG(MIPI_DSI_DWC_PWR_UP_OS, 1);

    // Setup Parameters of DPHY
    WRITE_LCD_REG(MIPI_DSI_DWC_PHY_TST_CTRL1_OS, 0x00010044);                            // testcode
    WRITE_LCD_REG(MIPI_DSI_DWC_PHY_TST_CTRL0_OS, 0x2);
    WRITE_LCD_REG(MIPI_DSI_DWC_PHY_TST_CTRL0_OS, 0x0);
    WRITE_LCD_REG(MIPI_DSI_DWC_PHY_TST_CTRL1_OS, 0x00000074);                            // testwrite
    WRITE_LCD_REG(MIPI_DSI_DWC_PHY_TST_CTRL0_OS, 0x2);
    WRITE_LCD_REG(MIPI_DSI_DWC_PHY_TST_CTRL0_OS, 0x0);

    // Power up D-PHY
    WRITE_LCD_REG(MIPI_DSI_DWC_PHY_RSTZ_OS, 0xf);

    //Analog
    dsi_phy_init(pConf->lcd_control.mipi_config->lane_num);

    // Check the phylock/stopstateclklane to decide if the DPHY is ready
    check_phy_status();

    // Trigger a sync active for esc_clk
    WRITE_DSI_REG(MIPI_DSI_PHY_CTRL, READ_DSI_REG(MIPI_DSI_PHY_CTRL) | (1 << 1));
}

static void dsi_video_config(Lcd_Config_t *pConf)
{
    DSI_Config_t *cfg= pConf->lcd_control.mipi_config;

    DPRINT(" ============= VIDEO TIMING SETTING =============\n");
    DPRINT(" HLINE        = %d\n", cfg->hline);
    DPRINT(" HSA          = %d\n", cfg->hsa);
    DPRINT(" HBP          = %d\n", cfg->hbp);
    DPRINT(" VSA          = %d\n", cfg->vsa);
    DPRINT(" VBP          = %d\n", cfg->vbp);
    DPRINT(" VFP          = %d\n", cfg->vfp);
    DPRINT(" VACT         = %d\n", cfg->vact);
    DPRINT(" ================================================\n");

    WRITE_LCD_REG( MIPI_DSI_DWC_VID_HLINE_TIME_OS,    cfg->hline);
    WRITE_LCD_REG( MIPI_DSI_DWC_VID_HSA_TIME_OS,      cfg->hsa);
    WRITE_LCD_REG( MIPI_DSI_DWC_VID_HBP_TIME_OS,      cfg->hbp);
    WRITE_LCD_REG( MIPI_DSI_DWC_VID_VSA_LINES_OS,     cfg->vsa);
    WRITE_LCD_REG( MIPI_DSI_DWC_VID_VBP_LINES_OS,     cfg->vbp);
    WRITE_LCD_REG( MIPI_DSI_DWC_VID_VFP_LINES_OS,     cfg->vfp);
    WRITE_LCD_REG( MIPI_DSI_DWC_VID_VACTIVE_LINES_OS, cfg->vact);
}

static void startup_transfer_cmd(void)
{
    // Startup transfer
    WRITE_LCD_REG( MIPI_DSI_DWC_LPCLK_CTRL_OS, (0x1 << BIT_AUTOCLKLANE_CTRL) | (0x1 << BIT_TXREQUESTCLKHS));
}
static void startup_transfer_video(void)
{
    WRITE_LCD_REG( MIPI_DSI_DWC_LPCLK_CTRL_OS, (0x1 << BIT_TXREQUESTCLKHS));
}

static void set_mipi_dsi_host(int lane_num,                      // lane number, from 1 to 4
                              int vcid,                          // virtual id
                              int venc_data_width,               // VENC output data width
                              int dpi_data_format,               // dpi data format
                              int chroma_subsample,              // chroma_subsample for YUV422 or YUV420 only
                              int operation_mode,                // video mode/command mode
                              tv_enc_lcd_type_t output_type,     // video type, such as 1080x720
                              int vid_mode_type,                 // video mode : burst/non_burst
                              int check_phy_status,              // enable/disable phy lock check, disable for multiple pic test
                              Lcd_Config_t *p)
{
    int real_lane_num = lane_num+1;
    int num_of_chunk;
    int pixel_per_chunk = 4;
    int byte_per_chunk=0;
    int totol_bytes_per_chunk;
    int chunk_overhead;
    int vid_null_size=0;

    totol_bytes_per_chunk = real_lane_num*pixel_per_chunk*3/4;

    // one lene has 8 bytes for 4 pixels
    // according to DSI spec line50
    switch(dpi_data_format) {
        case COLOR_24_BIT_YCBCR :
        case COLOR_24BIT        :
            byte_per_chunk = 18;                                             // at least 3 lanes(12+6)=18: 3*8-18=6>=6
            // byte_per_chunk = 12;                                             // at least 3 lanes(12+6)=18: 3*8-18=6>=6
            break;
        case COLOR_20BIT_LOOSE  :
        case COLOR_18BIT_CFG_2  :
            byte_per_chunk = 12;                                             // at least 3 lanes(12+6)=18: 3*8-18=6>=6
            break;
        case COLOR_16BIT_YCBCR :
        case COLOR_16BIT_CFG_1 :
        case COLOR_16BIT_CFG_2 :
        case COLOR_16BIT_CFG_3 :
            byte_per_chunk =  8;                                             // at least 3 lanes(8+6=14): 3*8-14=10>6
            break;
        case COLOR_30BIT :
            byte_per_chunk = 15;                                             // at least 4 lanes(15+6=21): 4*8-21=11>6
            break;
        case COLOR_36BIT :
            byte_per_chunk = 18;                                             // at least 4 leans(18+6=24): 4*8-24=8>6
            break;
        case COLOR_12BIT :
            byte_per_chunk =  6;                                             // at least 3 leans(6+6=12): 3*8-12=12>6
            break;
        case COLOR_18BIT_CFG_1 :
            byte_per_chunk =  9;                                             // at least 23lanes(9+6=15): 3*8-15=9>6
            break;
        case COLOR_RGB_111 :
        case COLOR_RGB_332 :
        case COLOR_RGB_444 :
            break;
        default :
            DPRINT(" Error: un-support data Format So Far, Please Add More\n");
            break;
    }    /*switch(dpi_data_format)*/
    num_of_chunk = p->lcd_basic.h_active / pixel_per_chunk;
    chunk_overhead = totol_bytes_per_chunk-(byte_per_chunk+6);                 // byte_per_chunk+6=valid_payload

    if(operation_mode == OPERATION_VIDEO_MODE && vid_mode_type != BURST_MODE) {
        if(chunk_overhead >= 6) {                                              // if room for null_vid's head(4)+crc(2)
            vid_null_size = chunk_overhead-6;                                  // chunk_overhead-null_vid's head(4)+crc(2) = null_vid's payload
        } else {
            DPRINT(" No room for null, chunk_overhead is %d\n", chunk_overhead);
        }
    }
    // -----------------------------------------------------
    // Standard Configuration for Video Mode Operation
    // -----------------------------------------------------
    // 1,    Configure Lane number and phy stop wait time
    if ((output_type != TV_ENC_LCD240x160_dsi) && (output_type != TV_ENC_LCD1920x1200p) &&
        (output_type != TV_ENC_LCD2560x1600) && (output_type != TV_ENC_LCD768x1024p)) {
        WRITE_LCD_REG( MIPI_DSI_DWC_PHY_IF_CFG_OS, (0x28 << BIT_PHY_STOP_WAIT_TIME) | (lane_num << BIT_N_LANES));
    } else {
        WRITE_LCD_REG( MIPI_DSI_DWC_PHY_IF_CFG_OS, (1 << BIT_PHY_STOP_WAIT_TIME) | (lane_num << BIT_N_LANES));
    }

    // 2.1,  Configure Virtual channel settings
    WRITE_LCD_REG( MIPI_DSI_DWC_DPI_VCID_OS, vcid );
    // 2.2,  Configure Color format
    WRITE_LCD_REG( MIPI_DSI_DWC_DPI_COLOR_CODING_OS, (((dpi_data_format == COLOR_18BIT_CFG_2) ? 1 : 0) << BIT_LOOSELY18_EN) | (dpi_data_format << BIT_DPI_COLOR_CODING) );
    // 2.2.1 Configure Set color format for DPI register
    WRITE_LCD_REG( MIPI_DSI_TOP_CNTL, ((READ_LCD_REG(MIPI_DSI_TOP_CNTL) & ~(0xf<<BIT_DPI_COLOR_MODE) & ~(0x7<<BIT_IN_COLOR_MODE) & ~(0x3<<BIT_CHROMA_SUBSAMPLE)) |
                                (dpi_data_format    << BIT_DPI_COLOR_MODE)  |
                                (venc_data_width    << BIT_IN_COLOR_MODE)   |
                                (chroma_subsample   << BIT_CHROMA_SUBSAMPLE)) );
    // 2.3   Configure Signal polarity
    WRITE_LCD_REG( MIPI_DSI_DWC_DPI_CFG_POL_OS, (0x0 << BIT_COLORM_ACTIVE_LOW) |
                        (0x0 << BIT_SHUTD_ACTIVE_LOW)  |
                        (0x0 << BIT_HSYNC_ACTIVE_LOW)  |
                        (0x0 << BIT_VSYNC_ACTIVE_LOW)  |
                        (0x0 << BIT_DATAEN_ACTIVE_LOW));

    if (operation_mode == OPERATION_VIDEO_MODE) {
        // 3.1   Configure Low power and video mode type settings
        WRITE_LCD_REG( MIPI_DSI_DWC_VID_MODE_CFG_OS, (1 << BIT_LP_HFP_EN)  |                  // enalbe lp
                        (1 << BIT_LP_HBP_EN)  |                  // enalbe lp
                        (1 << BIT_LP_VCAT_EN) |                  // enalbe lp
                        (1 << BIT_LP_VFP_EN)  |                  // enalbe lp
                        (1 << BIT_LP_VBP_EN)  |                  // enalbe lp
                        (1 << BIT_LP_VSA_EN)  |                  // enalbe lp
                        (1 << BIT_FRAME_BTA_ACK_EN) |            // enable BTA after one frame, TODO, need check
                        //(1 << BIT_LP_CMD_EN) |                   // enable the command transmission only in lp mode    //evoke add for test
                        (vid_mode_type << BIT_VID_MODE_TYPE) );  // burst/non burst

        // 3.2   Configure video packet size settings
        if( vid_mode_type == BURST_MODE ) {                                        // burst mode
            WRITE_LCD_REG( MIPI_DSI_DWC_VID_PKT_SIZE_OS, p->lcd_basic.h_active);                          // should be one line in pixels, such as 480/240...
        }
        else if(vid_mode_type == NON_BURST_SYNC_PULSE || vid_mode_type == NON_BURST_SYNC_EVENT) {                           // non-burst mode
            WRITE_LCD_REG( MIPI_DSI_DWC_VID_PKT_SIZE_OS, pixel_per_chunk);                    // in unit of pixels, (pclk period/byte clk period)*num_of_lane should be integer
            // in our system, 16/8*num_of_lane is integer, so 6 pixel should be enough for 24bpp
            // Worst case: (16/8)*8(pixel)*1(lane) >= 6(pkt head+crc)+3(max 24bpp)
        }

        // 3.3   Configure number of chunks and null packet size for one line
        if( vid_mode_type == BURST_MODE ) {                                        // burst mode
            WRITE_LCD_REG( MIPI_DSI_DWC_VID_NUM_CHUNKS_OS, 0);
            WRITE_LCD_REG( MIPI_DSI_DWC_VID_NULL_SIZE_OS, 0);
        }
        else {                                                                     // non burst mode
            WRITE_LCD_REG( MIPI_DSI_DWC_VID_NUM_CHUNKS_OS, num_of_chunk);                     // HACT/VID_PKT_SIZE
            WRITE_LCD_REG( MIPI_DSI_DWC_VID_NULL_SIZE_OS, vid_null_size);                     // video null size
            DPRINT(" ============== NON_BURST SETTINGS =============\n");
            DPRINT(" pixel_per_chunk       = %d\n", pixel_per_chunk);
            DPRINT(" num_of_chunk          = %d\n", num_of_chunk);
            DPRINT(" totol_bytes_per_chunk = %d\n", totol_bytes_per_chunk);
            DPRINT(" byte_per_chunk        = %d\n", byte_per_chunk);
            DPRINT(" chunk_overhead        = %d\n", chunk_overhead);
            DPRINT(" vid_null_size         = %d\n", vid_null_size);
            DPRINT(" ===============================================\n");
        }

        // 4     Configure the video relative parameters according to the output type
        //         include horizontal timing and vertical line
        dsi_video_config(p);
    }  /* operation_mode == OPERATION_VIDEO_MODE */

    // -----------------------------------------------------
    // Finish Configuration
    // -----------------------------------------------------

    // Inner clock divider settings
    WRITE_LCD_REG( MIPI_DSI_DWC_CLKMGR_CFG_OS, (0x1 << BIT_TO_CLK_DIV) | (0x1e << BIT_TX_ESC_CLK_DIV) );
    // Packet header settings
    WRITE_LCD_REG( MIPI_DSI_DWC_PCKHDL_CFG_OS, (1 << BIT_CRC_RX_EN) |
                        (1 << BIT_ECC_RX_EN) |
                        (0 << BIT_BTA_EN) |
                        (0 << BIT_EOTP_RX_EN) |
                        (0 << BIT_EOTP_TX_EN) );
    // operation mode setting: video/command mode
    WRITE_LCD_REG( MIPI_DSI_DWC_MODE_CFG_OS, operation_mode );

    // Phy Timer
    if ((output_type != TV_ENC_LCD240x160_dsi) &&
                    (output_type != TV_ENC_LCD1920x1200p) &&
                    (output_type != TV_ENC_LCD2560x1600) &&
                    (output_type != TV_ENC_LCD768x1024p)) {
        WRITE_LCD_REG( MIPI_DSI_DWC_PHY_TMR_CFG_OS, 0x03320000);
    } else {
        WRITE_LCD_REG( MIPI_DSI_DWC_PHY_TMR_CFG_OS, 0x090f0000);
    }

    // Configure DPHY Parameters
    if ((output_type != TV_ENC_LCD240x160_dsi) &&
                    (output_type != TV_ENC_LCD1920x1200p) &&
                    (output_type != TV_ENC_LCD2560x1600) &&
                    (output_type != TV_ENC_LCD768x1024p)) {
        WRITE_LCD_REG( MIPI_DSI_DWC_PHY_TMR_LPCLK_CFG_OS, 0x870025);
    } else {
        WRITE_LCD_REG( MIPI_DSI_DWC_PHY_TMR_LPCLK_CFG_OS, 0x260017);
    }
}

static void mipi_dsi_config(Lcd_Config_t *pConf)
{
    unsigned int        dpi_data_format;
    unsigned int        venc_data_width;
    tv_enc_lcd_type_t   venc_format;
    unsigned char       lane_num;
    unsigned char       operation_mode, video_mode_type;

    operation_mode  = pConf->lcd_control.mipi_config->operation_mode;
    video_mode_type = pConf->lcd_control.mipi_config->video_mode_type;
    venc_format     = pConf->lcd_control.mipi_config->venc_fmt;
    venc_data_width = pConf->lcd_control.mipi_config->venc_data_width;
    dpi_data_format = pConf->lcd_control.mipi_config->dpi_data_format;
    lane_num        = pConf->lcd_control.mipi_config->lane_num - 1;

    DPRINT("%s, %d\n", __func__, __LINE__);
#ifdef PRINT_DEBUG_INFO
    print_info(pConf->lcd_control.mipi_config);
#endif

    DPRINT("Set mipi_dsi_host\n");
    set_mipi_dcs(MIPI_DSI_CMD_TRANS_TYPE,            //0: high speed, 1: low power
                 MIPI_DSI_DCS_ACK_TYPE,              // if need bta ack check
                 MIPI_DSI_TEAR_SWITCH);                // enable tear ack

    DPRINT("%s, %d\n", __func__, __LINE__);
    set_mipi_dsi_host(lane_num,                        // Lane number
                      MIPI_DSI_VIRTUAL_CHAN_ID,        // Virtual channel id
                      venc_data_width,                 // MIPI dsi venc data bit width
                      dpi_data_format,                 // MIPI dsi dpi data format
                      0,                               // Chroma sub sample, only for YUV 422 or 420, even or odd
                      operation_mode,                  // DSI operation mode, video or command
                      venc_format,                     // Venc resolution format, eg, 240x160
                      video_mode_type,                 // Video mode, burst or non-burst
                      1,                               // If check the phy status, need check when first pic
                      pConf);
}

void mipi_dsi_link_on(Lcd_Config_t *pConf)
{
    int vcid = MIPI_DSI_VIRTUAL_CHAN_ID;
    int req_ack = MIPI_DSI_DCS_ACK_TYPE;
    unsigned int        dpi_data_format;
    unsigned int        venc_data_width;
    tv_enc_lcd_type_t   venc_format;
    unsigned char       lane_num;
    unsigned char       operation_mode, video_mode_type;
    int i=0, j=0, ending_flag=0;
#ifdef CONFIG_AML_LCD_EXTERN
    struct aml_lcd_extern_driver_t *lcd_extern_driver;
#endif

    printf("%s\n", __FUNCTION__);
    operation_mode  = OPERATION_VIDEO_MODE;//pConf->lcd_control.mipi_config->operation_mode;
    video_mode_type = pConf->lcd_control.mipi_config->video_mode_type;
    venc_format     = pConf->lcd_control.mipi_config->venc_fmt;
    venc_data_width = pConf->lcd_control.mipi_config->venc_data_width;
    dpi_data_format = pConf->lcd_control.mipi_config->dpi_data_format;
    lane_num        = pConf->lcd_control.mipi_config->lane_num - 1;

    if(pConf->lcd_control.mipi_config->transfer_ctrl == 0)
        startup_transfer_video();
    else
        startup_transfer_cmd();

    if (pConf->lcd_control.mipi_config->init_on_flag ==1) {
#ifdef CONFIG_AML_LCD_EXTERN
        lcd_extern_driver = aml_lcd_extern_get_driver();
        if (lcd_extern_driver == NULL) {
            printf("no lcd_extern driver\n");
        }
        else {
            if (lcd_extern_driver->init_on_cmd_8) {
                while(ending_flag == 0) {
                    if(lcd_extern_driver->init_on_cmd_8[i]==0xff) {
                        j = 2;
                        if(lcd_extern_driver->init_on_cmd_8[i+1]==0xff)
                            ending_flag = 1;
                        else
                            mdelay(lcd_extern_driver->init_on_cmd_8[i+1]);
                    }
                    else {
                        j = lcd_extern_driver->init_on_cmd_8[i];
                        if (j == 1) {//no parameter
                            DCS_write_short_packet_0_para(DT_DCS_SHORT_WR_0, vcid, lcd_extern_driver->init_on_cmd_8[i+1], req_ack);
                        }
                        else if (j == 2) {//1 parameter
                            DCS_write_short_packet_1_para(DT_DCS_SHORT_WR_1, vcid, lcd_extern_driver->init_on_cmd_8[i+1], lcd_extern_driver->init_on_cmd_8[i+2], req_ack);
                        }
                        else {//long write
                            DCS_long_write_packet(DT_DCS_LONG_WR, vcid, lcd_extern_driver->init_on_cmd_8[i+1], &lcd_extern_driver->init_on_cmd_8[i+1], j, req_ack);
                        }
                        j++;
                    }
                    i += j;
                }
                printf("%s power on init\n", lcd_extern_driver->name);
            }
        }
#endif
    }

    DPRINT("send sleep out\n");
    DCS_write_short_packet_0_para(DT_DCS_SHORT_WR_0, vcid, DCS_EXIT_SLEEP_MODE, req_ack);
    mdelay(pConf->lcd_control.mipi_config->sleep_out_delay);

    DPRINT("send display on\n");
    DCS_write_short_packet_0_para(DT_DCS_SHORT_WR_0, vcid, DCS_SET_DISPLAY_ON, req_ack);
    mdelay(pConf->lcd_control.mipi_config->display_on_delay);

    if (operation_mode != pConf->lcd_control.mipi_config->operation_mode) {
        set_mipi_dsi_host(lane_num,                        // Lane number
                      MIPI_DSI_VIRTUAL_CHAN_ID,        // Virtual channel id
                      venc_data_width,                 // MIPI dsi venc RGB data bit width
                      dpi_data_format,                 // MIPI dsi dpi data format
                      0,                               // Chroma sub sample, only for YUV 422 or 420, even or odd
                      operation_mode,                  // DSI operation mode, video or command
                      venc_format,                     // Venc resolution format, eg, 240x160
                      video_mode_type,                 // Video mode, burst or non-burst
                      1,                               // If check the phy status, need check when first pic
                      pConf);
    }
}

void mipi_dsi_link_off(Lcd_Config_t *pConf)
{
    int vcid = MIPI_DSI_VIRTUAL_CHAN_ID;
    int req_ack = 0;
    int i=0, j=0, ending_flag=0;
#ifdef CONFIG_AML_LCD_EXTERN
    struct aml_lcd_extern_driver_t *lcd_extern_driver;
#endif

    printf("%s\n", __FUNCTION__);
    DPRINT("send display off\n");
    DCS_write_short_packet_0_para(DT_DCS_SHORT_WR_0, vcid, DCS_SET_DISPLAY_OFF, req_ack);
    mdelay(10);

    DPRINT("send enter sleep mode\n");
    DCS_write_short_packet_0_para(DT_DCS_SHORT_WR_0, vcid, DCS_ENTER_SLEEP_MODE, req_ack);
    mdelay(20);

    if (pConf->lcd_control.mipi_config->init_off_flag ==1) {
#ifdef CONFIG_AML_LCD_EXTERN
        lcd_extern_driver = aml_lcd_extern_get_driver();
        if (lcd_extern_driver == NULL) {
            printf("no lcd_extern driver\n");
        }
        else {
            if (lcd_extern_driver->init_off_cmd_8) {
                while(ending_flag == 0) {
                    if(lcd_extern_driver->init_off_cmd_8[i]==0xff) {
                        j = 2;
                        if(lcd_extern_driver->init_off_cmd_8[i+1]==0xff)
                            ending_flag = 1;
                        else
                            mdelay(lcd_extern_driver->init_off_cmd_8[i+1]);
                    }
                    else {
                        j = lcd_extern_driver->init_off_cmd_8[i];
                        if (j == 1) {//no parameter
                            DCS_write_short_packet_0_para(DT_DCS_SHORT_WR_0, vcid, lcd_extern_driver->init_off_cmd_8[i+1], req_ack);
                        }
                        else if (j == 2) {//1 parameter
                            DCS_write_short_packet_1_para(DT_DCS_SHORT_WR_1, vcid, lcd_extern_driver->init_off_cmd_8[i+1], lcd_extern_driver->init_off_cmd_8[i+2], req_ack);
                        }
                        else {//long write
                            DCS_long_write_packet(DT_DCS_LONG_WR, vcid, lcd_extern_driver->init_off_cmd_8[i+1], &lcd_extern_driver->init_off_cmd_8[i+1], j, req_ack);
                        }
                        j++;
                    }
                    i += j;
                }
                printf("%s power off init\n", lcd_extern_driver->name);
            }
        }
#endif
    }
}

void set_mipi_dsi_control(Lcd_Config_t *pConf)
{
    DSI_Config_t *cfg= pConf->lcd_control.mipi_config;

    cfg->video_mode_type = MIPI_DSI_VIDEO_MODE_TYPE;
    if(pConf->lcd_basic.lcd_bits == 6){
        cfg->dpi_data_format = COLOR_18BIT_CFG_2;
        cfg->venc_data_width = MIPI_DSI_VENC_COLOR_18B;
    }else{
        cfg->dpi_data_format  = COLOR_24BIT;
        cfg->venc_data_width = MIPI_DSI_VENC_COLOR_24B;
    }

    if((pConf->lcd_basic.h_active !=240)&&(pConf->lcd_basic.h_active !=768)&&(pConf->lcd_basic.h_active !=1920)&&(pConf->lcd_basic.h_active !=2560))
        cfg->venc_fmt=TV_ENC_LCD1280x720;
    else
        cfg->venc_fmt=TV_ENC_LCD768x1024p;

    if (cfg->factor_numerator == 0) {
        pConf->lcd_control.mipi_config->factor_denominator = pConf->lcd_control.mipi_config->bit_rate / 8 / 1000;
        pConf->lcd_control.mipi_config->factor_numerator = pConf->lcd_timing.lcd_clk / 1000;
    }
    cfg->hline =(pConf->lcd_basic.h_period * cfg->factor_denominator + cfg->factor_numerator - 1) / cfg->factor_numerator;  // Rounded. Applicable for Period(pixclk)/Period(bytelaneclk)=9/16
    cfg->hsa =(pConf->lcd_timing.hsync_width * cfg->factor_denominator + cfg->factor_numerator - 1) / cfg->factor_numerator;
    cfg->hbp =((pConf->lcd_timing.hsync_bp-pConf->lcd_timing.hsync_width) * cfg->factor_denominator + cfg->factor_numerator - 1) / cfg->factor_numerator;

    cfg->vsa = pConf->lcd_timing.vsync_width;
    cfg->vbp = pConf->lcd_timing.vsync_bp - pConf->lcd_timing.vsync_width;
    cfg->vfp = pConf->lcd_basic.v_period - pConf->lcd_timing.vsync_bp - pConf->lcd_basic.v_active;
    cfg->vact = pConf->lcd_basic.v_active;

    dsi_phy_config(pConf);

    mipi_dsi_config(pConf);
}

void mipi_dsi_off(void)
{
    DPRINT("poweroff dsi digital\n");
    // Power down DSI
    WRITE_LCD_REG(MIPI_DSI_DWC_PWR_UP_OS, 0);

    // Power down D-PHY, do not have to close dphy
    // WRITE_LCD_REG(MIPI_DSI_DWC_PHY_RSTZ_OS, (READ_LCD_REG( MIPI_DSI_DWC_PHY_RSTZ_OS ) & 0xc));
    // WRITE_LCD_REG(MIPI_DSI_DWC_PHY_RSTZ_OS, 0xc);

    WRITE_DSI_REG(MIPI_DSI_CHAN_CTRL, 0x1f);
    DPRINT("MIPI_DSI_PHY_CTRL=0x%x\n", READ_DSI_REG(MIPI_DSI_PHY_CTRL)); //read
    WRITE_DSI_REG_BITS(MIPI_DSI_PHY_CTRL, 0, 7, 1);
}
