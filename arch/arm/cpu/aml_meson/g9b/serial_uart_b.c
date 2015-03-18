
#include <config.h>
#include <asm/arch/cpu.h>
#include <asm/arch/uart.h>
#include <asm/arch/io.h>
#include <common.h>

const char* UPGRADE_FBC_REQUEST_CMD = "reboot -r upgrade\n";
const char* UPGRADE_FBC_REBOOT = "reboot\n";
char UPGRADE_FBC_REQUEST_HDMI_CMD[64] = {0x5a,0x5a,0xe,0x0,0x0,0x1,0x88,0x88,0x88,0x88,0x20,0x43,0xd8,0x55};
#define UPGRADE_FBC_REQUEST_CNT 3
#define FBC_BOOT_MAIN_ADDR_INDEX    2
#define SPI_BIN_MEM_ADDR    0x12000000
#define CONFIG_UART_B_CMD_LEN   64
#define CONFIG_UART_B_FIFO_LEN  64
#define FBC_RESPONSE_ARRAY_LEN  1024
int is_uart_b_init = 0;

/*
 *tip : uart_b(uart1) TFIFO/RFIFO is 64 Bytes
 */

void serial_init_uart_b(void)
{
    unsigned uart_b_control = (clk_get_rate(UART_CLK_SRC)/(115200*4) -1)
        | UART_STP_BIT
        | UART_PRTY_BIT
        | UART_CHAR_LEN
        | UART_CNTL_MASK_TX_EN
        | UART_CNTL_MASK_RX_EN
        | UART_CNTL_MASK_RST_TX
        | UART_CNTL_MASK_RST_RX
        | UART_CNTL_MASK_CLR_ERR ;
    printf("serial uart b init success\n");
    CLEAR_CBUS_REG_MASK(0x202d,0x3000000);  //disable i2c
    writel(uart_b_control,CBUS_REG_ADDR(UART_PORT_1+UART_CONTROL));
    SET_CBUS_REG_MASK(0x2030,0x300);
    CLEAR_CBUS_REG_MASK(0x202f,0x4000000);
    CLEAR_CBUS_REG_MASK(0x2033,0x30080000);
    clrbits_le32(CBUS_REG_ADDR(UART_PORT_1+UART_CONTROL),
        UART_CNTL_MASK_RST_TX | UART_CNTL_MASK_RST_RX | UART_CNTL_MASK_CLR_ERR);
}

void serial_putc_uart_b(const char c)
{
    while ((readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS )) & UART_STAT_MASK_TFIFO_FULL));   /* Wait till dataTx register is not full */
    writel(c, CBUS_REG_ADDR(UART_PORT_1+UART_WFIFO));   //write a byte
}

char serial_getc_uart_b(void)
{
    char ch;
    /* Wait till RFIFO have data*/
    while(0 == (readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & (UART_STAT_MASK_RFIFO_CNT | UART_STAT_MASK_RFIFO_FULL)));

    /* check frame error and parity error*/
    if (readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & (UART_STAT_MASK_PRTY_ERR | UART_STAT_MASK_FRAM_ERR))
    {
        setbits_le32(CBUS_REG_ADDR(UART_PORT_1+UART_CONTROL),UART_CNTL_MASK_CLR_ERR);
        clrbits_le32(CBUS_REG_ADDR(UART_PORT_1+UART_CONTROL),UART_CNTL_MASK_CLR_ERR);
    }

    ch = (char)(readl(CBUS_REG_ADDR(UART_PORT_1+UART_RFIFO)) & 0x00ff);
    return ch;
}

void serial_puts_uart_b(const char *s)
{
    while (*s) {
        serial_putc_uart_b(*s++);
    }
    while ((readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_TFIFO_EMPTY)==0);    //till TFIFO empty
}

void msdelay(int ms)
{
    int i;
    for(i=0;i<ms;i++)
        udelay(1000);
}

void serial_write_bytes_uart_b(char *buf, int nbyte)
{
    int count;
    for(count=0;count<nbyte;count++) {
        if(count % CONFIG_UART_B_FIFO_LEN == 0)
        {
            msdelay(10);
        }
        serial_putc_uart_b(buf[count]);
    }
}

void serial_read_bytes_uart_b(char *buf, int nbyte)
{
    int count;
    for(count=0;count<nbyte;count++) {
        buf[count] = serial_getc_uart_b();
    }
}

void serial_send_cmd_uart_b(const char *buf)
{
    char cmd_array[CONFIG_UART_B_CMD_LEN];
    memset(cmd_array,0,CONFIG_UART_B_CMD_LEN);
    strcpy(cmd_array,buf);
    serial_write_bytes_uart_b(cmd_array,strlen(buf));
}

void serial_clear_error_uart_b(void)
{
    setbits_le32(CBUS_REG_ADDR(UART_PORT_1+UART_CONTROL),UART_CNTL_MASK_CLR_ERR);
    clrbits_le32(CBUS_REG_ADDR(UART_PORT_1+UART_CONTROL),UART_CNTL_MASK_CLR_ERR);
}

//burn fbc function and cmd
void print_uart_b_status(void)
{
    printf("\nuart_b_status = 0x%lx\n", readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)));
    printf("uart_b_status:\n");
    printf("\tR busy             (26 bit) : %ld\n", (readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_RECV_BUSY)>>26);
    printf("\tT busy             (25 bit) : %ld\n", (readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_XMIT_BUSY)>>25);
    printf("\tRECV_FIFO_OVERFLOW (24 bit) : %ld\n", (readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_RECV_FIFO_OF)>>24);
    printf("\tCTS Level          (23 bit) : %ld\n", (readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_CTS_LEVEL)>>23);
    printf("\tT FIFO Empty       (22 bit) : %ld\n", (readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_TFIFO_EMPTY)>>22);
    printf("\tT FIFO Full        (21 bit) : %ld\n", (readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_TFIFO_FULL)>>21);
    printf("\tR FIFO Empty       (20 bit) : %ld\n", (readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_RFIFO_EMPTY)>>20);
    printf("\tR FIFO Full        (19 bit) : %ld\n", (readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_RFIFO_FULL)>>19);
    printf("\tFIFO is written    (18 bit) : %ld\n", (readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_WFULL_ERR)>>18);
    printf("\tFrame error        (17 bit) : %ld\n", (readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_FRAM_ERR)>>17);
    printf("\tParity error       (16 bit) : %ld\n", (readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_PRTY_ERR)>>16);
    printf("\tT FIFO count       (14~8 bit) : %ld\n", (readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_TFIFO_CNT)>>8);
    printf("\tR FIFO count       (6~0  bit) : %ld\n\n", (readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_RFIFO_CNT));
}

unsigned int datacrc32(unsigned int crc, const unsigned char *ptr, unsigned int buf_len)
{
    static const unsigned int s_crc32[16] ={
        0, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4,
        0x4db26158, 0x5005713c,0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c };
    unsigned int crcu32 = crc;
    if (!ptr)
    {
        return 0;
    }
    crcu32 = ~crcu32;
    while(buf_len--)
    {
        unsigned char b = *ptr++;
        crcu32 = (crcu32 >> 4) ^ s_crc32[(crcu32 & 0xF) ^ (b & 0xF)];
        crcu32 = (crcu32 >> 4) ^ s_crc32[(crcu32 & 0xF) ^ (b >> 4)];
    }
    return ~crcu32;
}

void print_rfifo(void)
{
    unsigned long i;
    unsigned long j = (readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_RFIFO_CNT);
    for(i=0;i<j;i++) {
        printf("%c", serial_getc_uart_b());
    }
}

static int do_uart_b(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    if(argc < 2) return cmd_usage(cmdtp);

    if(!strcmp("init", argv[1]))
    {
        serial_init_uart_b();
        is_uart_b_init = 1;
    }

    if(!strcmp("fbc", argv[1]))
    {
        int count;
        int fbc_burn_addr_begin_index = 0;
        int fbc_burn_addr_end_index = 0;
        char getc_temp;

        unsigned long data_addr_base = SPI_BIN_MEM_ADDR;
        unsigned long data_addr = 0;
        unsigned char data_buf[0x10000] = {0};  //64K
        unsigned long file_len = 0;
        unsigned long upgrade_file_64K_cnt = 0;

        unsigned int crc;
        char crcbuf[4] = {0};

        char addr_hex[10];
        char upgrade_fbc_cmd[CONFIG_UART_B_CMD_LEN] = {0};
        char* UPGRADE_FBC_CMD = "";
        char fbc_response[FBC_RESPONSE_ARRAY_LEN] = {0};
        int fbc_response_count = 0;

        int upgrade_flag = 0;

        //clear RFIFO before burn fbc
        while((readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & (UART_STAT_MASK_RFIFO_CNT | UART_STAT_MASK_RFIFO_FULL)))
        {
            getc_temp = serial_getc_uart_b();
        }
        serial_clear_error_uart_b();    //clear error

        if(!is_uart_b_init)
        {
            printf("Please run 'uart init' command to init uart b\n");
            goto g9_upgrade_fbc_end;
        }

        //load fbc upgrade file
        char *FILESIZE = getenv("filesize");
        if(FILESIZE == NULL)
        {
            printf("Please fatload the spi.bin file at memory 0x%lx\n", data_addr_base);
            goto g9_upgrade_fbc_end;
        }
        else
        {
            file_len = simple_strtoul(FILESIZE, NULL, 16);
            printf("file len = 0x%lx\n", file_len);
            if(file_len%(64*1024) != 0)
            {
                printf("Error : spi.bin file size must be 64K bytes integer times\n");
                goto g9_upgrade_fbc_end;
            }
            else
            {
                upgrade_file_64K_cnt = file_len/(64*1024);
                printf("upgrade file have %ld*64K data\n", upgrade_file_64K_cnt);
            }
        }

        //config the position of fbc burn
        if(2 == argc)
        {
            fbc_burn_addr_begin_index = FBC_BOOT_MAIN_ADDR_INDEX;
            fbc_burn_addr_end_index = upgrade_file_64K_cnt;
        }
        else if(3 == argc)
        {
            if(!strcmp("main", argv[2]))
            {
                fbc_burn_addr_begin_index = FBC_BOOT_MAIN_ADDR_INDEX;
                fbc_burn_addr_end_index = upgrade_file_64K_cnt;
            }
            else if(!strcmp("boot", argv[2]))
            {
                fbc_burn_addr_begin_index = 0;
                fbc_burn_addr_end_index = FBC_BOOT_MAIN_ADDR_INDEX;
            }
            else if(!strcmp("all", argv[2]))
            {
                fbc_burn_addr_begin_index = 0;
                fbc_burn_addr_end_index = upgrade_file_64K_cnt;
            }
            else
            {
                printf("parameter error!!!\n");
                goto g9_upgrade_fbc_end;
            }
        }

        //G9 send upgrade fbc request
        for(count=1;count<=UPGRADE_FBC_REQUEST_CNT;count++)
        {
            //serial_send_cmd_uart_b(UPGRADE_FBC_REQUEST_CMD);
            serial_write_bytes_uart_b(UPGRADE_FBC_REQUEST_HDMI_CMD,14);
            msdelay(1000);
            if(0!=(readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_RFIFO_CNT))
            {
                printf("success send request cmd to fbc for %d time\n\n", count);
                while((readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & (UART_STAT_MASK_RFIFO_CNT | UART_STAT_MASK_RFIFO_FULL)))
                {
                    getc_temp = serial_getc_uart_b();   //read the data at RFIFO
                }
                serial_clear_error_uart_b();    //clear error
                upgrade_flag = 1;
                break;
            }
            else if(  (0==((readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_RECV_FIFO_OF)>>24))
                    & (0==((readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_RFIFO_FULL)>>19))
                    & (1==((readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_RFIFO_EMPTY)>>20))  )
            {
                printf("serial does not connect\n");
                if(count == UPGRADE_FBC_REQUEST_CNT)
                {
                    upgrade_flag = 0;
                }
            }
        }

        //upgrade fbc
        while(upgrade_flag)
        {
            if(fbc_burn_addr_begin_index < fbc_burn_addr_end_index)
            {
                msdelay(400);

                //get data
                memset(data_buf,0,0x10000);
                data_addr = data_addr_base + fbc_burn_addr_begin_index * 0x10000;
                printf("get 0x10000 bytes from memory 0x%lx\n", data_addr);
                for(count=0;count<0x10000;count++)
                {
                    data_buf[count] = *((char *)data_addr);
                    data_addr++;
                }

                //send upgrade fbc cmd
                memset(upgrade_fbc_cmd,0,CONFIG_UART_B_CMD_LEN);
                strcat(upgrade_fbc_cmd,"upgrade 0x");
                sprintf(addr_hex,"%x",fbc_burn_addr_begin_index*0x10000);
                strcat(upgrade_fbc_cmd,addr_hex);
                strcat(upgrade_fbc_cmd," 0x10000\n");
                strcpy(UPGRADE_FBC_CMD,upgrade_fbc_cmd);
                serial_send_cmd_uart_b(UPGRADE_FBC_CMD);
                msdelay(200);
                print_rfifo();

                //send upgrade fbc data
                printf("send upgrade fbc data...\n");
                for(count=0;count<0x10000;count++)
                {
                    if(count%64 == 0) {
                        msdelay(10);
                    }
                    if(count%256 == 0)
                    {
                        print_rfifo();
                    }
                    serial_putc_uart_b(data_buf[count]);
                }
                printf("%c\n", serial_getc_uart_b());
                msdelay(50);
                print_rfifo();

                //send crc and get fbc response
                printf("send crc...\n");
                crc = datacrc32(0,data_buf,0x10000);
                crcbuf[0] = crc & 0xFF;
                crcbuf[1] = (crc >> 8) & 0xFF;
                crcbuf[2] = (crc >> 16) & 0xFF;
                crcbuf[3] = (crc >> 24) & 0xFF;
                printf("waiting fbc response...\n");
                count = 0;
                fbc_response_count = 0;
                memset(fbc_response,0,FBC_RESPONSE_ARRAY_LEN);
                serial_write_bytes_uart_b(crcbuf,4);
                while(count<1000000)
                {
                    while((readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_RFIFO_CNT))
                    {
                        fbc_response[fbc_response_count] = serial_getc_uart_b();
                        fbc_response_count++;
                        if(FBC_RESPONSE_ARRAY_LEN == fbc_response_count)
                        {
                            printf("fbc response array overflow\n");
                            break;
                        }
                    }
                    count++;
                }
                printf("%s", fbc_response);

                //deal with fbc response
                for(count=0;count<fbc_response_count-3;count++)
                {
                    if((0x5A == fbc_response[count])&&(0x5A == fbc_response[count+1])&&(0x5A == fbc_response[count+2]))
                    //if((0x5A == fbc_response[count])&&(0x5A == fbc_response[count+1])&&(0x5A == fbc_response[count+2])&&(0x5A == fbc_response[count+3]))
                    {
                        printf("fbc write data at 0x%x ok!\n\n", fbc_burn_addr_begin_index * 0x10000);
                        fbc_burn_addr_begin_index++;
                    }
                    else if((0xA5 == fbc_response[count])&&(0xA5 == fbc_response[count+1])&&(0xA5 == fbc_response[count+2]))
                    //else if((0xA5 == fbc_response[count])&&(0xA5 == fbc_response[count+1])&&(0xA5 == fbc_response[count+2])&&(0xA5 == fbc_response[count+3]))
                    {
                        printf("fbc write data at 0x%x error! rewrite!\n\n", fbc_burn_addr_begin_index * 0x10000);
                    }
                }
            }
            else
            {
                break;
            }
        }

        if(1 == upgrade_flag) {
            if(2 == argc)
            {
                printf("upgrade fbc [main] success\n");
            }
            else if(3 == argc)
            {
                printf("upgrade fbc [%s] success\n", argv[2]);
            }
            printf("now reboot fbc...\n");
            serial_send_cmd_uart_b(UPGRADE_FBC_REBOOT);
            msdelay(6000);
            while((readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & (UART_STAT_MASK_RFIFO_CNT | UART_STAT_MASK_RFIFO_FULL)))
            {
                getc_temp = serial_getc_uart_b();
            }
            serial_clear_error_uart_b();
        }
        else {
            printf("please check hardware link\n");
        }

        g9_upgrade_fbc_end: ;
    }

    if(!strcmp("w", argv[1]))
    {
        char* cmd = "";
        char cmd_temp[CONFIG_UART_B_CMD_LEN] = {0};
        if(argc == 3)
        {
            if(strlen(argv[2])>63)
            {
                printf("command too long\n");
            }
            else
            {
                memset(cmd_temp,0,CONFIG_UART_B_CMD_LEN);
                strcpy(cmd_temp,argv[2]);
                strcat(cmd_temp,"\n");
                strcpy(cmd,cmd_temp);
                serial_send_cmd_uart_b(cmd);
                if(!strcmp("reboot -r upgrade", argv[2]))
                {
                    msdelay(1000);
                }
                else if(!strcmp("reboot", argv[2]))
                {
                    msdelay(6000);
                }
                else
                {
                    msdelay(500);
                }
            }
        }
        else
        {
            return cmd_usage(cmdtp);
        }
    }

    if(!strcmp("r", argv[1]))
    {
        if(2 == argc)
        {
            if(0 != (readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_RFIFO_CNT)) {
                printf("char = %c\n", serial_getc_uart_b());
            }
        }
        else if(3 == argc)
        {
            unsigned long count;
            unsigned long read_cnt = (unsigned long)simple_strtoul(argv[2],NULL,10);
            unsigned long rfifo_cnt = (readl(CBUS_REG_ADDR(UART_PORT_1+UART_STATUS)) & UART_STAT_MASK_RFIFO_CNT);
            if(read_cnt<=rfifo_cnt) {
                for(count=0;count<read_cnt;count++) {
                    printf("%c", serial_getc_uart_b());
                }
            }
            else {
                for(count=0;count<rfifo_cnt;count++) {
                    printf("%c", serial_getc_uart_b());
                }
            }
            printf("\n");
        }
    }

    if(!strcmp("c", argv[1]))
    {
        serial_clear_error_uart_b();
    }

    if(!strcmp("p", argv[1]))
    {
        print_uart_b_status();
    }
    return 0;
}

U_BOOT_CMD(
    uart,3,1,do_uart_b,
    "uart_b burn fbc & debug",
    "init : init uart b\n"
    "uart w cmd : send cmd to fbc\n"
    "uart r [count] : read data from fbc\n"
    "uart fbc : burn fbc main code\n"
    "uart fbc main : burn fbc main code\n"
    "uart fbc boot : burn fbc boot code\n"
    "uart fbc all  : burn fbc all code\n"
);
