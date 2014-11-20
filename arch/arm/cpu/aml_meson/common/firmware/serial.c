/*******************************************************************
 *
 *  Copyright C 2010 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: Serial driver.
 *
 *  Author: Jerry Yu
 *  Created: 2009-3-12
 *
 *******************************************************************/

#include <config.h>
#include <asm/arch/cpu.h>
#include <asm/arch/uart.h>
#include <asm/arch/io.h>

int serial_set_pin_port(unsigned port_base);

SPL_STATIC_FUNC void serial_init(unsigned set)
{
    /* baud rate */
	writel(set
	    |UART_CNTL_MASK_RST_TX
	    |UART_CNTL_MASK_RST_RX
	    |UART_CNTL_MASK_CLR_ERR
	    |UART_CNTL_MASK_TX_EN
	    |UART_CNTL_MASK_RX_EN
	,P_UART_CONTROL(UART_PORT_CONS));
#ifndef CONFIG_M3
	switch(readl(0xd9040004))
	{
	case 0x74e: //M8B revA
	case 0xb72: //M8M2 revA
		writel(	(1<<24)|(1 << 23)|((8000000/(CONFIG_BAUDRATE)) -1),0xc81004d4); // set xtal as M8B/M8M2 uart-ao clock source,xtal is 24000000,
	    //new baudrate formula is (24000000/3/baudrate),0xc81004d4 is AO_UART0_REG5 address
		break;
	}
#endif
    serial_set_pin_port(UART_PORT_CONS);
    clrbits_le32(P_UART_CONTROL(UART_PORT_CONS),
	    UART_CNTL_MASK_RST_TX | UART_CNTL_MASK_RST_RX | UART_CNTL_MASK_CLR_ERR);

}
//SPL_STATIC_FUNC
void serial_putc(const char c)
{
    if (c == '\n')
    {
        while ((readl(P_UART_STATUS(UART_PORT_CONS)) & UART_STAT_MASK_TFIFO_FULL));
        writel('\r', P_UART_WFIFO(UART_PORT_CONS));
    }
    /* Wait till dataTx register is not full */
    while ((readl(P_UART_STATUS(UART_PORT_CONS)) & UART_STAT_MASK_TFIFO_FULL));
    writel(c, P_UART_WFIFO(UART_PORT_CONS));
    /* Wait till dataTx register is empty */
}

//SPL_STATIC_FUNC
void serial_wait_tx_empty(void)
{
    while ((readl(P_UART_STATUS(UART_PORT_CONS)) & UART_STAT_MASK_TFIFO_EMPTY)==0);

}
/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise 0.
 */
SPL_STATIC_FUNC
int serial_tstc(void)
{
	return (readl(P_UART_STATUS(UART_PORT_CONS)) & UART_STAT_MASK_RFIFO_CNT);

}

/*
 * Read a single byte from the serial port.
 */
SPL_STATIC_FUNC
int serial_getc(void)
{
    unsigned char ch;
    /* Wait till character is placed in fifo */
  	while((readl(P_UART_STATUS(UART_PORT_CONS)) & UART_STAT_MASK_RFIFO_CNT)==0) ;

    /* Also check for overflow errors */
    if (readl(P_UART_STATUS(UART_PORT_CONS)) & (UART_STAT_MASK_PRTY_ERR | UART_STAT_MASK_FRAM_ERR))
	{
	    setbits_le32(P_UART_CONTROL(UART_PORT_CONS),UART_CNTL_MASK_CLR_ERR);
	    clrbits_le32(P_UART_CONTROL(UART_PORT_CONS),UART_CNTL_MASK_CLR_ERR);
	}

    ch = readl(P_UART_RFIFO(UART_PORT_CONS)) & 0x00ff;
    return ((int)ch);

}

#ifdef TEST_UBOOT_BOOT_SPEND_TIME
static int sec = 0; 
SPL_STATIC_FUNC void serial_put_dec(unsigned int data);
void print_timestamp(void)
{
	unsigned int time = get_utimer(0);
	unsigned int sec_p, usec_p;
	unsigned int scale;

	while (time >= (1000000 * (sec + 1))) {
		sec++;	
	}
	sec_p = sec;
	usec_p = time - (sec * 1000000);
	serial_putc('[');
	scale = 10000;
	while (scale > sec_p) {
		if (!sec_p && (scale == 1)) {
			break;	
		}
        serial_putc(' ');
		scale /= 10;
	}
	serial_put_dec(sec_p);
	serial_putc('.');
	scale = 100000;
	while (scale > usec_p) {
		if (!usec_p && (scale == 1)) {
			break;	
		}
        serial_putc('0');
		scale /= 10;
	}
	serial_put_dec(usec_p);
	serial_putc(']');
	serial_putc(' ');
	serial_wait_tx_empty();
}
#endif

//SPL_STATIC_FUNC
void serial_puts(const char *s)
{
#ifdef TEST_UBOOT_BOOT_SPEND_TIME
	static int newline = 1, first = 0;
	if (!first) {
		while (*s++ == '\n');			// escape redundant '\n'
		serial_putc('\n');
		first = 1;
	}
	if (newline) {
		print_timestamp();
	}
#endif
    while (*s) {
        serial_putc(*s);
	#ifdef TEST_UBOOT_BOOT_SPEND_TIME
		if (*s == '\n' && s[1]) {		// not end
			print_timestamp();
		}
	#endif
		s++;
    }
#ifdef TEST_UBOOT_BOOT_SPEND_TIME
	newline = (('\n' == (*(s-1))) ? 1:0);
#endif

	serial_wait_tx_empty();
}
//SPL_STATIC_FUNC
void serial_put_hex(unsigned int data,unsigned bitlen)
{
	int i;
    for (i=bitlen-4;i>=0;i-=4){
        if((data>>i)==0)
        {
            serial_putc(0x30);
            continue;
        }

        unsigned char s = (data>>i)&0xf;
        if (s<10)
            serial_putc(0x30+s);
        else
            serial_putc(0x61+s-10);
    }

	serial_wait_tx_empty();

}

SPL_STATIC_FUNC void serial_put_dec(unsigned int data)
{
	char szTxt[10];
	szTxt[0] = 0x30;
	int i = 0;

	do {
		szTxt[i++] = (data % 10) + 0x30;
		data = data / 10;
	} while(data);

	for(--i;i >=0;--i)	
		serial_putc(szTxt[i]);

	serial_wait_tx_empty();
}


#define serial_put_char(data) serial_puts("0x");serial_put_hex((unsigned)data,8);serial_puts("\n")
#define serial_put_dword(data) serial_puts("0x");serial_put_hex((unsigned)data,32);serial_puts("\n")
void do_exception(unsigned reason,unsigned lr)
{
    serial_puts("Enter Exception:");
    serial_put_dword(reason);
    serial_puts("\tlink addr:");
        serial_put_dword(lr);
#ifdef CONFIG_ENABLE_WATCHDOG
	AML_WATCH_DOG_START();//enable watch dog
#endif
}
