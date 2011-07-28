#include <config.h>
#include <asm/arch/io.h>
#include <asm/arch/sdio.h>

static struct aml_card_sd_info m3_sdio_ports[]={
    { .sdio_port=SDIO_PORT_A,.name="SDIO Port A"},
    { .sdio_port=SDIO_PORT_B,.name="SDIO Port B"},
    { .sdio_port=SDIO_PORT_C,.name="SDIO Port C"},
};
struct aml_card_sd_info * cpu_sdio_get(unsigned port)
{
    if(port<SDIO_PORT_C+1)
        return &m3_sdio_ports[port];
    return NULL;
}

void  cpu_sdio_pwr_prepare(unsigned port)
{
    switch(port)
    {
        case SDIO_PORT_A:
            clrbits_le32(P_PREG_PAD_GPIO4_EN_N,0x30f);
            clrbits_le32(P_PREG_PAD_GPIO4_O   ,0x30f);
            clrbits_le32(P_PERIPHS_PIN_MUX_8,0x3f);
            break;
        case SDIO_PORT_B:
            clrbits_le32(P_PREG_PAD_GPIO5_EN_N,0x3f<<23);
            clrbits_le32(P_PREG_PAD_GPIO5_O   ,0x3f<<23);
            clrbits_le32(P_PERIPHS_PIN_MUX_2,0x3f<<10);
            break;
        case SDIO_PORT_C:
            clrbits_le32(P_PREG_PAD_GPIO3_EN_N,0xc0f);
            clrbits_le32(P_PREG_PAD_GPIO3_O   ,0xc0f);
            clrbits_le32(P_PERIPHS_PIN_MUX_6,(0x3f<<24));
			break;
    }
    
    /**
        do nothing here
    */
}
int cpu_sdio_init(unsigned port)
{
    switch(port)
    {
        case SDIO_PORT_A:
           setbits_le32(P_PERIPHS_PIN_MUX_8,0x3f);
			     break;
        case SDIO_PORT_B:
			     setbits_le32(P_PERIPHS_PIN_MUX_2,0x3f<<10);
			     break;
        case SDIO_PORT_C:
			     //clear NAND pinmux
           clrbits_le32(P_PERIPHS_PIN_MUX_2,(0x1f<<22));
			     //set sdio c pinmux
           setbits_le32(P_PERIPHS_PIN_MUX_6,(0x3f<<24));
           break;
        default:
           return -1;
    }
    return 0;
}
