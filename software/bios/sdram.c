#include <stdio.h>
#include <stdlib.h>

#include <hw/csr.h>
#include <hw/flags.h>
#include <hw/mem.h>

#include "sdram.h"

static void cdelay(int i)
{
	while(i > 0) {
		__asm__ volatile("l.nop");
		i--;
	}
}

static void command_p0(int cmd)
{
	dfii_pi0_command_write(cmd);
	dfii_pi0_command_issue_write(1);
}

static void command_p1(int cmd)
{
	dfii_pi1_command_write(cmd);
	dfii_pi1_command_issue_write(1);
}

static void init_sequence(void)
{
	int i;
	
	/* Bring CKE high */
	dfii_pi0_address_write(0x0000);
	dfii_pi0_baddress_write(0);
	dfii_control_write(DFII_CONTROL_CKE);
	
	/* Precharge All */
	dfii_pi0_address_write(0x0400);
	command_p0(DFII_COMMAND_RAS|DFII_COMMAND_WE|DFII_COMMAND_CS);
	
	/* Load Extended Mode Register */
	dfii_pi0_baddress_write(1);
	dfii_pi0_address_write(0x0000);
	command_p0(DFII_COMMAND_RAS|DFII_COMMAND_CAS|DFII_COMMAND_WE|DFII_COMMAND_CS);
	dfii_pi0_baddress_write(0);
	
	/* Load Mode Register */
	dfii_pi0_address_write(0x0132); /* Reset DLL, CL=3, BL=4 */
	command_p0(DFII_COMMAND_RAS|DFII_COMMAND_CAS|DFII_COMMAND_WE|DFII_COMMAND_CS);
	cdelay(200);
	
	/* Precharge All */
	dfii_pi0_address_write(0x0400);
	command_p0(DFII_COMMAND_RAS|DFII_COMMAND_WE|DFII_COMMAND_CS);
	
	/* 2x Auto Refresh */
	for(i=0;i<2;i++) {
		dfii_pi0_address_write(0);
		command_p0(DFII_COMMAND_RAS|DFII_COMMAND_CAS|DFII_COMMAND_CS);
		cdelay(4);
	}
	
	/* Load Mode Register */
	dfii_pi0_address_write(0x0032); /* CL=3, BL=4 */
	command_p0(DFII_COMMAND_RAS|DFII_COMMAND_CAS|DFII_COMMAND_WE|DFII_COMMAND_CS);
	cdelay(200);
}

void ddrsw(void)
{
	dfii_control_write(DFII_CONTROL_CKE);
	printf("DDR now under software control\n");
}

void ddrhw(void)
{
	dfii_control_write(DFII_CONTROL_SEL|DFII_CONTROL_CKE);
	printf("DDR now under hardware control\n");
}

void ddrrow(char *_row)
{
	char *c;
	unsigned int row;
	
	if(*_row == 0) {
		dfii_pi0_address_write(0x0000);
		dfii_pi0_baddress_write(0);
		command_p0(DFII_COMMAND_RAS|DFII_COMMAND_WE|DFII_COMMAND_CS);
		cdelay(15);
		printf("Precharged\n");
	} else {
		row = strtoul(_row, &c, 0);
		if(*c != 0) {
			printf("incorrect row\n");
			return;
		}
		dfii_pi0_address_write(row);
		dfii_pi0_baddress_write(0);
		command_p0(DFII_COMMAND_RAS|DFII_COMMAND_CS);
		cdelay(15);
		printf("Activated row %d\n", row);
	}
}

void ddrrd(char *startaddr)
{
	char *c;
	unsigned int addr;
	int i;

	if(*startaddr == 0) {
		printf("ddrrd <address>\n");
		return;
	}
	addr = strtoul(startaddr, &c, 0);
	if(*c != 0) {
		printf("incorrect address\n");
		return;
	}
	
	dfii_pi0_address_write(addr);
	dfii_pi0_baddress_write(0);
	command_p0(DFII_COMMAND_CAS|DFII_COMMAND_CS|DFII_COMMAND_RDDATA);
	cdelay(15);
	
	for(i=0;i<8;i++)
		printf("%02x", MMPTR(0xe0000834+4*i));
	for(i=0;i<8;i++)
		printf("%02x", MMPTR(0xe0000884+4*i));
	printf("\n");
}

void ddrwr(char *startaddr)
{
	char *c;
	unsigned int addr;
	int i;

	if(*startaddr == 0) {
		printf("ddrrd <address>\n");
		return;
	}
	addr = strtoul(startaddr, &c, 0);
	if(*c != 0) {
		printf("incorrect address\n");
		return;
	}
	
	for(i=0;i<8;i++) {
		MMPTR(0xe0000814+4*i) = i;
		MMPTR(0xe0000864+4*i) = 0xf0 + i;
	}
	
	dfii_pi1_address_write(addr);
	dfii_pi1_baddress_write(0);
	command_p1(DFII_COMMAND_CAS|DFII_COMMAND_WE|DFII_COMMAND_CS|DFII_COMMAND_WRDATA);
}

#define TEST_SIZE (4*1024*1024)

int memtest_silent(void)
{
	volatile unsigned int *array = (unsigned int *)SDRAM_BASE;
	int i;
	unsigned int prv;
	unsigned int error_cnt;
	
	prv = 0;
	for(i=0;i<TEST_SIZE/4;i++) {
		prv = 1664525*prv + 1013904223;
		array[i] = prv;
	}
	
	prv = 0;
	error_cnt = 0;
	for(i=0;i<TEST_SIZE/4;i++) {
		prv = 1664525*prv + 1013904223;
		if(array[i] != prv)
			error_cnt++;
	}
	return error_cnt;
}

int memtest(void)
{
	unsigned int e;

	e = memtest_silent();
	if(e != 0) {
		printf("Memtest failed: %d/%d words incorrect\n", e, TEST_SIZE/4);
		return 0;
	} else {
		printf("Memtest OK\n");
		return 1;
	}
}

int ddrinit(void)
{
	printf("Initializing DDR SDRAM...\n");
	
	init_sequence();
	dfii_control_write(DFII_CONTROL_SEL|DFII_CONTROL_CKE);
	if(!memtest())
		return 0;
	
	return 1;
}

static const char *format_slot_state(int state)
{
	switch(state) {
		case 0: return "Empty";
		case 1: return "Pending";
		case 2: return "Processing";
		default: return "UNEXPECTED VALUE";
	}
}

void asmiprobe(void)
{
	volatile unsigned int *regs = (unsigned int *)ASMIPROBE_BASE;
	int slot_count;
	int trace_depth;
	int i;
	int offset;
	
	offset = 0;
	slot_count = regs[offset++];
	trace_depth = regs[offset++];
	for(i=0;i<slot_count;i++)
		printf("Slot #%d: %s\n", i, format_slot_state(regs[offset++]));
	printf("Latest tags:\n");
	for(i=0;i<trace_depth;i++)
		printf("%d ", regs[offset++]);
	printf("\n");
}
