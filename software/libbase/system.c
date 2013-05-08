#include <irq.h>
#include <uart.h>

#include <system.h>

void flush_cpu_icache(void)
{
	unsigned long iccfgr;
	unsigned long cache_set_size;
	unsigned long cache_ways;
	unsigned long cache_block_size;
	unsigned long cache_size;
	int i;

	iccfgr = mfspr(SPR_ICCFGR);
	cache_ways = 1 << (iccfgr & SPR_ICCFGR_NCW);
	cache_set_size = 1 << ((iccfgr & SPR_ICCFGR_NCS) >> 3);
	cache_block_size = (iccfgr & SPR_ICCFGR_CBS) ? 32 : 16;
	cache_size = cache_set_size * cache_ways * cache_block_size;

	for (i = 0; i < cache_size; i += cache_block_size)
		mtspr(SPR_ICBIR, i);
}

void flush_cpu_dcache(void)
{
	unsigned long dccfgr;
	unsigned long cache_set_size;
	unsigned long cache_ways;
	unsigned long cache_block_size;
	unsigned long cache_size;
	int i;

	dccfgr = mfspr(SPR_DCCFGR);
	cache_ways = 1 << (dccfgr & SPR_ICCFGR_NCW);
	cache_set_size = 1 << ((dccfgr & SPR_DCCFGR_NCS) >> 3);
	cache_block_size = (dccfgr & SPR_DCCFGR_CBS) ? 32 : 16;
	cache_size = cache_set_size * cache_ways * cache_block_size;

	for (i = 0; i < cache_size; i += cache_block_size)
		mtspr(SPR_DCBIR, i);
}
