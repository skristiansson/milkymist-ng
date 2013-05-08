#ifndef __SYSTEM_H
#define __SYSTEM_H
#include <spr-defs.h>

#ifdef __cplusplus
extern "C" {
#endif

void flush_cpu_icache(void);
void flush_cpu_dcache(void);

static inline unsigned long mfspr(unsigned long add)
{
	unsigned long ret;

	__asm__ __volatile__ ("l.mfspr %0,r0,%1" : "=r" (ret) : "K" (add));

	return ret;
}

static inline void mtspr(unsigned long add, unsigned long val)
{
	__asm__ __volatile__ ("l.mtspr r0,%1,%0" : : "K" (add), "r" (val));
}

#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_H */
