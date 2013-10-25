#include <common.h>
#include <asm/arch/mx6.h>
#include <asm/arch/regs-anadig.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/crm_regs.h>
#ifdef CONFIG_CMD_CLOCK
#include <asm/clock.h>
#endif

static void mx6_snvs_poweroff(void)
{
#define SNVS_LPCR 0x38
	u32 value;
	
	value = readl(SNVS_BASE_ADDR + SNVS_LPCR);
	/*set TOP and DP_EN bit*/
	writel(value | 0x60, SNVS_BASE_ADDR + SNVS_LPCR);
}


int do_mx6_shutdown(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int wait_ms=1000;

	
	printf("machine shutdown ...\n");
	mx6_snvs_poweroff();

	//
	while(0<wait_ms--){
		udelay(1000);
	}

	//should not reach here
	printf("shutdown failed????\n");

	return 0;
}

U_BOOT_CMD(
	shutdown, 1, 1, do_mx6_shutdown,
	"shutdown - shutdown machine",
	"");


