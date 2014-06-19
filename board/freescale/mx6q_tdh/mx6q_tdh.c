/*
 * Copyright (C) 2012-2013 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <version.h>
#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/arch/mx6.h>
#include <bmpmanager.h>
#include <asm/arch/mx6_pins.h>
#include <mipi_common.h>
#include <mipi_dsi.h>
#if defined(CONFIG_SECURE_BOOT)
#include <asm/arch/mx6_secure.h>
#endif
#include <asm/arch/crm_regs.h>
#include <asm/arch/mx6dl_pins.h>
#include <asm/arch/iomux-v3.h>
#include <asm/arch/regs-anadig.h>
#include <asm/errno.h>

#if defined(CONFIG_VIDEO_MX5)
#include <asm/imx_pwm.h>
#include <linux/list.h>
#include <linux/fb.h>
#include <linux/mxcfb.h>
#include <ipu.h>
#endif
#if defined(CONFIG_VIDEO_MX5) || defined(CONFIG_MXC_EPDC)
#include <lcd.h>
#endif

#include "../../../drivers/video/mxc_epdc_fb.h"


#if CONFIG_I2C_MXC
#include <i2c.h>
#endif

#ifdef CONFIG_CMD_MMC
#include <mmc.h>
#include <fsl_esdhc.h>
#endif

#ifdef CONFIG_ARCH_MMU
#include <asm/mmu.h>
#include <asm/arch/mmu.h>
#endif

#ifdef CONFIG_CMD_CLOCK
#include <asm/clock.h>
#endif

#ifdef CONFIG_CMD_IMXOTP
#include <imx_otp.h>
#endif

#ifdef CONFIG_MXC_GPIO
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#endif

#ifdef CONFIG_ANDROID_RECOVERY
#include <recovery.h>
#include <bootloader.h>
#endif



#include <bmpmanager.h>

#include "powersupply.h"

DECLARE_GLOBAL_DATA_PTR;
enum {
 eBootModeNormal=0,
 eBootModeRecovery,
 eBootModeCharger,
 eBootModeFastboot,
 eBootModeAutoupdate,
 eBootModeFactory,
 eBootModeMax,
};

enum {
 eBootReasonPOR,
 eBootReasonWDOG,
 eBootReasonReset,
 eBootReasonCharger,
};
static int android_bootmode=eBootModeNormal;
static int system_boot_reason=eBootReasonPOR;
static char* panel_name=NULL;
static unsigned char bmp_bat0[]={
	#include "bat0.inc"
};

//static int bmp_bat0_size = sizeof(bmp_bat0);


static enum boot_device boot_dev;

#define ENABLE_KEYPAD_SHORTCUT

#define BOARD_REV_INVALID 0x0
#define BOARD_TDH_REVA 0x1
#define BOARD_TDH_REVB 0x2
#define BOARD_TDB_REVC 0x3

#ifndef NEW_PAD_CTRL
#define NEW_PAD_CTRL(cfg, pad)	(((cfg) & ~MUX_PAD_CTRL_MASK) | \
		MUX_PAD_CTRL(pad))
#endif

#define MX6_KEY_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE  |		\
		PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
		PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_SLOW  | PAD_CTL_HYS)

#define KEY_HOT_KEY1_IO IMX_GPIO_NR(4,14)
#define KEY_HOT_KEY2_IO IMX_GPIO_NR(4,11)
#define KEY_PTT_IO		IMX_GPIO_NR(4,10)
#define KEY_VOLPLUS_IO	IMX_GPIO_NR(6,9)
#define KEY_VOLMINUS_IO	IMX_GPIO_NR(6,10)


#define TDH_MOTOR_PWR_EN	IMX_GPIO_NR(1, 4)



#ifdef CONFIG_VIDEO_MX5
extern unsigned char fsl_bmp_reversed_600x400[];
extern int fsl_bmp_reversed_600x400_size;
extern int g_ipu_hw_rev;

#if defined(CONFIG_BMP_8BPP)
unsigned short colormap[256];
#elif defined(CONFIG_BMP_16BPP)
unsigned short colormap[65536];
#else
unsigned short colormap[16777216];
#endif

static struct pwm_device pwm0 = {
	.pwm_id = 0,
	.pwmo_invert = 0,
};

static int di = 1;

extern int ipuv3_fb_init(struct fb_videomode *mode, int di,
			int interface_pix_fmt,
			ipu_di_clk_parent_t di_clk_parent,
			int di_clk_val);

#if MIPI_DSI
static struct fb_videomode mipi_dsi = {
	 "mipi",
	 60,/*refresh*/
	 540,960,/*xres,yres*/
	 30000,/*pixclock ps*/
	 10, 10, /*left margin,right margin*/
	 50, 30,/*upper margin,lower margin*/
	 10,10,/*hsync len,vsync len*/
	 FB_SYNC_OE_LOW_ACT,/*sync*/
	 FB_VMODE_NONINTERLACED,/*vmode*/
	 0,/*flag*/
};
#else
static struct fb_videomode lvds_wvga = {
	 "WVGA", 60, 800, 480, 29850, 89, 164, 23, 10, 10, 10,
	 FB_SYNC_EXT,
	 FB_VMODE_NONINTERLACED,
	 0,
};
#endif

static struct mipi_lcd_config mipilcd_config = {
	.virtual_ch		= 0x0,
	.data_lane_num  = 0x2,
	.max_phy_clk    = 450,
	.dpi_fmt		= MIPI_RGB888,
};

vidinfo_t panel_info;
#endif

static inline void setup_boot_device(void)
{
	uint soc_sbmr = readl(SRC_BASE_ADDR + 0x4);
	uint bt_mem_ctl = (soc_sbmr & 0x000000FF) >> 4 ;
	uint bt_mem_type = (soc_sbmr & 0x00000008) >> 3;

	switch (bt_mem_ctl) {
	case 0x0:
		if (bt_mem_type)
			boot_dev = ONE_NAND_BOOT;
		else
			boot_dev = WEIM_NOR_BOOT;
		break;
	case 0x2:
			boot_dev = SATA_BOOT;
		break;
	case 0x3:
		if (bt_mem_type)
			boot_dev = I2C_BOOT;
		else
			boot_dev = SPI_NOR_BOOT;
		break;
	case 0x4:
	case 0x5:
		boot_dev = SD_BOOT;
		break;
	case 0x6:
	case 0x7:
		boot_dev = MMC_BOOT;
		break;
	case 0x8 ... 0xf:
		boot_dev = NAND_BOOT;
		break;
	default:
		boot_dev = UNKNOWN_BOOT;
		break;
	}
}

enum boot_device get_boot_device(void)
{
	return boot_dev;
}

u32 get_board_rev(void)
{
	return fsl_system_rev;
}

#ifdef CONFIG_ARCH_MMU
void board_mmu_init(void)
{
	unsigned long ttb_base = PHYS_SDRAM_1 + 0x4000;
	unsigned long i;

	/*
	* Set the TTB register
	*/
	asm volatile ("mcr  p15,0,%0,c2,c0,0" : : "r"(ttb_base) /*:*/);

	/*
	* Set the Domain Access Control Register
	*/
	i = ARM_ACCESS_DACR_DEFAULT;
	asm volatile ("mcr  p15,0,%0,c3,c0,0" : : "r"(i) /*:*/);

	/*
	* First clear all TT entries - ie Set them to Faulting
	*/
	memset((void *)ttb_base, 0, ARM_FIRST_LEVEL_PAGE_TABLE_SIZE);
	/* Actual   Virtual  Size   Attributes          Function */
	/* Base     Base     MB     cached? buffered?  access permissions */
	/* xxx00000 xxx00000 */
	X_ARM_MMU_SECTION(0x000, 0x000, 0x001,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* ROM, 1M */
	X_ARM_MMU_SECTION(0x001, 0x001, 0x008,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* 8M */
	X_ARM_MMU_SECTION(0x009, 0x009, 0x001,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* IRAM */
	X_ARM_MMU_SECTION(0x00A, 0x00A, 0x0F6,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* 246M */

	/* 2 GB memory starting at 0x10000000, only map 1.875 GB */
	X_ARM_MMU_SECTION(0x100, 0x100, 0x780,
			ARM_CACHEABLE, ARM_BUFFERABLE,
			ARM_ACCESS_PERM_RW_RW);
	/* uncached alias of the same 1.875 GB memory */
	X_ARM_MMU_SECTION(0x100, 0x880, 0x780,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW);

	/* Enable MMU */
	MMU_ON();
}
#endif

#define ANATOP_PLL_LOCK                 0x80000000
#define ANATOP_PLL_ENABLE_MASK          0x00002000
#define ANATOP_PLL_BYPASS_MASK          0x00010000
#define ANATOP_PLL_PWDN_MASK            0x00001000
#define ANATOP_PLL_HOLD_RING_OFF_MASK   0x00000800
#define ANATOP_SATA_CLK_ENABLE_MASK     0x00100000


/* Note: udelay() is not accurate for i2c timing */
static void __udelay(int time)
{
	int i, j;

	for (i = 0; i < time; i++) {
		for (j = 0; j < 200; j++) {
			asm("nop");
			asm("nop");
		}
	}
}

static void sdelay(int s){
  s*=1000;
  while(s>0){
	s--;
	__udelay(1000);
  }
}


int dram_init(void)
{
	/*
	 * Switch PL301_FAST2 to DDR Dual-channel mapping
	 * however this block the boot up, temperory redraw
	 */
	/*
	 * u32 reg = 1;
	 * writel(reg, GPV0_BASE_ADDR);
	 */

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

static void setup_uart(void)
{
	/* UART1 TXD */
	mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_CSI0_DAT10__UART1_TXD));

	/* UART1 RXD */
	mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_CSI0_DAT11__UART1_RXD));
}

#ifdef CONFIG_VIDEO_MX5
void setup_lvds_poweron(void)
{
	int reg;
	mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_NANDF_RB0__GPIO_6_10));

	reg = readl(GPIO6_BASE_ADDR + GPIO_GDIR);
	reg |= (1 << 10);
	writel(reg, GPIO6_BASE_ADDR + GPIO_GDIR);

	reg = readl(GPIO6_BASE_ADDR + GPIO_DR);
	reg |= (1 << 10);
	writel(reg, GPIO6_BASE_ADDR + GPIO_DR);
}
#endif

#ifdef CONFIG_I2C_MXC
#define I2C1_SDA_GPIO5_26_BIT_MASK  (1 << 26)
#define I2C1_SCL_GPIO5_27_BIT_MASK  (1 << 27)
#define I2C2_SCL_GPIO4_12_BIT_MASK  (1 << 12)
#define I2C2_SDA_GPIO4_13_BIT_MASK  (1 << 13)
#define I2C3_SCL_GPIO1_3_BIT_MASK   (1 << 3)
#define I2C3_SDA_GPIO1_6_BIT_MASK   (1 << 6)

extern u32 I2C_BASE;

static void setup_i2c(unsigned int module_base)
{
	unsigned int reg;

	switch (module_base) {
	case I2C1_BASE_ADDR:
		/* i2c1 SDA */
		mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_CSI0_DAT8__I2C1_SDA));

		/* i2c1 SCL */
		mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_CSI0_DAT9__I2C1_SCL));

		/* Enable i2c clock */
		reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR2);
		reg |= 0xC0;
		writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR2);

		break;
	case I2C2_BASE_ADDR:
		/* i2c2 SDA */
		mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_KEY_ROW3__I2C2_SDA));

		/* i2c2 SCL */
		mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_KEY_COL3__I2C2_SCL));

		/* Enable i2c clock */
		reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR2);
		reg |= 0x300;
		writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR2);

		break;
	case I2C3_BASE_ADDR:
		/* GPIO_3 for I2C3_SCL */
		mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_GPIO_3__I2C3_SCL));
		/* GPIO_6 for I2C3_SDA */
		mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_GPIO_6__I2C3_SDA));
		/* Enable i2c clock */
		reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR2);
		reg |= 0xC00;
		writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR2);

		break;
	default:
		printf("Invalid I2C base: 0x%x\n", module_base);
		break;
	}

	I2C_BASE  = module_base;
}

static void mx6q_i2c_gpio_scl_direction(int bus, int output)
{
	u32 reg;

	switch (bus) {
	case 1:
		mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_CSI0_DAT9__GPIO_5_27));
		reg = readl(GPIO5_BASE_ADDR + GPIO_GDIR);
		if (output)
			reg |= I2C1_SCL_GPIO5_27_BIT_MASK;
		else
			reg &= ~I2C1_SCL_GPIO5_27_BIT_MASK;
		writel(reg, GPIO5_BASE_ADDR + GPIO_GDIR);
		break;
	case 2:
		mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_KEY_COL3__GPIO_4_12));
		reg = readl(GPIO4_BASE_ADDR + GPIO_GDIR);
		if (output)
			reg |= I2C2_SCL_GPIO4_12_BIT_MASK;
		else
			reg &= ~I2C2_SCL_GPIO4_12_BIT_MASK;
		writel(reg, GPIO4_BASE_ADDR + GPIO_GDIR);
		break;
	case 3:
		mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_GPIO_3__GPIO_1_3));
		reg = readl(GPIO1_BASE_ADDR + GPIO_GDIR);
		if (output)
			reg |= I2C3_SCL_GPIO1_3_BIT_MASK;
		else
			reg &= I2C3_SCL_GPIO1_3_BIT_MASK;
		writel(reg, GPIO1_BASE_ADDR + GPIO_GDIR);
		break;
	}
}

/* set 1 to output, sent 0 to input */
static void mx6q_i2c_gpio_sda_direction(int bus, int output)
{
	u32 reg;

	switch (bus) {
	case 1:
		mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_CSI0_DAT8__GPIO_5_26));
		reg = readl(GPIO5_BASE_ADDR + GPIO_GDIR);
		if (output)
			reg |= I2C1_SDA_GPIO5_26_BIT_MASK;
		else
			reg &= ~I2C1_SDA_GPIO5_26_BIT_MASK;
		writel(reg, GPIO5_BASE_ADDR + GPIO_GDIR);
		break;
	case 2:
		mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_KEY_ROW3__GPIO_4_13));
		reg = readl(GPIO4_BASE_ADDR + GPIO_GDIR);
		if (output)
			reg |= I2C2_SDA_GPIO4_13_BIT_MASK;
		else
			reg &= ~I2C2_SDA_GPIO4_13_BIT_MASK;
		writel(reg, GPIO4_BASE_ADDR + GPIO_GDIR);
	case 3:
		mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_GPIO_6__GPIO_1_6));
		reg = readl(GPIO1_BASE_ADDR + GPIO_GDIR);
		if (output)
			reg |= I2C3_SDA_GPIO1_6_BIT_MASK;
		else
			reg &= ~I2C3_SDA_GPIO1_6_BIT_MASK;
		writel(reg, GPIO1_BASE_ADDR + GPIO_GDIR);
	default:
		break;
	}
}

/* set 1 to high 0 to low */
static void mx6q_i2c_gpio_scl_set_level(int bus, int high)
{
	u32 reg;

	switch (bus) {
	case 1:
		reg = readl(GPIO5_BASE_ADDR + GPIO_DR);
		if (high)
			reg |= I2C1_SCL_GPIO5_27_BIT_MASK;
		else
			reg &= ~I2C1_SCL_GPIO5_27_BIT_MASK;
		writel(reg, GPIO5_BASE_ADDR + GPIO_DR);
		break;
	case 2:
		reg = readl(GPIO4_BASE_ADDR + GPIO_DR);
		if (high)
			reg |= I2C2_SCL_GPIO4_12_BIT_MASK;
		else
			reg &= ~I2C2_SCL_GPIO4_12_BIT_MASK;
		writel(reg, GPIO4_BASE_ADDR + GPIO_DR);
		break;
	case 3:
		reg = readl(GPIO1_BASE_ADDR + GPIO_DR);
		if (high)
			reg |= I2C3_SCL_GPIO1_3_BIT_MASK;
		else
			reg &= ~I2C3_SCL_GPIO1_3_BIT_MASK;
		writel(reg, GPIO1_BASE_ADDR + GPIO_DR);
		break;
	}
}

/* set 1 to high 0 to low */
static void mx6q_i2c_gpio_sda_set_level(int bus, int high)
{
	u32 reg;

	switch (bus) {
	case 1:
		reg = readl(GPIO5_BASE_ADDR + GPIO_DR);
		if (high)
			reg |= I2C1_SDA_GPIO5_26_BIT_MASK;
		else
			reg &= ~I2C1_SDA_GPIO5_26_BIT_MASK;
		writel(reg, GPIO5_BASE_ADDR + GPIO_DR);
		break;
	case 2:
		reg = readl(GPIO4_BASE_ADDR + GPIO_DR);
		if (high)
			reg |= I2C2_SDA_GPIO4_13_BIT_MASK;
		else
			reg &= ~I2C2_SDA_GPIO4_13_BIT_MASK;
		writel(reg, GPIO4_BASE_ADDR + GPIO_DR);
		break;
	case 3:
		reg = readl(GPIO1_BASE_ADDR + GPIO_DR);
		if (high)
			reg |= I2C3_SDA_GPIO1_6_BIT_MASK;
		else
			reg &= ~I2C3_SDA_GPIO1_6_BIT_MASK;
		writel(reg, GPIO1_BASE_ADDR + GPIO_DR);
		break;
	}
}

static int mx6q_i2c_gpio_check_sda(int bus)
{
	u32 reg;
	int result = 0;

	switch (bus) {
	case 1:
		reg = readl(GPIO5_BASE_ADDR + GPIO_PSR);
		result = !!(reg & I2C1_SDA_GPIO5_26_BIT_MASK);
		break;
	case 2:
		reg = readl(GPIO4_BASE_ADDR + GPIO_PSR);
		result = !!(reg & I2C2_SDA_GPIO4_13_BIT_MASK);
		break;
	case 3:
		reg = readl(GPIO1_BASE_ADDR + GPIO_PSR);
		result = !!(reg & I2C3_SDA_GPIO1_6_BIT_MASK);
		break;
	}

	return result;
}

 /* Random reboot cause i2c SDA low issue:
  * the i2c bus busy because some device pull down the I2C SDA
  * line. This happens when Host is reading some byte from slave, and
  * then host is reset/reboot. Since in this case, device is
  * controlling i2c SDA line, the only thing host can do this give the
  * clock on SCL and sending NAK, and STOP to finish this
  * transaction.
  *
  * How to fix this issue:
  * detect if the SDA was low on bus send 8 dummy clock, and 1
  * clock + NAK, and STOP to finish i2c transaction the pending
  * transfer.
  */
int i2c_bus_recovery(void)
{
	int i, bus, result = 0;

	for (bus = 1; bus <= 3; bus++) {
		mx6q_i2c_gpio_sda_direction(bus, 0);

		if (mx6q_i2c_gpio_check_sda(bus) == 0) {
			printf("i2c: I2C%d SDA is low, start i2c recovery...\n", bus);
			mx6q_i2c_gpio_scl_direction(bus, 1);
			mx6q_i2c_gpio_scl_set_level(bus, 1);
			__udelay(10000);

			for (i = 0; i < 9; i++) {
				mx6q_i2c_gpio_scl_set_level(bus, 1);
				__udelay(5);
				mx6q_i2c_gpio_scl_set_level(bus, 0);
				__udelay(5);
			}

			/* 9th clock here, the slave should already
			   release the SDA, we can set SDA as high to
			   a NAK.*/
			mx6q_i2c_gpio_sda_direction(bus, 1);
			mx6q_i2c_gpio_sda_set_level(bus, 1);
			__udelay(1); /* Pull up SDA first */
			mx6q_i2c_gpio_scl_set_level(bus, 1);
			__udelay(5); /* plus pervious 1 us */
			mx6q_i2c_gpio_scl_set_level(bus, 0);
			__udelay(5);
			mx6q_i2c_gpio_sda_set_level(bus, 0);
			__udelay(5);
			mx6q_i2c_gpio_scl_set_level(bus, 1);
			__udelay(5);
			/* Here: SCL is high, and SDA from low to high, it's a
			 * stop condition */
			mx6q_i2c_gpio_sda_set_level(bus, 1);
			__udelay(5);

			mx6q_i2c_gpio_sda_direction(bus, 0);
			if (mx6q_i2c_gpio_check_sda(bus) == 1)
				printf("I2C%d Recovery success\n", bus);
			else {
				printf("I2C%d Recovery failed, I2C SDA still low!!!\n", bus);
				result |= 1 << bus;
			}
		}

		/* configure back to i2c */
		switch (bus) {
		case 1:
			setup_i2c(I2C1_BASE_ADDR);
			break;
		case 2:
			setup_i2c(I2C2_BASE_ADDR);
			break;
		case 3:
			setup_i2c(I2C3_BASE_ADDR);
			break;
		}
	}

	return result;
}


static int setup_pmic_voltages(void)
{
	unsigned char value, rev_id = 0 ;
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	if (!i2c_probe(0x8)) {
		if (i2c_read(0x8, 0, 1, &value, 1)) {
			printf("Read device ID error!\n");
			return -1;
		}
		if (i2c_read(0x8, 3, 1, &rev_id, 1)) {
			printf("Read Rev ID error!\n");
			return -1;
		}
		printf("Found PFUZE100! deviceid=%x,revid=%x\n", value, rev_id);
		/*increase SW2->3.3*/
		value=0x72;
		if (i2c_write(0x8, 0x35, 1, &value, 1)) {
			printf("Set SW2VOLT error!\n");
			return -1;
		}

		if (i2c_write(0x8, 0x36, 1, &value, 1)) {
			printf("Set SW2STBY error!\n");
			return -1;
		}

		if (i2c_write(0x8, 0x37, 1, &value, 1)) {
			printf("Set SW2OFF error!\n");
			return -1;
		}
		/*For camera streaks issue,swap VGEN5 and VGEN3 to power camera.
		*sperate VDDHIGH_IN and camera 2.8V power supply, after switch:
		*VGEN5 for VDDHIGH_IN and increase to 3V to align with datasheet
		*VGEN3 for camera 2.8V power supply
		*/
		/*increase VGEN3 from 2.5 to 2.8V*/
 		value =0x3a;
		if (i2c_write(0x8, 0x6e, 1, &value, 1)) {
			printf("Set VGEN3 error!\n");
			return -1;
		}
		/*increase VGEN5 from 2.8 to 3V*/
		if (i2c_read(0x8, 0x70, 1, &value, 1)) {
			printf("Read VGEN5 error!\n");
			return -1;
		}
		value &= ~0xf;
		value |= 0xc;
		if (i2c_write(0x8, 0x70, 1, &value, 1)) {
			printf("Set VGEN5 error!\n");
			return -1;
		}

		/*decrease VGEN6 to 2.8V*/ 
		value =0x1a;
		if (i2c_write(0x8, 0x71, 1, &value, 1)) {
			printf("Set VGEN6 error!\n");
			return -1;
		}
		return 0;
	}

	return -1;
}
#endif


#ifdef CONFIG_NET_MULTI
int board_eth_init(bd_t *bis)
{
	int rc = -ENODEV;

	return rc;
}
#endif

#ifdef CONFIG_CMD_MMC

/* On this board, only SD3 can support 1.8V signalling
 * that is required for UHS-I mode of operation.
 * Last element in struct is used to indicate 1.8V support.
 */
struct fsl_esdhc_cfg usdhc_cfg[4] = {
	{USDHC1_BASE_ADDR, 1, 1, 1, 0},
	{USDHC2_BASE_ADDR, 1, 1, 1, 0},
	{USDHC3_BASE_ADDR, 1, 1, 1, 0},
	{USDHC4_BASE_ADDR, 1, 1, 1, 0},
};

#if defined CONFIG_MX6Q
iomux_v3_cfg_t usdhc1_pads[] = {
	MX6Q_PAD_SD1_CLK__USDHC1_CLK,
	MX6Q_PAD_SD1_CMD__USDHC1_CMD,
	MX6Q_PAD_SD1_DAT0__USDHC1_DAT0,
	MX6Q_PAD_SD1_DAT1__USDHC1_DAT1,
	MX6Q_PAD_SD1_DAT2__USDHC1_DAT2,
	MX6Q_PAD_SD1_DAT3__USDHC1_DAT3,
};

iomux_v3_cfg_t usdhc2_pads[] = {
	MX6Q_PAD_SD2_CLK__USDHC2_CLK,
	MX6Q_PAD_SD2_CMD__USDHC2_CMD,
	MX6Q_PAD_SD2_DAT0__USDHC2_DAT0,
	MX6Q_PAD_SD2_DAT1__USDHC2_DAT1,
	MX6Q_PAD_SD2_DAT2__USDHC2_DAT2,
	MX6Q_PAD_SD2_DAT3__USDHC2_DAT3,
};

iomux_v3_cfg_t usdhc3_pads[] = {
	MX6Q_PAD_SD3_CLK__USDHC3_CLK,
	MX6Q_PAD_SD3_CMD__USDHC3_CMD,
	MX6Q_PAD_SD3_DAT0__USDHC3_DAT0,
	MX6Q_PAD_SD3_DAT1__USDHC3_DAT1,
	MX6Q_PAD_SD3_DAT2__USDHC3_DAT2,
	MX6Q_PAD_SD3_DAT3__USDHC3_DAT3,
	MX6Q_PAD_SD3_DAT4__USDHC3_DAT4,
	MX6Q_PAD_SD3_DAT5__USDHC3_DAT5,
	MX6Q_PAD_SD3_DAT6__USDHC3_DAT6,
	MX6Q_PAD_SD3_DAT7__USDHC3_DAT7,
};

iomux_v3_cfg_t usdhc4_pads[] = {
	MX6Q_PAD_SD4_CLK__USDHC4_CLK,
	MX6Q_PAD_SD4_CMD__USDHC4_CMD,
	MX6Q_PAD_SD4_DAT0__USDHC4_DAT0,
	MX6Q_PAD_SD4_DAT1__USDHC4_DAT1,
	MX6Q_PAD_SD4_DAT2__USDHC4_DAT2,
	MX6Q_PAD_SD4_DAT3__USDHC4_DAT3,
	MX6Q_PAD_SD4_DAT4__USDHC4_DAT4,
	MX6Q_PAD_SD4_DAT5__USDHC4_DAT5,
	MX6Q_PAD_SD4_DAT6__USDHC4_DAT6,
	MX6Q_PAD_SD4_DAT7__USDHC4_DAT7,
};
#elif defined CONFIG_MX6DL
iomux_v3_cfg_t usdhc1_pads[] = {
	MX6DL_PAD_SD1_CLK__USDHC1_CLK,
	MX6DL_PAD_SD1_CMD__USDHC1_CMD,
	MX6DL_PAD_SD1_DAT0__USDHC1_DAT0,
	MX6DL_PAD_SD1_DAT1__USDHC1_DAT1,
	MX6DL_PAD_SD1_DAT2__USDHC1_DAT2,
	MX6DL_PAD_SD1_DAT3__USDHC1_DAT3,
};

iomux_v3_cfg_t usdhc2_pads[] = {
	MX6DL_PAD_SD2_CLK__USDHC2_CLK,
	MX6DL_PAD_SD2_CMD__USDHC2_CMD,
	MX6DL_PAD_SD2_DAT0__USDHC2_DAT0,
	MX6DL_PAD_SD2_DAT1__USDHC2_DAT1,
	MX6DL_PAD_SD2_DAT2__USDHC2_DAT2,
	MX6DL_PAD_SD2_DAT3__USDHC2_DAT3,
};

iomux_v3_cfg_t usdhc3_pads[] = {
	MX6DL_PAD_SD3_CLK__USDHC3_CLK,
	MX6DL_PAD_SD3_CMD__USDHC3_CMD,
	MX6DL_PAD_SD3_DAT0__USDHC3_DAT0,
	MX6DL_PAD_SD3_DAT1__USDHC3_DAT1,
	MX6DL_PAD_SD3_DAT2__USDHC3_DAT2,
	MX6DL_PAD_SD3_DAT3__USDHC3_DAT3,
	MX6DL_PAD_SD3_DAT4__USDHC3_DAT4,
	MX6DL_PAD_SD3_DAT5__USDHC3_DAT5,
	MX6DL_PAD_SD3_DAT6__USDHC3_DAT6,
	MX6DL_PAD_SD3_DAT7__USDHC3_DAT7,
};

iomux_v3_cfg_t usdhc4_pads[] = {
	MX6DL_PAD_SD4_CLK__USDHC4_CLK,
	MX6DL_PAD_SD4_CMD__USDHC4_CMD,
	MX6DL_PAD_SD4_DAT0__USDHC4_DAT0,
	MX6DL_PAD_SD4_DAT1__USDHC4_DAT1,
	MX6DL_PAD_SD4_DAT2__USDHC4_DAT2,
	MX6DL_PAD_SD4_DAT3__USDHC4_DAT3,
	MX6DL_PAD_SD4_DAT4__USDHC4_DAT4,
	MX6DL_PAD_SD4_DAT5__USDHC4_DAT5,
	MX6DL_PAD_SD4_DAT6__USDHC4_DAT6,
	MX6DL_PAD_SD4_DAT7__USDHC4_DAT7,
};
#endif

int usdhc_gpio_init(bd_t *bis)
{
	s32 status = 0;
	u32 index = 0;

	for (index = 0; index < CONFIG_SYS_FSL_USDHC_NUM;
		++index) {
		switch (index) {
		case 0:
			//don't init sdhc1 since we use it as non usdhc
			/*
			mxc_iomux_v3_setup_multiple_pads(usdhc1_pads,
				sizeof(usdhc1_pads) /
				sizeof(usdhc1_pads[0]));
			*/
			break;
		case 1:
			mxc_iomux_v3_setup_multiple_pads(usdhc2_pads,
				sizeof(usdhc2_pads) /
				sizeof(usdhc2_pads[0]));
			break;
		case 2:
			mxc_iomux_v3_setup_multiple_pads(usdhc3_pads,
				sizeof(usdhc3_pads) /
				sizeof(usdhc3_pads[0]));
			break;
		case 3:
			mxc_iomux_v3_setup_multiple_pads(usdhc4_pads,
				sizeof(usdhc4_pads) /
				sizeof(usdhc4_pads[0]));
			break;
		default:
			printf("Warning: you configured more USDHC controllers"
				"(%d) then supported by the board (%d)\n",
				index+1, CONFIG_SYS_FSL_USDHC_NUM);
			return status;
		}
		status |= fsl_esdhc_initialize(bis, &usdhc_cfg[index]);
	}

	return status;
}

int board_mmc_init(bd_t *bis)
{
	if (!usdhc_gpio_init(bis))
		return 0;
	else
		return -1;
}


/* For DDR mode operation, provide target delay parameter for each SD port.
 * Use cfg->esdhc_base to distinguish the SD port #. The delay for each port
 * is dependent on signal layout for that particular port.  If the following
 * CONFIG is not defined, then the default target delay value will be used.
 */
#ifdef CONFIG_GET_DDR_TARGET_DELAY
u32 get_ddr_delay(struct fsl_esdhc_cfg *cfg)
{
	/* No delay required on SABRESD board SD ports */
	return 0;
}
#endif

#endif

static void
msleep(int count)
{
	int i;
	for (i = 0; i < count; i++)
		udelay(1000);
}
#if MIPI_DSI

#define LCD_PWR IMX_GPIO_NR(2,6)
#define LCD_RESET IMX_GPIO_NR(6,16)
#define LCD_BL_PWR IMX_GPIO_NR(4,15)

static void panel_power_on(int on)
{
	mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_NANDF_D6__GPIO_2_6));
	mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_NANDF_CS3__GPIO_6_16));
	if(on){
	//LCD PWR
	gpio_direction_output(LCD_PWR, 0);
	
	//LCD_RST_B
	gpio_direction_output(LCD_RESET, 0);
	msleep(10);
	gpio_direction_output(LCD_RESET, 1);
	msleep(120);
	}else {	
		gpio_direction_output(LCD_PWR, 1);
		gpio_direction_output(LCD_RESET, 0);		
	}

}

static void panel_bl_on(int on){
	//LCD_BL_PWR_EN	
	mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_KEY_ROW4__GPIO_4_15));
	/*
	reg = readl(GPIO4_BASE_ADDR + GPIO_GDIR);
	reg |= (1 << 15);
	writel(reg, GPIO4_BASE_ADDR + GPIO_GDIR);
	reg = readl(GPIO4_BASE_ADDR + GPIO_DR);
	reg |= (1 << 15);
	writel(reg, GPIO4_BASE_ADDR + GPIO_DR);
	*/
	gpio_direction_output(LCD_BL_PWR, on?1:0);
}

struct _mipi_dsi_phy_pll_clk {
	u32		max_phy_clk;
	u32		config;
};

/* configure data for DPHY PLL 27M reference clk out */
static const struct _mipi_dsi_phy_pll_clk mipi_dsi_phy_pll_clk_table[] = {
	{1000, 0x74}, /*  950-1000MHz	*/
	{950,  0x54}, /*  900-950Mhz	*/
	{900,  0x34}, /*  850-900Mhz	*/
	{850,  0x14}, /*  800-850MHz	*/
	{800,  0x32}, /*  750-800MHz	*/
	{750,  0x12}, /*  700-750Mhz	*/
	{700,  0x30}, /*  650-700Mhz	*/
	{650,  0x10}, /*  600-650MHz	*/
	{600,  0x2e}, /*  550-600MHz	*/
	{550,  0x0e}, /*  500-550Mhz	*/
	{500,  0x2c}, /*  450-500Mhz	*/
	{450,  0x0c}, /*  400-450MHz	*/
	{400,  0x4a}, /*  360-400MHz	*/
	{360,  0x2a}, /*  330-360Mhz	*/
	{330,  0x48}, /*  300-330Mhz	*/
	{300,  0x28}, /*  270-300MHz	*/
	{270,  0x08}, /*  250-270MHz	*/
	{250,  0x46}, /*  240-250Mhz	*/
	{240,  0x26}, /*  210-240Mhz	*/
	{210,  0x06}, /*  200-210MHz	*/
	{200,  0x44}, /*  180-200MHz	*/
	{180,  0x24}, /*  160-180MHz	*/
	{160,  0x04}, /*  150-160MHz	*/
};
static u32 cal_mipi_phy_pll(u32 max_phy_clk){
	int i;
	u32 pll;
	for (i = 0; i < ARRAY_SIZE(mipi_dsi_phy_pll_clk_table); i++) {
		if (mipi_dsi_phy_pll_clk_table[i].max_phy_clk <
				max_phy_clk)
			break;
	}
	if ((i == ARRAY_SIZE(mipi_dsi_phy_pll_clk_table)) ||
		(max_phy_clk >
			mipi_dsi_phy_pll_clk_table[0].max_phy_clk)) {
		printf("failed to find data in"
				"mipi_dsi_phy_pll_clk_table.\n");
		return -EINVAL;
	}
	pll =  mipi_dsi_phy_pll_clk_table[--i].config;
	
	//printf("dphy_pll_config:0x%x.\n", pll);

	return pll;
}
static void mipi_clk_enable(int enable)
{
	u32 reg;
	reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR3);
	if(enable)
		reg |= (MXC_CCM_CCGR_CG_MASK<<MXC_CCM_CCGR3_CG8_OFFSET);
	else		
		reg &= ~(MXC_CCM_CCGR_CG_MASK<<MXC_CCM_CCGR3_CG8_OFFSET);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR3);
}

static void mipi_dsi_dphy_init(u32 cmd, u32 data)
{
	u32 val;
	u32 timeout = 0;

	mipi_dsi_write_register(MIPI_DSI_PHY_IF_CTRL,
			DSI_PHY_IF_CTRL_RESET);
	mipi_dsi_write_register(MIPI_DSI_PWR_UP, DSI_PWRUP_POWERUP);

	mipi_dsi_write_register(MIPI_DSI_PHY_TST_CTRL0, 0);
	mipi_dsi_write_register(MIPI_DSI_PHY_TST_CTRL1,
		(0x10000 | cmd));
	mipi_dsi_write_register(MIPI_DSI_PHY_TST_CTRL0, 2);
	mipi_dsi_write_register(MIPI_DSI_PHY_TST_CTRL0, 0);
	mipi_dsi_write_register(MIPI_DSI_PHY_TST_CTRL1, (0 | data));
	mipi_dsi_write_register(MIPI_DSI_PHY_TST_CTRL0, 2);
	mipi_dsi_write_register(MIPI_DSI_PHY_TST_CTRL0, 0);
	val = DSI_PHY_RSTZ_EN_CLK | DSI_PHY_RSTZ_DISABLE_RST |
			DSI_PHY_RSTZ_DISABLE_SHUTDOWN;
	mipi_dsi_write_register(MIPI_DSI_PHY_RSTZ, val);

	mipi_dsi_read_register( MIPI_DSI_PHY_STATUS, &val);
	while ((val & DSI_PHY_STATUS_LOCK) != DSI_PHY_STATUS_LOCK) {
		msleep(1);
		timeout++;
		if (timeout == MIPI_DSI_PHY_TIMEOUT) {
			printf("Error: phy lock timeout!\n");
			break;
		}
		mipi_dsi_read_register(MIPI_DSI_PHY_STATUS, &val);
	}
	timeout = 0;
	while ((val & DSI_PHY_STATUS_STOPSTATE_CLK_LANE) !=
			DSI_PHY_STATUS_STOPSTATE_CLK_LANE) {
		msleep(1);
		timeout++;
		if (timeout == MIPI_DSI_PHY_TIMEOUT) {
			printf("Error: phy lock lane timeout!\n");
			break;
		}
		mipi_dsi_read_register(MIPI_DSI_PHY_STATUS, &val);
	}
}

static void mipi_dsi_enable_controller(void)
{
	u32		val;
	u32		lane_byte_clk_period;
	/* config MIPI DSI controller*/
	mipi_dsi_write_register(MIPI_DSI_PWR_UP,DSI_PWRUP_RESET);
	mipi_dsi_write_register(MIPI_DSI_PHY_RSTZ,DSI_PHY_RSTZ_RST);
	mipi_dsi_write_register(MIPI_DSI_CLKMGR_CFG,DSI_CLKMGR_CFG_CLK_DIV);
	val=0;
	if (!(mipi_dsi.sync & FB_SYNC_VERT_HIGH_ACT))
			val = DSI_DPI_CFG_VSYNC_ACT_LOW;
	if (!(mipi_dsi.sync & FB_SYNC_HOR_HIGH_ACT))
			val |= DSI_DPI_CFG_HSYNC_ACT_LOW;
	if ((mipi_dsi.sync & FB_SYNC_OE_LOW_ACT))
			val |= DSI_DPI_CFG_DATAEN_ACT_LOW;
	if (MIPI_RGB666_LOOSELY == mipilcd_config.dpi_fmt)
			val |= DSI_DPI_CFG_EN18LOOSELY;
	val |= (mipilcd_config.dpi_fmt & DSI_DPI_CFG_COLORCODE_MASK)
				<< DSI_DPI_CFG_COLORCODE_SHIFT;
	val |= (mipilcd_config.virtual_ch & DSI_DPI_CFG_VID_MASK)
				<< DSI_DPI_CFG_VID_SHIFT;
	mipi_dsi_write_register(MIPI_DSI_DPI_CFG,val);

	val = DSI_PCKHDL_CFG_EN_BTA |
			DSI_PCKHDL_CFG_EN_ECC_RX |
			DSI_PCKHDL_CFG_EN_CRC_RX;
	
	mipi_dsi_write_register(MIPI_DSI_PCKHDL_CFG,val);

	val = (mipi_dsi.xres & DSI_VID_PKT_CFG_VID_PKT_SZ_MASK)
				<< DSI_VID_PKT_CFG_VID_PKT_SZ_SHIFT;
	val |= (NUMBER_OF_CHUNKS & DSI_VID_PKT_CFG_NUM_CHUNKS_MASK)
				<< DSI_VID_PKT_CFG_NUM_CHUNKS_SHIFT;
	val |= (NULL_PKT_SIZE & DSI_VID_PKT_CFG_NULL_PKT_SZ_MASK)
				<< DSI_VID_PKT_CFG_NULL_PKT_SZ_SHIFT;
	mipi_dsi_write_register(MIPI_DSI_VID_PKT_CFG,val);

	mipi_dsi_write_register(MIPI_DSI_CMD_MODE_CFG,MIPI_DSI_CMD_MODE_CFG_EN_LOWPOWER);

	lane_byte_clk_period = NS2PS_RATIO /
				(mipilcd_config.max_phy_clk / BITS_PER_BYTE);
	val  = ROUND_UP(mipi_dsi.hsync_len * mipi_dsi.pixclock /
				NS2PS_RATIO / lane_byte_clk_period)
				<< DSI_TME_LINE_CFG_HSA_TIME_SHIFT;
	val |= ROUND_UP(mipi_dsi.left_margin * mipi_dsi.pixclock /
				NS2PS_RATIO / lane_byte_clk_period)
				<< DSI_TME_LINE_CFG_HBP_TIME_SHIFT;
	val |= ROUND_UP((mipi_dsi.left_margin + mipi_dsi.right_margin +
				mipi_dsi.hsync_len + mipi_dsi.xres) * mipi_dsi.pixclock
				/ NS2PS_RATIO / lane_byte_clk_period)
				<< DSI_TME_LINE_CFG_HLINE_TIME_SHIFT;
	mipi_dsi_write_register(MIPI_DSI_TMR_LINE_CFG,val);

	val = ((mipi_dsi.vsync_len & DSI_VTIMING_CFG_VSA_LINES_MASK)
					<< DSI_VTIMING_CFG_VSA_LINES_SHIFT);
	val |= ((mipi_dsi.upper_margin & DSI_VTIMING_CFG_VBP_LINES_MASK)
				<< DSI_VTIMING_CFG_VBP_LINES_SHIFT);
	val |= ((mipi_dsi.lower_margin & DSI_VTIMING_CFG_VFP_LINES_MASK)
				<< DSI_VTIMING_CFG_VFP_LINES_SHIFT);
	val |= ((mipi_dsi.yres & DSI_VTIMING_CFG_V_ACT_LINES_MASK)
				<< DSI_VTIMING_CFG_V_ACT_LINES_SHIFT);
	mipi_dsi_write_register(MIPI_DSI_VTIMING_CFG,val);

	val = ((PHY_BTA_MAXTIME & DSI_PHY_TMR_CFG_BTA_TIME_MASK)
			<< DSI_PHY_TMR_CFG_BTA_TIME_SHIFT);
	val |= ((PHY_LP2HS_MAXTIME & DSI_PHY_TMR_CFG_LP2HS_TIME_MASK)
			<< DSI_PHY_TMR_CFG_LP2HS_TIME_SHIFT);
	val |= ((PHY_HS2LP_MAXTIME & DSI_PHY_TMR_CFG_HS2LP_TIME_MASK)
			<< DSI_PHY_TMR_CFG_HS2LP_TIME_SHIFT);
	mipi_dsi_write_register(MIPI_DSI_PHY_TMR_CFG,val);

	val = (((mipilcd_config.data_lane_num - 1) &
		DSI_PHY_IF_CFG_N_LANES_MASK)
		<< DSI_PHY_IF_CFG_N_LANES_SHIFT);
	val |= ((PHY_STOP_WAIT_TIME & DSI_PHY_IF_CFG_WAIT_TIME_MASK)
			<< DSI_PHY_IF_CFG_WAIT_TIME_SHIFT);
	mipi_dsi_write_register(MIPI_DSI_PHY_IF_CFG,val);

	mipi_dsi_read_register(MIPI_DSI_ERROR_ST0,&val);
	mipi_dsi_read_register(MIPI_DSI_ERROR_ST1,&val);
	mipi_dsi_write_register(MIPI_DSI_ERROR_MSK0,0);
	mipi_dsi_write_register(MIPI_DSI_ERROR_MSK1,0);

	mipi_dsi_dphy_init(DSI_PHY_CLK_INIT_COMMAND,
				cal_mipi_phy_pll(mipilcd_config.max_phy_clk));

}
static void mipi_dsi_disable_controller(void)
{
	mipi_dsi_write_register(MIPI_DSI_PHY_IF_CTRL,
			DSI_PHY_IF_CTRL_RESET);
	mipi_dsi_write_register(MIPI_DSI_PWR_UP, DSI_PWRUP_RESET);
	mipi_dsi_write_register(MIPI_DSI_PHY_RSTZ, DSI_PHY_RSTZ_RST);
}

static void mipi_dsi_set_mode(int cmd_mode)
{
	u32	val;

	if (cmd_mode) {
		mipi_dsi_write_register(MIPI_DSI_PWR_UP,
			DSI_PWRUP_RESET);
		mipi_dsi_read_register(MIPI_DSI_CMD_MODE_CFG, &val);
		val |= MIPI_DSI_CMD_MODE_CFG_EN_CMD_MODE;
		mipi_dsi_write_register(MIPI_DSI_CMD_MODE_CFG, val);
		mipi_dsi_write_register(MIPI_DSI_VID_MODE_CFG, 0);
		mipi_dsi_write_register(MIPI_DSI_PWR_UP,
			DSI_PWRUP_POWERUP);
	} else {
		mipi_dsi_write_register(MIPI_DSI_PWR_UP,
			DSI_PWRUP_RESET);
		 /* Disable Command mode when tranfering video data */
		mipi_dsi_read_register(MIPI_DSI_CMD_MODE_CFG, &val);
		val &= ~MIPI_DSI_CMD_MODE_CFG_EN_CMD_MODE;
		mipi_dsi_write_register(MIPI_DSI_CMD_MODE_CFG, val);
		val = DSI_VID_MODE_CFG_EN | DSI_VID_MODE_CFG_EN_BURSTMODE |
				DSI_VID_MODE_CFG_EN_LP_MODE;
		mipi_dsi_write_register(MIPI_DSI_VID_MODE_CFG, val);
		mipi_dsi_write_register( MIPI_DSI_PWR_UP,
			DSI_PWRUP_POWERUP);
		mipi_dsi_write_register(MIPI_DSI_PHY_IF_CTRL,
				DSI_PHY_IF_CTRL_TX_REQ_CLK_HS);
	}
}

static char* findtoken(char* findstring,char token){
	char* s=findstring;
	while(*s!='\0'){
		if(token==*s) return s;
		s++;
	}
	return NULL;
}
static void apply_mipi_patch(void){
	char* env = getenv("dispformat");
	if(env){
		int pclk,vs,vbp,vfp,hs,hbp,hfp,phyclk;
		char fmt[128];
		int length=strlen(env);
		char* s;
		char* ds= strdup(env);
		char* token;
		pclk=vs=vbp=vfp=hs=hbp=hfp=phyclk=0;
		s = ds;
		if((token=findtoken(s,','))){
			*token='\0';
			pclk = simple_strtoul(s, NULL, 16);
			printf("find pclk=%d\n",pclk);
			s = ++token;
		}
		if((*s!='\0')&&(token=findtoken(s,','))){
			*token='\0';
			vs = simple_strtoul(s, NULL, 16);
			printf("find vs=%d\n",vs);
			s = ++token;
		}
		if((*s!='\0')&&(token=findtoken(s,','))){
			*token='\0';
			vbp = simple_strtoul(s, NULL, 16);
			printf("find vbp=%d\n",vbp);
			s = ++token;
		}
		if((*s!='\0')&&(token=findtoken(s,','))){
			*token='\0';
			vfp = simple_strtoul(s, NULL, 16);
			printf("find vfp=%d\n",vfp);
			s = ++token;
		}
		if((*s!='\0')&&(token=findtoken(s,','))){
			*token='\0';
			hs = simple_strtoul(s, NULL, 16);
			printf("find hs=%d\n",hs);
			s = ++token;
		}
		if((*s!='\0')&&(token=findtoken(s,','))){
			*token='\0';
			hbp = simple_strtoul(s, NULL, 16);
			printf("find hbp=%d\n",hbp);
			s = ++token;
		}
		if((*s!='\0')&&(token=findtoken(s,','))){
			*token='\0';
			hfp = simple_strtoul(s, NULL, 16);
			printf("find hfp=%d\n",hfp);
			s = ++token;
		}
		if((*s!='\0')&&(token=findtoken(s,','))){
			*token='\0';
			phyclk = simple_strtoul(s, NULL, 16);
			printf("find phyclk=%d\n",phyclk);
			s = ++token;
		}

		mipi_dsi.pixclock = pclk;
		mipi_dsi.vsync_len = vs;
		mipi_dsi.upper_margin = vbp;
		mipi_dsi.lower_margin = vfp;
		mipi_dsi.hsync_len = hs;
		mipi_dsi.right_margin = hbp;
		mipi_dsi.left_margin = hfp;
		mipilcd_config.max_phy_clk = phyclk;

	}
}

static int do_dispformat(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	apply_mipi_patch();
	return 0;
}

U_BOOT_CMD(
        dispformat, CONFIG_SYS_MAXARGS, 0, do_dispformat, NULL, NULL
);

static void mipi_dsi_enable(void)
{
	struct mipipanel_info* pi;

	int err;
	mipi_clk_enable(1);
	msleep(5);
	mipi_dsi_enable_controller();	
	//msleep(100);
	//fixed lcd panel type
	setenv("panel","SI-QHD");
	err = mipi_panel_detect(&pi);
	if (err < 0) {
		printf("init default lcd panel\n");
		mipi_panel_init_def();
	}else {
		panel_name = pi->name;
		//FIXME: reinit MIPI controller??
		memcpy(&mipi_dsi,pi->vm,sizeof(mipi_dsi));
		memcpy(&mipilcd_config,pi->phy,sizeof(mipilcd_config));
		//panel_power_on(0);
		//panel_power_on(1);

		apply_mipi_patch();

		panel_info_init();
		mipi_dsi_disable_controller();
		//msleep(5);
		mipi_dsi_enable_controller();
		pi->init();
	}
	
    msleep(5);
	mipi_dsi_set_mode(0);

}
#endif

/*add by allenyao*/
#if  defined(CONFIG_LCD)||defined(CONFIG_VIDEO)
void lcd_enable(void)
{
	char *s;
	int ret;
	unsigned int reg;

	s = getenv("lvds_num");
	di = simple_strtol(s, NULL, 10);

	/*
	* hw_rev 2: IPUV3DEX
	* hw_rev 3: IPUV3M
	* hw_rev 4: IPUV3H
	*/
	g_ipu_hw_rev = IPUV3_HW_REV_IPUV3H;

	panel_bl_on(0);


	/* PWM backlight */
	mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_SD1_DAT3__PWM1_PWMO));

	//def brightness is 120 on android
	imx_pwm_config(pwm0, 24000, 50000);
	imx_pwm_enable(pwm0);

	/* Disable ipu1_clk/ipu1_di_clk_x/ldb_dix_clk/mipi_clk. */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR3);
	reg &= ~(0x3F03F);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR3);


#if defined CONFIG_MX6Q
	/*
	 * Align IPU1 HSP clock and IPU1 DIx pixel clock
	 * with kernel setting to avoid screen flick when
	 * booting into kernel. Developer should change
	 * the relevant setting if kernel setting changes.
	 * IPU1 HSP clock tree:
	 * osc_clk(24M)->pll2_528_bus_main_clk(528M)->
	 * periph_clk(528M)->mmdc_ch0_axi_clk(528M)->
	 * ipu1_clk(264M)
	 */
	/* pll2_528_bus_main_clk */
	/* divider */
	writel(0x1, ANATOP_BASE_ADDR + 0x34);

	/* periph_clk */
	/* source */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CBCMR);
	reg &= ~(0x3 << 18);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CBCMR);

	reg = readl(CCM_BASE_ADDR + CLKCTL_CBCDR);
	reg &= ~(0x1 << 25);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CBCDR);

	/*
	 * Check PERIPH_CLK_SEL_BUSY in
	 * MXC_CCM_CDHIPR register.
	 */
	do {
		udelay(5);
		reg = readl(CCM_BASE_ADDR + CLKCTL_CDHIPR);
	} while (reg & (0x1 << 5));

	/* mmdc_ch0_axi_clk */
	/* divider */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CBCDR);
	reg &= ~(0x7 << 19);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CBCDR);

	/*
	 * Check MMDC_CH0PODF_BUSY in
	 * MXC_CCM_CDHIPR register.
	 */
	do {
		udelay(5);
		reg = readl(CCM_BASE_ADDR + CLKCTL_CDHIPR);
	} while (reg & (0x1 << 4));

	/* ipu1_clk */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CSCDR3);
	/* source */
	reg &= ~(0x3 << 9);
	/* divider */
	reg &= ~(0x7 << 11);
	reg |= (0x1 << 11);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CSCDR3);

#if MIPI_DSI
	/* configs for mipi_pllref_clk, mipi_pllref_clk <- pll3_PFD_540M*/
	/* pll3_usb_otg_main_clk */
	/* divider */
	writel(0x3, ANATOP_BASE_ADDR + 0x18); /* 24*20 = 480M */
	/* pll3_pfd_540M */
	/* divider */
	writel(0x3F << 8, ANATOP_BASE_ADDR + 0xF8);
	writel(0x10 << 8, ANATOP_BASE_ADDR + 0xF4); /* 480*18/16 = 540M */
	/* enable */
	writel(0x1 << 15, ANATOP_BASE_ADDR + 0xF8);//change by allenyao
#else
	/*
	 * ipu1_pixel_clk_x clock tree:
	 * osc_clk(24M)->pll2_528_bus_main_clk(528M)->
	 * pll2_pfd_352M(452.57M)->ldb_dix_clk(64.65M)->
	 * ipu1_di_clk_x(64.65M)->ipu1_pixel_clk_x(64.65M)
	 */
	/* pll2_pfd_352M */
	/* disable */
	writel(0x1 << 7, ANATOP_BASE_ADDR + 0x104);
	/* divider */
	writel(0x3F, ANATOP_BASE_ADDR + 0x108);
	writel(0x15, ANATOP_BASE_ADDR + 0x104);

	/* ldb_dix_clk */
	/* source */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CS2CDR);
	reg &= ~(0x3F << 9);
	reg |= (0x9 << 9);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CS2CDR);
	/* divider */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CSCMR2);
	reg |= (0x3 << 10);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CSCMR2);

	/* pll2_pfd_352M */
	/* enable after ldb_dix_clk source is set */
	writel(0x1 << 7, ANATOP_BASE_ADDR + 0x108);

	/* ipu1_di_clk_x */
	/* source */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CHSCCDR);
	reg &= ~0xE07;
	reg |= 0x803;
	writel(reg, CCM_BASE_ADDR + CLKCTL_CHSCCDR);
#endif
#elif defined CONFIG_MX6DL /* CONFIG_MX6Q */
	/*
	 * IPU1 HSP clock tree:
	 * osc_clk(24M)->pll3_usb_otg_main_clk(480M)->
	 * pll3_pfd_540M(540M)->ipu1_clk(270M)
	 */
	/* pll3_usb_otg_main_clk */
	/* divider */
	writel(0x3, ANATOP_BASE_ADDR + 0x18);

	/* pll3_pfd_540M */
	/* divider */
	writel(0x3F << 8, ANATOP_BASE_ADDR + 0xF8);
	writel(0x10 << 8, ANATOP_BASE_ADDR + 0xF4);
	/* enable */
	writel(0x1 << 15, ANATOP_BASE_ADDR + 0xF8);

	/* ipu1_clk */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CSCDR3);
	/* source */
	reg |= (0x3 << 9);
	/* divider */
	reg &= ~(0x7 << 11);
	reg |= (0x1 << 11);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CSCDR3);

	/*
	 * ipu1_pixel_clk_x clock tree:
	 * osc_clk(24M)->pll2_528_bus_main_clk(528M)->
	 * pll2_pfd_352M(452.57M)->ldb_dix_clk(64.65M)->
	 * ipu1_di_clk_x(64.65M)->ipu1_pixel_clk_x(64.65M)
	 */
	/* pll2_528_bus_main_clk */
	/* divider */
	writel(0x1, ANATOP_BASE_ADDR + 0x34);

	/* pll2_pfd_352M */
	/* disable */
	writel(0x1 << 7, ANATOP_BASE_ADDR + 0x104);
	/* divider */
	writel(0x3F, ANATOP_BASE_ADDR + 0x108);
	writel(0x15, ANATOP_BASE_ADDR + 0x104);

	/* ldb_dix_clk */
	/* source */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CS2CDR);
	reg &= ~(0x3F << 9);
	reg |= (0x9 << 9);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CS2CDR);
	/* divider */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CSCMR2);
	reg |= (0x3 << 10);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CSCMR2);

	/* pll2_pfd_352M */
	/* enable after ldb_dix_clk source is set */
	writel(0x1 << 7, ANATOP_BASE_ADDR + 0x108);

	/* ipu1_di_clk_x */
	/* source */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CHSCCDR);
	reg &= ~0xE07;
	reg |= 0x803;
	writel(reg, CCM_BASE_ADDR + CLKCTL_CHSCCDR);
#endif	/* CONFIG_MX6DL */

	/* Enable ipu1/ipu1_dix/ldb_dix clocks. */
	if (di == 1) {
		reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR3);
		reg |= 0x3C033;
		writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR3);
	} else {
		reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR3);
		reg |= 0x3300F;
		writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR3);
	}

	/*add by allenyao*/
#if MIPI_DSI
	#if defined CONFIG_MX6Q
	ret = ipuv3_fb_init(&mipi_dsi, di, IPU_PIX_FMT_RGB24,
			DI_PCLK_PLL3, 26400000);
	#elif defined CONFIG_MX6DL
	ret = ipuv3_fb_init(&mipi_dsi, di, IPU_PIX_FMT_RGB24,
			DI_PCLK_PLL3, 0);
	#endif
#else
	ret = ipuv3_fb_init(&lvds_wvga, di, IPU_PIX_FMT_RGB24,
			DI_PCLK_LDB, 65000000);
#endif
	if (ret)
		puts("LCD cannot be configured\n");
	/*add by allenyao*/
#if MIPI_DSI
	/* mipi source mux to IPU1 DI1 */
	if (di == 1) {
		reg = readl(IOMUXC_BASE_ADDR + 0xC);
		reg &= ~(0x00000030);
		reg |= 0x00000010;
		writel(reg, IOMUXC_BASE_ADDR + 0xC);
	}
	/* mipi source mux to IPU1 DI0 */
	if (di == 0) {
		reg = readl(IOMUXC_BASE_ADDR + 0xC);
		reg &= ~(0x0000030);
		writel(reg, IOMUXC_BASE_ADDR + 0xC);
	}
	panel_power_on(1);
	mipi_dsi_enable();
#else //ldb
	/*
	 * LVDS0 mux to IPU1 DI0.
	 * LVDS1 mux to IPU1 DI1.
	 */
	reg = readl(IOMUXC_BASE_ADDR + 0xC);
	reg &= ~(0x000003C0);
	reg |= 0x00000100;
	writel(reg, IOMUXC_BASE_ADDR + 0xC);

	if (di == 1)
		writel(0x40C, IOMUXC_BASE_ADDR + 0x8);
	else
		writel(0x201, IOMUXC_BASE_ADDR + 0x8);

#endif


}
#endif

#ifdef CONFIG_VIDEO_MX5
#if MIPI_DSI
void panel_info_init(void)
{
	panel_info.vl_bpix = LCD_BPP;
	panel_info.vl_col = mipi_dsi.xres;
	panel_info.vl_row = mipi_dsi.yres;
	panel_info.cmap = colormap;
}
#else
void panel_info_init(void)
{
	panel_info.vl_bpix = LCD_BPP;
	panel_info.vl_col = lvds_wvga.xres;
	panel_info.vl_row = lvds_wvga.yres;
	panel_info.cmap = colormap;
}
#endif
#endif

extern u32 get_mcu_main_clk(void);

#ifdef CONFIG_LCD_INFO
void lcd_print_size (phys_size_t size, const char *s)
{
	ulong m = 0, n;
	phys_size_t d = 1 << 30;		/* 1 GB */
	char  c = 'G';

	if (size < d) {			/* try MB */
		c = 'M';
		d = 1 << 20;
		if (size < d) {		/* print in kB */
			c = 'k';
			d = 1 << 10;
		}
	}

	n = size / d;

	/* If there's a remainder, deal with it */
	if(size % d) {
		m = (10 * (size - (n * d)) + (d / 2) ) / d;

		if (m >= 10) {
			m -= 10;
			n += 1;
		}
	}

	lcd_printf ("%2ld", n);
	if (m) {
		lcd_printf (".%ld", m);
	}
	lcd_printf (" %cB%s", c, s);
}



void lcd_show_board_info(void)
{
        ulong dram_size, nand_size, flash_size, dataflash_size;
        int i;
        char temp[32];

        lcd_printf ("%s\n", U_BOOT_VERSION);
        lcd_printf ("(C) 2013 Quester Technology\n");
        lcd_printf ("raymond.wang@quester.com.cn\n");
		lcd_printf("CPU: Freescale i.MX6 family TO%d.%d at %d MHz\n",
			   (get_board_rev() & 0xFF) >> 4,
			   (get_board_rev() & 0xF),
			get_mcu_main_clk() / 1000000);
		
		
		for (dram_size=0,i=0; i<CONFIG_NR_DRAM_BANKS; i++) {
			dram_size += gd->bd->bi_dram[i].size;
		}
		lcd_printf("DRAM:  ");
		lcd_print_size(dram_size, "\n");	
		//fixme shor MMC size

}
#endif
#ifdef CONFIG_CONSOLE_EXTRA_INFO
void video_get_info_str (int line_number, char *info)
{
        char str[128];		
        ulong dram_size;
        int i;

        if(line_number == 1) strcpy(info,"(C) 2014 XingHuo Engineering Inc,.");
		else if(line_number == 2) {			
			strcpy(info,"raymond1860@gmail.com");
		}
		else if(line_number == 3) {			
			sprintf(str,"CPU: Freescale i.MX6 family TO%d.%d at %d MHz\n",
			   (get_board_rev() & 0xFF) >> 4,
			   (get_board_rev() & 0xF),
			get_mcu_main_clk() / 1000000);
			strcpy(info,str);
		}else if(line_number == 4) {			
			for (dram_size=0,i=0; i<CONFIG_NR_DRAM_BANKS; i++) {
				dram_size += gd->bd->bi_dram[i].size;
			}
			sprintf(str,"DRAM:  %2ldMB",dram_size/1048576);
			strcpy(info,str);
		}else {
		info [0] = '\0';
		}
}
#endif

#ifdef CONFIG_SPLASH_SCREEN

void setup_splash_image(void)
{
	char *s;
	ulong addr;
	unsigned long size=0;
	unsigned long logo;
	bmp_t splashbmp;
	logo = CONFIG_SYS_LOAD_ADDR;	
	if(eBootModeCharger==android_bootmode){
		//in charger-only mode,we don't want to display any content in bootloader step
		//setenv("splashimage",NULL);
		
		return;
	}
	run_command("mmc dev 3",0);
	if(!bmp_manager_getbmp("bmp.splash",&splashbmp)){
		size = splashbmp.size;
		bmp_manager_readbmp("bmp.splash",logo,0x20000000);
	}
	
	if(!size)
		setenv("splashimage",0);
	else
		setenv("splashimage","0x30000000");//copy from config file,to avoid splashimage env is cleared
	
	s = getenv("splashimage");

	if (s != NULL) {
		addr = simple_strtoul(s, NULL, 16);

#if defined(CONFIG_ARCH_MMU)
		addr = ioremap_nocache(iomem_to_phys(addr),
				fsl_bmp_reversed_600x400_size);
#endif
		if(size)
			memcpy((char *)addr, (char *)logo,size);
	}
}
#endif

int board_init(void)
{
/* need set Power Supply Glitch to 0x41736166
*and need clear Power supply Glitch Detect bit
* when POR or reboot or power on Otherwise system
*could not be power off anymore*/
	u32 reg;
	writel(0x41736166, SNVS_BASE_ADDR + 0x64);/*set LPPGDR*/
	udelay(10);
	reg = readl(SNVS_BASE_ADDR + 0x4c);
	reg |= (1 << 3);
	writel(reg, SNVS_BASE_ADDR + 0x4c);/*clear LPSR*/

	/* Toming: fixed self power on randomly */
	reg = readl(SNVS_BASE_ADDR + 0x38);
	reg &= ~(3 << 18); reg |= (2 << 18);
	writel(reg, SNVS_BASE_ADDR + 0x38);
	
	mxc_iomux_v3_init((void *)IOMUXC_BASE_ADDR);
	setup_boot_device();
	fsl_set_system_rev();

	/* board id for linux */
	gd->bd->bi_arch_number = MACH_TYPE_MX6Q_TDH;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	setup_uart();


#ifdef CONFIG_VIDEO_MX5
	/* Enable lvds power */
	setup_lvds_poweron();

	panel_info_init();

	gd->fb_base = CONFIG_FB_BASE;
#ifdef CONFIG_ARCH_MMU
	gd->fb_base = ioremap_nocache(iomem_to_phys(gd->fb_base), 0);
#endif
	//memset(gd->fb_base,0,CONFIG_FB_SIZE);
#endif

	//shutdown motor
	mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_GPIO_4__GPIO_1_4));
	gpio_direction_output(TDH_MOTOR_PWR_EN, 1);


	return 0;
}

#ifdef ENABLE_KEYPAD_SHORTCUT
enum{
	eKeyPadBootModeNormal=0,
	eKeyPadBootModeRecovery,
	eKeyPadBootModeFastboot,
	eKeyPadBootModeAutoupdate,
	eKeyPadBootModeMfg,
	eKeyPadBootModeMax
};

static int stablize_keypad_bootmode(int data_io,int detect_state){
	int t=0;
	int state;
	detect_state=detect_state?1:0;
	//default stablize time is 3s
	do{
		sdelay(1);
		state=gpio_get_value(data_io);
		state=state?1:0;
		t++;
	}while(state==detect_state&&t<3);

	return (t>=3);
}
static int keypad_detect_bootmode(void){
	static int keypad_io_init=0;
	int detect_state;
	int keypad_bootmode = eKeyPadBootModeNormal;
	if(!keypad_io_init++){
		mxc_iomux_v3_setup_pad(NEW_PAD_CTRL(MX6X_IOMUX(PAD_KEY_COL4__GPIO_4_14),MX6_KEY_PAD_CTRL));
		mxc_iomux_v3_setup_pad(NEW_PAD_CTRL(MX6X_IOMUX(PAD_KEY_ROW2__GPIO_4_11),MX6_KEY_PAD_CTRL));
		mxc_iomux_v3_setup_pad(NEW_PAD_CTRL(MX6X_IOMUX(PAD_KEY_COL2__GPIO_4_10),MX6_KEY_PAD_CTRL));
		mxc_iomux_v3_setup_pad(NEW_PAD_CTRL(MX6X_IOMUX(PAD_NANDF_WP_B__GPIO_6_9),MX6_KEY_PAD_CTRL));
		mxc_iomux_v3_setup_pad(NEW_PAD_CTRL(MX6X_IOMUX(PAD_NANDF_RB0__GPIO_6_10),MX6_KEY_PAD_CTRL));
		gpio_direction_input(KEY_HOT_KEY1_IO);
		gpio_direction_input(KEY_HOT_KEY2_IO);
		gpio_direction_input(KEY_PTT_IO);
		gpio_direction_input(KEY_VOLMINUS_IO);
		gpio_direction_input(KEY_VOLPLUS_IO);
	}

	//check mode according to android priority
	if(!(detect_state=gpio_get_value(KEY_HOT_KEY1_IO))&&
		stablize_keypad_bootmode(KEY_HOT_KEY1_IO,detect_state)){
		printf("Recovery mode detected from keypad\n");
		keypad_bootmode = eKeyPadBootModeRecovery;
	}else if(!(detect_state=gpio_get_value(KEY_HOT_KEY2_IO))&&
		stablize_keypad_bootmode(KEY_HOT_KEY2_IO,detect_state)){
		printf("Fastboot mode detected from keypad\n");
		keypad_bootmode = eKeyPadBootModeFastboot;
	}else if(!(detect_state=gpio_get_value(KEY_VOLMINUS_IO))&&
		stablize_keypad_bootmode(KEY_VOLMINUS_IO,detect_state)){
		printf("Autoupdate mode detected from keypad\n");
		keypad_bootmode = eKeyPadBootModeAutoupdate;
	}else if(!(detect_state=gpio_get_value(KEY_VOLPLUS_IO))&&
		stablize_keypad_bootmode(KEY_VOLPLUS_IO,detect_state)){
		printf("Mfg mode detected from keypad\n");
		keypad_bootmode = eKeyPadBootModeMfg;
	}
	return keypad_bootmode;
}
#endif

#ifdef CONFIG_ANDROID_RECOVERY

int check_recovery_cmd_file(void)
{
	int recovery_switch=0;
	char *env;

	env = getenv("android_recovery_switch");
	if (!strcmp(env, "1")) {
		printf("Env recovery detected!\nEnter recovery mode!\n");
		recovery_switch++;
	}
	if(!recovery_switch){
		if(check_and_clean_recovery_flag()){
			printf("Linux recovery detected!\nEnter recovery mode!\n");
			recovery_switch++;
		}
	}

	#ifdef ENABLE_KEYPAD_SHORTCUT
	if(!recovery_switch){
		if(mx6_board_rev()!=BOARD_REV_INVALID){
			if(eKeyPadBootModeRecovery==keypad_detect_bootmode())
				recovery_switch++;
		}
	}
	#endif
	if(!recovery_switch){
		struct bootloader_message* boot = malloc(sizeof(struct bootloader_message));
		if(boot){
			memset(boot, 0, sizeof(boot));
			get_bootloader_message(boot);	// this may fail, leaving a zeroed structure
			if(!memcmp(boot->command,"boot-recovery",13)){
				printf("BCB recovery detected!\nEnter recovery mode!\n");
				recovery_switch++;
			}
			free(boot);
		}
	}
	
	return recovery_switch;
}
#endif

#ifdef CONFIG_FASTBOOT
int fastboot_mode_detect(void){
	int fastboot_switch = 0;

	#ifdef ENABLE_KEYPAD_SHORTCUT
	if(mx6_board_rev()!=BOARD_REV_INVALID){
		if(eKeyPadBootModeFastboot==keypad_detect_bootmode())
			fastboot_switch++;
	}
	#endif


	return fastboot_switch;
	
}

#endif

#ifdef CONFIG_AUTOUPDATER
int autoupdate_mode_detect(void){
	int autoupdate_switch = 0;

	#ifdef ENABLE_KEYPAD_SHORTCUT
	if(mx6_board_rev()!=BOARD_REV_INVALID){
		if(eKeyPadBootModeAutoupdate==keypad_detect_bootmode())
			autoupdate_switch++;
	}
	#endif

	return autoupdate_switch;
	
}
#endif

#ifdef BOARD_LATE_INIT
extern int mfg_check_and_clean_flag(void);
int board_late_init(void)
{
	int ret = 0;
	#if  defined(CONFIG_LCD)||defined(CONFIG_VIDEO)
	//turn back light
	panel_bl_on(1);
	#endif

	if(mfg_check_and_clean_flag()){
		run_command("download", 0);
	}
	#ifdef ENABLE_KEYPAD_SHORTCUT
	if(mx6_board_rev()!=BOARD_REV_INVALID){
		if(eKeyPadBootModeMfg==keypad_detect_bootmode())
		run_command("download", 0);
	}
	#endif
	return ret;
}
#endif

#ifdef CONFIG_ANDROID_BOOTMODE
char* append_commandline_extra(char* cmdline){
	const char* bootmode_cmdline[] ={
		"",
		"",
		" androidboot.mode=charger",
		"",
		"",
		" androidboot.mode=factory",
	};
	char* newcmdline = malloc(strlen(cmdline)+strlen(bootmode_cmdline[android_bootmode])+1);
	if(!newcmdline) return cmdline;
	strcpy(newcmdline,cmdline);
	if(android_bootmode<eBootModeMax&&strlen(bootmode_cmdline[android_bootmode])){
		strcat(newcmdline,bootmode_cmdline[android_bootmode]);
	}
	#ifdef CONFIG_CMD_IMXOTP
	{
		char buffer[128];
		unsigned int id0,id1;
		imx_otp_read_one_u32(2,&id1);
		imx_otp_read_one_u32(1,&id0);
		sprintf(buffer," androidboot.serialno=%08x%08x",id1,id0);
		strcat(newcmdline,buffer);
	}
	#endif
	if(panel_name){
		char buffer[128];
		sprintf(buffer," panel=%s",panel_name);
		strcat(newcmdline,buffer);
	}

	return newcmdline;
}
#endif



static const char* board_identity(void){
	static const char* ids[] =
	{
		"Unknown",
		"Sabre-AI (ARD)",
		"Smart Device (SD)",
		"Quick-Start Board (QSB",
		"SoloLite EVK (SL-EVK)",
		"Unknown",
		"HDMI Dongle",
		"Unknown",
		"Unknown",
		"Unknown",
		"SparkAuto",
		"QPad",	//0xB
		"TDH",//0xC
	};
	int id=mx6_board_id();
	
	if(id>0xC)
		id=0;
	return ids[id];
}

static const char* board_revision(void){
	static const char* revs[] =
	{
		"Unkown",
		"RevA",
		"RevB",
		"RevC",
		"RevD",
		"RevE",
		"Unknown",//0x5 maximum E is enough?
	};	
	int rev=mx6_board_rev();
	
	if(rev>5)
		rev=5;
	return revs[rev];
	
}

#ifdef CONFIG_CMD_BMP
static int draw_bmp(u8* bmp_image,int mode){
    int ret=0;
	bmp_image_t *bmp = (bmp_image_t *) bmp_image;
	int screen_w;
	int screen_h;
	int x,y;

	screen_w = panel_info.vl_col;
	screen_h = panel_info.vl_row;
	if (mode)
	{
		x = y = 0;
	}
	else
	{
        unsigned long width, height;
	    width = height = 0;
        if (((bmp->header.signature[0] == 'B') &&
              (bmp->header.signature[1] == 'M'))) {
            width = le32_to_cpu (bmp->header.width);
            height = le32_to_cpu (bmp->header.height);
        }
		
		//fix issue that height value may be negative 
		if(height<0) height=-height;
		//avoid bad bmp draw on screen
		if(width>screen_w||height>screen_h)
			return -1;

		x = max(0, (screen_w- width) / 2);
		y = max(0, (screen_h- height) / 2);
	}

	#if defined(CONFIG_LCD)
	extern int lcd_display_bitmap (ulong, int, int);

	ret = lcd_display_bitmap ((unsigned long)bmp_image, x, y);
	#elif defined(CONFIG_VIDEO)
	extern int video_display_bitmap (ulong, int, int);

	ret = video_display_bitmap ((unsigned long)bmp_image, x, y);
	#else
	# error lcd_draw_bmp() requires CONFIG_LCD or CONFIG_VIDEO
	#endif

	return ret;
}

static void lcd_screen_clear(void){
	memset((void*)gd->fb_base,0,panel_info.vl_col*panel_info.vl_row*4);
}
static int do_batterybmp(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char buffer[16];
    int lvl=0;
    u8* bmp = CONFIG_SYS_LOAD_ADDR;
	bmp_t batbmp;
    if(argc>1)
        lvl = simple_strtol(argv[1],0,10);
	sprintf(buffer,"bmp.bat%d",lvl);
	if(!bmp_manager_getbmp(buffer,&batbmp)){
		bmp_manager_readbmp(buffer,bmp,0x2000000);
	}else bmp = bmp_bat0;

	lcd_screen_clear();
	
	draw_bmp(bmp,0);
	
	return 0;
}

U_BOOT_CMD(
        batterybmp, CONFIG_SYS_MAXARGS, 0, do_batterybmp, NULL, NULL
);

#endif

static int do_i2cport(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int port;
    if(argc>1){
        port = simple_strtol(argv[1],0,10);
		switch(port){
			case 1:{setup_i2c(I2C1_BASE_ADDR);i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);	}break;
			default:
			case 2:{setup_i2c(I2C2_BASE_ADDR);i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);	}break;
			case 3:{setup_i2c(I2C3_BASE_ADDR);i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);	}break;			
		}
	}
	printf("current i2cport=%d[0x%x]\n",
		(I2C1_BASE_ADDR==I2C_BASE)?1:
		(I2C2_BASE_ADDR==I2C_BASE)?2:
		(I2C3_BASE_ADDR==I2C_BASE)?3:-1,I2C_BASE);
	
	return 0;
}

U_BOOT_CMD(
        i2cport, CONFIG_SYS_MAXARGS, 0, do_i2cport, "i2cport - switch mxc i2cport[1|2|3]", NULL
);



int checkboard(void)
{
	printf("Board: %s-%s: %s Board: 0x%x [",
	mx6_chip_name(),
	board_identity(),
	board_revision(),
	fsl_system_rev);

	switch (__REG(SRC_BASE_ADDR + 0x8)) {
	case 0x0001:{
			printf("POR");
			system_boot_reason = eBootReasonPOR;
		}
		break;
	case 0x0009:{
			printf("RST");
			system_boot_reason = eBootReasonReset;			
		}
		break;
	case 0x0010:
	case 0x0011:{			
			system_boot_reason = eBootReasonWDOG;			
			printf("WDOG");
		}
		break;
	default:
		printf("unknown");
	}
	printf(" ]\n");

	printf("Boot Device: ");
	switch (get_boot_device()) {
	case WEIM_NOR_BOOT:
		printf("NOR\n");
		break;
	case ONE_NAND_BOOT:
		printf("ONE NAND\n");
		break;
	case PATA_BOOT:
		printf("PATA\n");
		break;
	case SATA_BOOT:
		printf("SATA\n");
		break;
	case I2C_BOOT:
		printf("I2C\n");
		break;
	case SPI_NOR_BOOT:
		printf("SPI NOR\n");
		break;
	case SD_BOOT:
		printf("SD\n");
		break;
	case MMC_BOOT:
		printf("MMC\n");
		break;
	case NAND_BOOT:
		printf("NAND\n");
		break;
	case UNKNOWN_BOOT:
	default:
		printf("UNKNOWN\n");
		break;
	}

#ifdef CONFIG_SECURE_BOOT
	if (check_hab_enable() == 1)
		get_hab_status();
#endif

	/*Move i2c and pmic setup voltages to here because MIPI lcd require VGEN6 decrease 2.8V from 3.3V*/
	#ifdef CONFIG_I2C_MXC
	i2c_bus_recovery();
	setup_i2c(CONFIG_SYS_I2C_PORT);
	if (setup_pmic_voltages()){
		printf("setup pmic voltagte error\n");	
	}
	#endif

	return 0;
}


#ifdef CONFIG_IMX_UDC

void udc_pins_setting(void)
{
	mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_GPIO_1__USBOTG_ID));	
	//no otg host feature on tdh board
	//Set GPIO_1 as OTG_ID pin
	mxc_iomux_set_gpr_register(1, 13, 1, 1);

}
#endif

int misc_init_r (void)
{
#ifdef CONFIG_ANDROID_BOOTMODE

	/*
	 * FIXME,add more robust feature here
	 *
	 * 1.Check if DC in
	 * 2.Check if battery level is low
	   */
	{
		int chargermode_wakeup=0;		
		int charger_online,charger_status;
		int soc;
		int soc_low=0;
		int bat_low_threshold=5;
		int bat_low_force=0;
		char* env;
		iomux_v3_cfg_t mx6_power_pads[] = {
			MX6X_IOMUX(PAD_EIM_A25__GPIO_5_2),  /* CHG_FLT1_B */
			NEW_PAD_CTRL(MX6X_IOMUX(PAD_EIM_D23__GPIO_3_23),PAD_CTL_PUE|PAD_CTL_HYS), /* CHG_STATUS1_B */
			0,  /* UOK_B */
			MX6X_IOMUX(PAD_EIM_CS1__GPIO_2_24),   /* DOK_B */
			MX6X_IOMUX(PAD_KEY_COL4__GPIO_4_14),	/*Battery Alert IRQ*/
			0, /*Batter Detection*/
		};
		qpower_charger_pdata qpp={
			.dok	= IMX_GPIO_NR(2,24),
			.uok	= 0,
			.chg	= IMX_GPIO_NR(3,23),
			.flt	= IMX_GPIO_NR(5,2),
			.det	= 0,

			.fuelgauge_bus = 0,
			.fuelgauge_addr = 0x36,
		};
		mxc_iomux_v3_setup_multiple_pads(mx6_power_pads,
			sizeof(mx6_power_pads) /
			sizeof(mx6_power_pads[0]));
		//init power supply 	
		setup_i2c(CONFIG_FUELGAUGE_I2C_PORT);
		powersupply_init(&qpp);
		soc=powersupply_soc();
		if(env=getenv("batlowthres")){
			int low_threshold = simple_strtoul(env,NULL,10);
			if((low_threshold>=0)&&
				(low_threshold<=100))
				bat_low_threshold = low_threshold;
		}
		if(soc>=0){
			printf("soc[%d]thres[%d]\n",soc,bat_low_threshold);
			if(soc<=bat_low_threshold)
				soc_low++;
		}
		charger_online=charger_status=0;

	#ifdef CONFIG_CHARGER_OFF
		//check if we should enter charger mode
		if(charger_check_and_clean_flag()||chargermode_wakeup){
			android_bootmode = eBootModeCharger;
		}
		if(!linux_check_and_clean_flag())
			android_bootmode = eBootModeCharger;
	#endif

		if(eBootModeNormal!=android_bootmode){
			//handle normal status
			env = getenv("autocharger");
			if(!env||strcmp(env,"enabled")){
				android_bootmode = eBootModeNormal;
			}
		}
		if(powersupply_dok())
			charger_online++;
		if(powersupply_uok())
			charger_online++;
		charger_status = powersupply_chg();

		if(eBootModeNormal!=android_bootmode){
			if(!charger_online||!charger_status)
				android_bootmode = eBootModeNormal;
		}
		if(eBootModeNormal!=android_bootmode){
			if(eBootReasonWDOG==system_boot_reason)
				android_bootmode = eBootModeNormal;
		}
		
		//FIXME,if battery alert sensed,only enabled charger mode???
		if(getenv("batlow")){
			bat_low_force++;
			setenv("batlow",0);
			saveenv();
		}
		if((soc_low&&!charger_online)||bat_low_force){
			bmp_t batbmp;
			lcd_screen_clear();
			if(!bmp_manager_getbmp("bmp.bat0",&batbmp)){
				bmp_manager_readbmp("bmp.bat0",CONFIG_SYS_LOAD_ADDR,0x2000000);
				draw_bmp((u8*)CONFIG_SYS_LOAD_ADDR,0);
			}else {
				draw_bmp(bmp_bat0,0);
			}
			sdelay(5);			
			//shutdown whole machine now
			run_command("shutdown", 0);
		}


		//switch back to pmic bus		
		setup_i2c(CONFIG_SYS_I2C_PORT);
	}
	   
		
	
		
#endif

	return 0;
}

#ifdef CONFIG_CFB_CONSOLE
#include <video_fb.h>

static GraphicDevice ctfb;

void *video_hw_init(void)
{
	lcd_base = (void *)(gd->fb_base); 
	lcd_enable();
	ctfb.winSizeX = panel_info.vl_col;
	ctfb.winSizeY = panel_info.vl_row;
	/* fill in Graphic device struct */
	sprintf(ctfb.modeIdent, "MXFB");

	ctfb.frameAdrs = (unsigned int)lcd_base;
	ctfb.plnSizeX = ctfb.winSizeX;
	ctfb.plnSizeY = ctfb.winSizeY;

	ctfb.gdfBytesPP = 2;
	ctfb.gdfIndex = GDF_16BIT_565RGB;

	ctfb.isaBase = 0;
	ctfb.pciBase = 0;
	ctfb.memSize = CONFIG_FB_SIZE;

	/* Cursor Start Address */
	ctfb.dprBase = (unsigned int) lcd_base + (ctfb.winSizeX * ctfb.winSizeY * ctfb.gdfBytesPP);        
	if ((ctfb.dprBase & 0x0fff) != 0) {
		/* allign it */
		ctfb.dprBase &= 0xfffff000;
		ctfb.dprBase += 0x00001000;
	}
	ctfb.vprBase = (unsigned int) lcd_base;
	ctfb.cprBase = (unsigned int) lcd_base;

	//background color
	ctfb.bg = 0;

	return &ctfb;
}

#endif
