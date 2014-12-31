#include <common.h>

#include <asm/io.h>

#include <asm/cache-l2x0.h>


static void cache_wait(unsigned reg, unsigned long mask)
{
	/* wait for the operation to complete */
	while (readl(reg) & mask)
		;
}

void cache_sync(void)
{
	writel(0, L2X0_CACHE_SYNC);
	cache_wait(L2X0_CACHE_SYNC, 1);
}

void l2x0_clean_line(unsigned long addr)
{
	cache_wait(L2X0_CLEAN_LINE_PA, 1);
	writel(addr,  L2X0_CLEAN_LINE_PA);
}
void l2x0_flush_line(unsigned long addr)
{
	cache_wait( L2X0_CLEAN_INV_LINE_PA, 1);
	writel(addr,L2X0_CLEAN_INV_LINE_PA);
}
void l2x0_inv_line(unsigned long addr)
{
	cache_wait(L2X0_INV_LINE_PA, 1);
	writel(addr,  L2X0_INV_LINE_PA);
}


void l2x0_inv_all(void)
{
    
    /* invalidate all ways */
	writel(0xff, L2X0_INV_WAY);
	cache_wait(L2X0_INV_WAY, 0xff);
	cache_sync();
}


void l2x0_clean_all ()
{
   
	/* invalidate all ways */
	writel(0xff, L2X0_CLEAN_WAY);
	cache_wait(L2X0_CLEAN_WAY, 0xff);
	cache_sync();
}
void l2x0_clean_inv_all ()
{
   
	/* invalidate all ways */
	writel(0xff, L2X0_CLEAN_INV_WAY);
	cache_wait(L2X0_CLEAN_INV_WAY, 0xff);
	cache_sync();
}
void l2x0_wait_inv(void)
{
    cache_wait( L2X0_INV_LINE_PA, 1);
	cache_sync();
}
void l2x0_wait_clean(void)
{
    cache_wait( L2X0_CLEAN_LINE_PA, 1);
	cache_sync();
}
void l2x0_wait_flush(void)
{
    cache_wait( L2X0_CLEAN_INV_LINE_PA, 1);
	cache_sync();
}
#define CACHE_LINE_SIZE 32
#define debug_writel(a) 
void l2x0_invalid_range(unsigned long start, unsigned long end)
{
	if (start & (CACHE_LINE_SIZE - 1)) {
		start &= ~(CACHE_LINE_SIZE - 1);
		l2x0_flush_line(start);
		start += CACHE_LINE_SIZE;
	}

	if (end & (CACHE_LINE_SIZE - 1)) {
		end &= ~(CACHE_LINE_SIZE - 1);
		l2x0_flush_line(end);
	}

	while (start < end) {
		unsigned long blk_end = start + min(end - start, 4096UL);

		while (start < blk_end) {
			l2x0_inv_line(start);
			start += CACHE_LINE_SIZE;
		}

		if (blk_end < end) {
			
		}
	}
	cache_wait( L2X0_INV_LINE_PA, 1);
	cache_sync();
	
    
}

void l2x0_clean_range(unsigned long start, unsigned long end)
{

	
	start &= ~(CACHE_LINE_SIZE - 1);
	while (start < end) {
		unsigned long blk_end = start + end - start;

		while (start < blk_end) {
			l2x0_clean_line(start);
			start += CACHE_LINE_SIZE;
		}
	}
	cache_wait(L2X0_CLEAN_LINE_PA, 1);
	cache_sync();
	
}

void l2x0_flush_range(unsigned long start, unsigned long end)
{
	start &= ~(CACHE_LINE_SIZE - 1);
	while (start < end) {
			l2x0_flush_line(start);
			start += CACHE_LINE_SIZE;
	}
	cache_wait(L2X0_CLEAN_INV_LINE_PA, 1);
	cache_sync();
}
int l2x0_status()
{
    return readl( L2X0_CTRL) & 1;
}
void l2x0_enable()
{
	__u32 aux;

	/*
	 * Check if l2x0 controller is already enabled.
	 * If you are booting from non-secure mode
	 * accessing the below registers will fault.
	 */
	if (!(readl( L2X0_CTRL) & 1)) {

		/* l2x0 controller is disabled */

		aux = readl(L2X0_AUX_CTRL);
		aux &= 0xff800fff;
		//aux |= 0x00020000;
		aux |= 0x7c420001;
		writel(aux,L2X0_AUX_CTRL);

		l2x0_inv_all();

		/* enable L2X0 */
		writel(1,  L2X0_CTRL);
	}

}
void l2x0_disable()
{
    writel(0,  L2X0_CTRL);
}


/////////////////////////////////////////////////////////////////////////////
#if defined(CONFIG_L2_CACHE_BOOST)
void l2x0_disable_x(void)
{

#if defined(CONFIG_MESON_TRUSTZONE)
	//#error "l2x0_disable_x() is not implement yet!"
#else
	if ((readl( L2X0_CTRL) & 1))
	{
		extern void dcache_flush(void);
		dcache_flush();
		l2x0_clean_inv_all();
	    writel(0,  L2X0_CTRL);
	}
#endif //CONFIG_MESON_TRUSTZONE
}
void l2x0_enable_x(void)
{
#if defined(CONFIG_MESON_TRUSTZONE)
	//#error "l2x0_enable_x() is not implement yet!"
#else
	__u32 aux;
	
	/*
	 * Check if l2x0 controller is already enabled.
	 * If you are booting from non-secure mode
	 * accessing the below registers will fault.
	 */
	if (!(readl( L2X0_CTRL) & 1)) {

		/* l2x0 controller is disabled */

		//SCU setup
		#define SCU_BASE 0xc4300000
		writel(readl(SCU_BASE + 0x0) | 1, (SCU_BASE+0x0));
		writel(0xF, (SCU_BASE+0x0C));
		
		//set latency
		writel(0x71000007,L2X0_PREFETCH_CTRL);	
		unsigned int data[3] = { 3, 3, 3 };
		unsigned int tag[3] = { 2, 2, 2 };

#define L2X0_LATENCY_CTRL_SETUP_SHIFT   (0)
#define L2X0_LATENCY_CTRL_RD_SHIFT      (4)
#define L2X0_LATENCY_CTRL_WR_SHIFT      (8)

		writel(
				((tag[0] - 1) << L2X0_LATENCY_CTRL_RD_SHIFT) |
				((tag[1] - 1) << L2X0_LATENCY_CTRL_WR_SHIFT) |
				((tag[2] - 1) << L2X0_LATENCY_CTRL_SETUP_SHIFT),
				L2X0_TAG_LATENCY_CTRL);	
		writel(
				((data[0] - 1) << L2X0_LATENCY_CTRL_RD_SHIFT) |
				((data[1] - 1) << L2X0_LATENCY_CTRL_WR_SHIFT) |
				((data[2] - 1) << L2X0_LATENCY_CTRL_SETUP_SHIFT),
				L2X0_DATA_LATENCY_CTRL);		

		//set AUX
		aux = readl(L2X0_AUX_CTRL);
		aux |= 0x7ec00001;
		writel(aux,L2X0_AUX_CTRL);

		l2x0_inv_all();

		/* enable L2X0 */
		writel(1,  L2X0_CTRL);
	}		
#endif //CONFIG_MESON_TRUSTZONE
}
#endif //CONFIG_L2_CACHE_BOOST