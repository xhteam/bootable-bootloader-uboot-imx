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
#include <asm/io.h>
#include <asm/arch/mx6.h>
#include <asm/arch/mx6_pins.h>
#include "../../../drivers/video/mipi_common.h"//add by allenyao
#include <mipi_dsi.h>//add by allenyao
#if defined(CONFIG_SECURE_BOOT)
#include <asm/arch/mx6_secure.h>
#endif
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
static int android_bootmode=eBootModeNormal;

static enum boot_device boot_dev;

//#define ENABLE_KEYPAD_SHORTCUT
#ifndef NEW_PAD_CTRL
#define NEW_PAD_CTRL(cfg, pad)	(((cfg) & ~MUX_PAD_CTRL_MASK) | \
		MUX_PAD_CTRL(pad))
#endif

#define MX6_KEY_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE  |		\
		PAD_CTL_PUS_22K_UP | PAD_CTL_SPEED_MED |		\
		PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_SLOW  | PAD_CTL_HYS)


#define KEY_MENU_IO IMX_GPIO_NR(3, 9) /*KEY1*/
#define KEY_HOME_IO IMX_GPIO_NR(3, 10) /*KEY2*/
#define KEY_BACK_IO IMX_GPIO_NR(3, 11) /*KEY3*/
#define KEY_POWER_IO IMX_GPIO_NR(3, 29) /*POWER BUTTON*/ 

#define BOARD_REV_IO1 IMX_GPIO_NR(1, 2)
#define BOARD_REV_IO2 IMX_GPIO_NR(1, 2)
#define BOARD_REV_IO3 IMX_GPIO_NR(1, 2)

#define BOARD_ID_IO1 IMX_GPIO_NR(1, 7)
#define BOARD_ID_IO2 IMX_GPIO_NR(7, 12)


#define USB_OTG_PWR IMX_GPIO_NR(3, 22)


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
static struct fb_videomode mipi_dsi = {/*add by allenyao*/
	 "NT-QHD", 60, 540, 960, 30500/*ps*/,  //945,30500
	 3, 3,
	 60, 35,//5,20
	 8,20,//18
	 FB_SYNC_OE_LOW_ACT,//ori is  FB_SYNC_OE_LOW_ACT
	 FB_VMODE_NONINTERLACED,//ori is  FB_VMODE_NONINTERLACED
	 0,
};
#else
static struct fb_videomode lvds_wvga = {
	 "WVGA", 60, 800, 480, 29850, 89, 164, 23, 10, 10, 10,
	 FB_SYNC_EXT,
	 FB_VMODE_NONINTERLACED,
	 0,
};
#endif

static struct mipi_lcd_config mipilcd_config = {//add by allenyao
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
#if defined CONFIG_MX6Q
	/* UART1 TXD */
	mxc_iomux_v3_setup_pad(MX6Q_PAD_CSI0_DAT10__UART1_TXD);

	/* UART1 RXD */
	mxc_iomux_v3_setup_pad(MX6Q_PAD_CSI0_DAT11__UART1_RXD);
#elif defined CONFIG_MX6DL
	/* UART1 TXD */
	mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_DAT10__UART1_TXD);

	/* UART1 RXD */
	mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_DAT11__UART1_RXD);
#endif
}

#ifdef CONFIG_VIDEO_MX5
void setup_lvds_poweron(void)
{
	int reg;
	/* AUX_5V_EN: GPIO(6, 10) */
#ifdef CONFIG_MX6DL
	mxc_iomux_v3_setup_pad(MX6DL_PAD_NANDF_RB0__GPIO_6_10);
#else
	mxc_iomux_v3_setup_pad(MX6Q_PAD_NANDF_RB0__GPIO_6_10);
#endif

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


static void setup_i2c(unsigned int module_base)
{
	unsigned int reg;

	switch (module_base) {
	case I2C1_BASE_ADDR:
#if defined CONFIG_MX6Q
		/* i2c1 SDA */
		mxc_iomux_v3_setup_pad(MX6Q_PAD_CSI0_DAT8__I2C1_SDA);

		/* i2c1 SCL */
		mxc_iomux_v3_setup_pad(MX6Q_PAD_CSI0_DAT9__I2C1_SCL);
#elif defined CONFIG_MX6DL
		/* i2c1 SDA */
		mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_DAT8__I2C1_SDA);
		/* i2c1 SCL */
		mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_DAT9__I2C1_SCL);
#endif

		/* Enable i2c clock */
		reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR2);
		reg |= 0xC0;
		writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR2);

		break;
	case I2C2_BASE_ADDR:
#if defined CONFIG_MX6Q
		/* i2c2 SDA */
		mxc_iomux_v3_setup_pad(MX6Q_PAD_KEY_ROW3__I2C2_SDA);

		/* i2c2 SCL */
		mxc_iomux_v3_setup_pad(MX6Q_PAD_KEY_COL3__I2C2_SCL);
#elif defined CONFIG_MX6DL
		/* i2c2 SDA */
		mxc_iomux_v3_setup_pad(MX6DL_PAD_KEY_ROW3__I2C2_SDA);

		/* i2c2 SCL */
		mxc_iomux_v3_setup_pad(MX6DL_PAD_KEY_COL3__I2C2_SCL);
#endif

		/* Enable i2c clock */
		reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR2);
		reg |= 0x300;
		writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR2);

		break;
	case I2C3_BASE_ADDR:
#if defined CONFIG_MX6Q
		/* GPIO_3 for I2C3_SCL */
		mxc_iomux_v3_setup_pad(MX6Q_PAD_GPIO_3__I2C3_SCL);
		/* GPIO_6 for I2C3_SDA */
		mxc_iomux_v3_setup_pad(MX6Q_PAD_GPIO_6__I2C3_SDA);

#elif defined CONFIG_MX6DL
		/* GPIO_3 for I2C3_SCL */
		mxc_iomux_v3_setup_pad(MX6DL_PAD_GPIO_3__I2C3_SCL);
		/* GPIO_6 for I2C3_SDA */
		mxc_iomux_v3_setup_pad(MX6DL_PAD_GPIO_6__I2C3_SDA);
#endif
		/* Enable i2c clock */
		reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR2);
		reg |= 0xC00;
		writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR2);

		break;
	default:
		printf("Invalid I2C base: 0x%x\n", module_base);
		break;
	}
}

static void mx6q_i2c_gpio_scl_direction(int bus, int output)
{
	u32 reg;

	switch (bus) {
	case 1:
#if defined CONFIG_MX6Q
		mxc_iomux_v3_setup_pad(MX6Q_PAD_CSI0_DAT9__GPIO_5_27);
#elif defined CONFIG_MX6DL
		mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_DAT9__GPIO_5_27);
#endif
		reg = readl(GPIO5_BASE_ADDR + GPIO_GDIR);
		if (output)
			reg |= I2C1_SCL_GPIO5_27_BIT_MASK;
		else
			reg &= ~I2C1_SCL_GPIO5_27_BIT_MASK;
		writel(reg, GPIO5_BASE_ADDR + GPIO_GDIR);
		break;
	case 2:
#if defined CONFIG_MX6Q
		mxc_iomux_v3_setup_pad(MX6Q_PAD_KEY_COL3__GPIO_4_12);
#elif defined CONFIG_MX6DL
		mxc_iomux_v3_setup_pad(MX6DL_PAD_KEY_COL3__GPIO_4_12);
#endif
		reg = readl(GPIO4_BASE_ADDR + GPIO_GDIR);
		if (output)
			reg |= I2C2_SCL_GPIO4_12_BIT_MASK;
		else
			reg &= ~I2C2_SCL_GPIO4_12_BIT_MASK;
		writel(reg, GPIO4_BASE_ADDR + GPIO_GDIR);
		break;
	case 3:
#if defined CONFIG_MX6Q
		mxc_iomux_v3_setup_pad(MX6Q_PAD_GPIO_3__GPIO_1_3);
#elif defined CONFIG_MX6DL
		mxc_iomux_v3_setup_pad(MX6DL_PAD_GPIO_3__GPIO_1_3);
#endif
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
#if defined CONFIG_MX6Q
		mxc_iomux_v3_setup_pad(MX6Q_PAD_CSI0_DAT8__GPIO_5_26);
#elif defined CONFIG_MX6DL
		mxc_iomux_v3_setup_pad(MX6DL_PAD_CSI0_DAT8__GPIO_5_26);
#endif
		reg = readl(GPIO5_BASE_ADDR + GPIO_GDIR);
		if (output)
			reg |= I2C1_SDA_GPIO5_26_BIT_MASK;
		else
			reg &= ~I2C1_SDA_GPIO5_26_BIT_MASK;
		writel(reg, GPIO5_BASE_ADDR + GPIO_GDIR);
		break;
	case 2:
#if defined CONFIG_MX6Q
		mxc_iomux_v3_setup_pad(MX6Q_PAD_KEY_ROW3__GPIO_4_13);
#elif defined CONFIG_MX6DL
		mxc_iomux_v3_setup_pad(MX6DL_PAD_KEY_ROW3__GPIO_4_13);
#endif
		reg = readl(GPIO4_BASE_ADDR + GPIO_GDIR);
		if (output)
			reg |= I2C2_SDA_GPIO4_13_BIT_MASK;
		else
			reg &= ~I2C2_SDA_GPIO4_13_BIT_MASK;
		writel(reg, GPIO4_BASE_ADDR + GPIO_GDIR);
	case 3:
#if defined CONFIG_MX6Q
		mxc_iomux_v3_setup_pad(MX6Q_PAD_GPIO_6__GPIO_1_6);
#elif defined CONFIG_MX6DL
		mxc_iomux_v3_setup_pad(MX6DL_PAD_GPIO_6__GPIO_1_6);
#endif
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
		/*For camera streaks issue,swap VGEN5 and VGEN3 to power camera.
		*sperate VDDHIGH_IN and camera 2.8V power supply, after switch:
		*VGEN5 for VDDHIGH_IN and increase to 3V to align with datasheet
		*VGEN3 for camera 2.8V power supply
		*/
		/*increase VGEN3 from 2.5 to 2.8V*/
		if (i2c_read(0x8, 0x6e, 1, &value, 1)) {
			printf("Read VGEN3 error!\n");
			return -1;
		}
		value &= ~0xf;
		value |= 0xa;
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
		if (i2c_read(0x8, 0x71, 1, &value, 1)) {
			printf("Read VGEN6 error!\n");
			return -1;
		}
		value &= ~0xf;
		value |= 0xa;
		if (i2c_write(0x8, 0x71, 1, &value, 1)) {
			printf("Set VGEN6 error!\n");
			return -1;
		}
	}

	return 0;
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
			mxc_iomux_v3_setup_multiple_pads(usdhc1_pads,
				sizeof(usdhc1_pads) /
				sizeof(usdhc1_pads[0]));
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

static void power_on_and_reset_mipi_panel_6Q(void)
{
	int reg;

	//LCD PWR
	mxc_iomux_v3_setup_pad(MX6Q_PAD_NANDF_D6__GPIO_2_6);
	reg = readl(GPIO2_BASE_ADDR + GPIO_GDIR);
	reg |= (1 << 6);
	writel(reg, GPIO2_BASE_ADDR + GPIO_GDIR);
	reg = readl(GPIO2_BASE_ADDR + GPIO_DR);
	reg &= ~(1 << 6);
	writel(reg, GPIO2_BASE_ADDR + GPIO_DR);

	//LCD_RST_B
	mxc_iomux_v3_setup_pad(MX6Q_PAD_NANDF_CS3__GPIO_6_16);
	reg = readl(GPIO6_BASE_ADDR + GPIO_GDIR);
	reg |= (1 << 16);
	writel(reg, GPIO6_BASE_ADDR + GPIO_GDIR);
	reg = readl(GPIO6_BASE_ADDR + GPIO_DR);
	reg |= (1 << 16);
	writel(reg, GPIO6_BASE_ADDR + GPIO_DR);
	udelay(10);
	reg = readl(GPIO6_BASE_ADDR + GPIO_DR);
	reg &= ~(1 << 16);
	writel(reg, GPIO6_BASE_ADDR + GPIO_DR);
	udelay(50);
	reg = readl(GPIO6_BASE_ADDR + GPIO_DR);
	reg |= (1 << 16);
	writel(reg, GPIO6_BASE_ADDR + GPIO_DR);
	msleep(200);

	//LCD_BL_PWR_EN	
	mxc_iomux_v3_setup_pad(MX6Q_PAD_KEY_ROW4__GPIO_4_15);
	reg = readl(GPIO4_BASE_ADDR + GPIO_GDIR);
	reg |= (1 << 15);
	writel(reg, GPIO4_BASE_ADDR + GPIO_GDIR);
	reg = readl(GPIO4_BASE_ADDR + GPIO_DR);
	reg |= (1 << 15);
	writel(reg, GPIO4_BASE_ADDR + GPIO_DR);
}
static void mipi_clk_enable(void)
{
	int reg;
	reg = readl(CCM_BASE_ADDR + CLKCTL_CCGR3);
	reg &= ~(3<<16);
	reg |= (3<<16);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CCGR3);
}
static void dphy_write_control(unsigned long testcode, unsigned long testwrite)
{
	writel(0x00000000, DSI_PHY_TST_CTRL0);
	writel((0x00010000 | testcode), DSI_PHY_TST_CTRL1);
	writel(0x00000002, DSI_PHY_TST_CTRL0);
	writel(0x00000000, DSI_PHY_TST_CTRL0);
	writel((0x00000000 | testwrite), DSI_PHY_TST_CTRL1);
	writel(0x00000002, DSI_PHY_TST_CTRL0);
	writel(0x00000000, DSI_PHY_TST_CTRL0);
}
static void mipi_dsi_enable_controller(void)
{
	int rd_data, timeout = 0;
	u32		val;
	u32		lane_byte_clk_period;
	/* config MIPI DSI controller*/
	writel(0x0, DSI_PWR_UP);
	writel(0x00000000, DSI_PHY_RSTZ);
	writel(0x107, DSI_CLKMGR_CFG);
	#if MIPI_DSI
	//add by allenyao
	val = readl(DSI_DPI_CFG);
	val &=0x00000000;
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
	writel(val, DSI_DPI_CFG);
	//add by allenyao end
	#else
	writel(0xf4, DSI_DPI_CFG);//not use by allenyao
	#endif
	writel(0x1c, DSI_PCKHDL_CFG);
	#if MIPI_DSI
	//add by allenyao
	val = (mipi_dsi.xres & DSI_VID_PKT_CFG_VID_PKT_SZ_MASK)
				<< DSI_VID_PKT_CFG_VID_PKT_SZ_SHIFT;
	val |= (NUMBER_OF_CHUNKS & DSI_VID_PKT_CFG_NUM_CHUNKS_MASK)
				<< DSI_VID_PKT_CFG_NUM_CHUNKS_SHIFT;
	val |= (NULL_PKT_SIZE & DSI_VID_PKT_CFG_NULL_PKT_SZ_MASK)
				<< DSI_VID_PKT_CFG_NULL_PKT_SZ_SHIFT;
	writel(val, DSI_VID_PKT_CFG);
	//add by allenyao end
	#else
	writel(0x10041e0, DSI_VID_PKT_CFG);//not use by allenyao
	#endif
	writel(0x00001fff, DSI_CMD_MODE_CFG);
	#if MIPI_DSI
	/*add by allenyao*/
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
	writel(val , DSI_TMR_LINE_CFG);
	/*add by allenyao end*/
	#else
	writel(0x1dd83e1f, DSI_TMR_LINE_CFG);	
	#endif
	#if MIPI_DSI
	/*add by allenyao*/
	val = ((mipi_dsi.vsync_len & DSI_VTIMING_CFG_VSA_LINES_MASK)
					<< DSI_VTIMING_CFG_VSA_LINES_SHIFT);
	val |= ((mipi_dsi.upper_margin & DSI_VTIMING_CFG_VBP_LINES_MASK)
				<< DSI_VTIMING_CFG_VBP_LINES_SHIFT);
	val |= ((mipi_dsi.lower_margin & DSI_VTIMING_CFG_VFP_LINES_MASK)
				<< DSI_VTIMING_CFG_VFP_LINES_SHIFT);
	val |= ((mipi_dsi.yres & DSI_VTIMING_CFG_V_ACT_LINES_MASK)
				<< DSI_VTIMING_CFG_V_ACT_LINES_SHIFT);
	writel(val, DSI_VTIMING_CFG);
	/*add by allenyao end*/
	#else
	writel(0x3201866, DSI_VTIMING_CFG);
	#endif
	writel(0x4040d00, DSI_TMR_CFG);
	writel(0x81, DSI_PHY_IF_CFG);//double lane
	writel(0x0, DSI_ERROR_MSK0);
	writel(0x0, DSI_ERROR_MSK1);
	/* mipi_dsi_dphy_init */
	writel(0x00, DSI_PHY_IF_CTRL);
	writel(0x1, DSI_PWR_UP);
	//dphy_write_control(0x44, 0x32);  /* PLL 27M ref_clk out to 1GHz */
	dphy_write_control(0x44, 0x0c);//change by allenyao
	writel(0x7, DSI_PHY_RSTZ);
	rd_data = readl(DSI_PHY_STATUS);
	while ((rd_data & 0x00000001) != 0x01) {
		msleep(1);
		timeout++;
		if (timeout == 10) {
			printf("Error: phy lock timeout!\n");
			break;
		}
		rd_data = readl(DSI_PHY_STATUS);
	}
	timeout = 0;
	while ((rd_data & 0x00000004) != 0x04) {
		msleep(1);
		timeout++;
		if (timeout == 10) {
			printf("Error: phy lock lane timeout!\n");
			break;
		}
		rd_data = readl(DSI_PHY_STATUS);
	}
	return;
}
static void mipi_dsi_set_mode(int cmd_mode)
{
	u32 reg;
	if (cmd_mode) {
		writel(0x00, DSI_PWR_UP);
		reg = readl(DSI_CMD_MODE_CFG);
		reg |= 0x1;
		writel(reg, DSI_CMD_MODE_CFG);
		writel(0x0, DSI_VID_MODE_CFG);
		writel(0x1, DSI_PWR_UP);
	} else {
		writel(0x00, DSI_PWR_UP);
		reg = readl(DSI_CMD_MODE_CFG);
		reg &= ~(0x1<<0);
		writel(reg, DSI_CMD_MODE_CFG);
		writel(0x1ff, DSI_VID_MODE_CFG);
		writel(0x1, DSI_PWR_UP);
		writel(0x1, DSI_PHY_IF_CTRL);
	}
}
static void mipi_dsi_enable()
{
	int err;
	msleep(5);
	mipi_clk_enable();
	msleep(5);
	mipi_dsi_enable_controller();	
    msleep(5);
	err = MIPILCD_ICINIT();
	if (err < 0) {
		printf("lcd init failed\n");
		return;
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

	/*Move i2c and pmic setup voltages to here because MIPI lcd require VGEN6 decrease 2.8V from 3.3V*/
	#ifdef CONFIG_I2C_MXC
	setup_i2c(CONFIG_SYS_I2C_PORT);
	i2c_bus_recovery();
	ret = setup_pmic_voltages();
	if (ret){
		printf("setup pmic voltagte error\n");	
	}
	#endif

	/*
	* hw_rev 2: IPUV3DEX
	* hw_rev 3: IPUV3M
	* hw_rev 4: IPUV3H
	*/
	g_ipu_hw_rev = IPUV3_HW_REV_IPUV3H;

	imx_pwm_config(pwm0, 25000, 50000);
	imx_pwm_enable(pwm0);

#if defined CONFIG_MX6Q
	/* PWM backlight */
	mxc_iomux_v3_setup_pad(MX6Q_PAD_SD1_DAT3__PWM1_PWMO);
#elif defined CONFIG_MX6DL
	/* PWM backlight */
	mxc_iomux_v3_setup_pad(MX6DL_PAD_SD1_DAT3__PWM1_PWMO);
#endif


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
			DI_PCLK_PLL3, 156454);
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
#if defined(CONFIG_MX6Q)
	power_on_and_reset_mipi_panel_6Q();
#elif defined(CONFIG_MX6DL)
	#error "power_on_and_reset_mipi_panel:put me on MX6DL"
#endif

	
	mipi_dsi_enable();
#endif
	/*add by allenyao end*/

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

        if(line_number == 1) strcpy(info,"(C) 2013 Quester Technology");
		else if(line_number == 2) {			
			strcpy(info,"raymond.wang@quester.com.cn");
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
	long size;
	unsigned long logo;
	logo = CONFIG_SYS_LOAD_ADDR;	
	if(eBootModeCharger==android_bootmode){
		//in charger-only mode,we don't want to display any content in bootloader step
		//setenv("splashimage",NULL);
		
		return;
	}
	run_command("mmc dev 3",0);
	size=bmp_manager_readbmp("bmp.splash",logo,0x20000000);
	if(size<0){
		printf("no splash found\n");
		size =0;	
	}else{
		size=size*512;
		printf("splash size 0x%x\n",size);
	}

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

	mxc_iomux_v3_init((void *)IOMUXC_BASE_ADDR);
	setup_boot_device();
	fsl_set_system_rev();

	/* board id for linux */
	gd->bd->bi_arch_number = MACH_TYPE_MX6Q_QPAD;

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
	memset(gd->fb_base,0,CONFIG_FB_SIZE);
#endif


	return 0;
}


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
		mxc_iomux_v3_setup_pad(NEW_PAD_CTRL(MX6X_IOMUX(PAD_EIM_DA9__GPIO_3_9),MX6_KEY_PAD_CTRL));
		mxc_iomux_v3_setup_pad(NEW_PAD_CTRL(MX6X_IOMUX(PAD_EIM_DA10__GPIO_3_10),MX6_KEY_PAD_CTRL));
		mxc_iomux_v3_setup_pad(NEW_PAD_CTRL(MX6X_IOMUX(PAD_EIM_DA11__GPIO_3_11),MX6_KEY_PAD_CTRL));

		gpio_direction_input(KEY_MENU_IO);
		gpio_direction_input(KEY_HOME_IO);
		gpio_direction_input(KEY_BACK_IO);

		if (!gpio_get_value(KEY_MENU_IO)&&!gpio_get_value(KEY_HOME_IO)) {
				if(!gpio_get_value(KEY_BACK_IO)){
					printf("keypad not connected??\n");
				}else {
					printf("Key recovery detected!\nEnter recovery mode!\n");
					recovery_switch++;
				}
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
	int button_pressed = 0;

	#ifdef ENABLE_KEYPAD_SHORTCUT
	mxc_iomux_v3_setup_pad(NEW_PAD_CTRL(MX6X_IOMUX(PAD_EIM_DA9__GPIO_3_9),MX6_KEY_PAD_CTRL));
	mxc_iomux_v3_setup_pad(NEW_PAD_CTRL(MX6X_IOMUX(PAD_EIM_DA10__GPIO_3_10),MX6_KEY_PAD_CTRL));
	mxc_iomux_v3_setup_pad(NEW_PAD_CTRL(MX6X_IOMUX(PAD_EIM_DA11__GPIO_3_11),MX6_KEY_PAD_CTRL));

	gpio_direction_input(KEY_MENU_IO);
	gpio_direction_input(KEY_HOME_IO);
	gpio_direction_input(KEY_BACK_IO);

	if (!gpio_get_value(KEY_HOME_IO)&&!gpio_get_value(KEY_BACK_IO)) { 
		if(!gpio_get_value(KEY_MENU_IO)){
			printf("keypad not connected??\n");
		}else {
			button_pressed = 1;
			printf("Fastboot key pressed\n");
		}
	}
	#endif


	return button_pressed;
	
}

#endif

#ifdef CONFIG_AUTOUPDATER
int autoupdate_mode_detect(void){
	int button_pressed = 0;

	#ifdef ENABLE_KEYPAD_SHORTCUT
	mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_EIM_DA9__GPIO_3_9));	
	mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_EIM_DA10__GPIO_3_10));
	mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_EIM_DA11__GPIO_3_11));

	gpio_direction_input(KEY_MENU_IO);
	gpio_direction_input(KEY_HOME_IO);
	gpio_direction_input(KEY_BACK_IO);

	if (!gpio_get_value(KEY_MENU_IO)&&!gpio_get_value(KEY_BACK_IO)) { 
		if(!gpio_get_value(KEY_HOME_IO)){
			printf("keypad not connected??\n");
		}else {
			button_pressed = 1;
			printf("Autoupdate key pressed\n");
		}
	}
	#endif

	return button_pressed;
	
}
#endif
int board_late_init(void)
{
	int ret = 0;
	return ret;
}

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
	if(android_bootmode<eBootModeMax&&strlen(bootmode_cmdline[android_bootmode])){
		char* newcmdline = malloc(strlen(cmdline)+strlen(bootmode_cmdline[android_bootmode])+1);
		printf("bootmode=%d,extra cmdline[%s]\n",android_bootmode,bootmode_cmdline[android_bootmode]);
		if(newcmdline){
			strcpy(newcmdline,cmdline);
			strcat(newcmdline,bootmode_cmdline[android_bootmode]);
			return newcmdline;
		}
	}
	return cmdline;
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
	};
	int id=mx6_board_id();

	//check board type
	if(!id){
		//assign id per hardware GPIO settings
		mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_GPIO_7__GPIO_1_7));
		mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_GPIO_17__GPIO_7_12));
		gpio_direction_input(BOARD_ID_IO1);
		gpio_direction_input(BOARD_ID_IO2);
		id=(gpio_get_value(BOARD_ID_IO2)<<1)+gpio_get_value(BOARD_ID_IO1);
		id+=0xb;
	}
	if(id>0xB)
		id=0;
	return ids[id];
}

static const char* board_revision(void){
	static const char* revs[] =
	{
		"RevA",
		"RevB",
		"RevC",
		"RevD",
		"RevE",
		"Unknown",//0x5 maximum E is enough?
	};	
	int id=mx6_board_id();
	int rev=mx6_board_rev();

	//check board type
	if(!id){
		//assign revision per hardware GPIO settings
		rev = 0;
		mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_GPIO_2__GPIO_1_2));
		mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_GPIO_4__GPIO_1_4));
		mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_GPIO_5__GPIO_1_5));
		gpio_direction_input(BOARD_REV_IO1);
		gpio_direction_input(BOARD_REV_IO2);
		gpio_direction_input(BOARD_REV_IO3);
		rev=(gpio_get_value(BOARD_REV_IO3)<<2)+(gpio_get_value(BOARD_REV_IO2)<<1)+gpio_get_value(BOARD_ID_IO1);
	}
	if(rev>0x4)
		rev=5;
	return revs[rev];
	
}

static int powerkey_detect(void){
	mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_EIM_D29__GPIO_3_29));	
	gpio_direction_input(KEY_POWER_IO);
	if(gpio_get_value(KEY_POWER_IO))
		return 1;
	return 0;
}
int checkboard(void)
{
	printf("Board: %s-%s: %s Board: 0x%x [",
	mx6_chip_name(),
	board_identity(),
	board_revision(),
	fsl_system_rev);

	switch (__REG(SRC_BASE_ADDR + 0x8)) {
	case 0x0001:
		printf("POR");
		break;
	case 0x0009:
		printf("RST");
		break;
	case 0x0010:
	case 0x0011:
		printf("WDOG");
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
		char* env;
		iomux_v3_cfg_t mx6q_power_pads[] = {
			MX6Q_PAD_EIM_A25__GPIO_5_2,  /* CHG_FLT1_B */
			NEW_PAD_CTRL(MX6Q_PAD_EIM_D23__GPIO_3_23,PAD_CTL_PUS_100K_DOWN|PAD_CTL_PUE|PAD_CTL_HYS), /* CHG_STATUS1_B */
			MX6Q_PAD_EIM_D17__GPIO_3_17,  /* UOK_B */
			MX6Q_PAD_EIM_CS1__GPIO_2_24,   /* DOK_B */
			MX6Q_PAD_KEY_COL4__GPIO_4_14,	/*Battery Alert IRQ*/
			NEW_PAD_CTRL(MX6Q_PAD_KEY_ROW2__GPIO_4_11,PAD_CTL_DSE_DISABLE), /*Batter Detection*/
		};
	    qpower_charger_pdata qpp={
			.dok	= IMX_GPIO_NR(2,24),
			.uok	= IMX_GPIO_NR(3,17),
			.chg	= IMX_GPIO_NR(3,23),
			.flt	= IMX_GPIO_NR(5,2),
			.det	= IMX_GPIO_NR(4,11),		

			.fuelgauge_bus = 0,
			.fuelgauge_addr = 0x36,
	    };		
		mxc_iomux_v3_setup_multiple_pads(mx6q_power_pads,
			sizeof(mx6q_power_pads) /
			sizeof(mx6q_power_pads[0]));
		//init power supply
		powersupply_init(&qpp);
		charger_online=charger_status=0;

		//
		//if(powersupply_dok())
		//	chargermode_wakeup++;
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
			if(env&&!strcmp(env,"disabled")){
				android_bootmode = eBootModeNormal;
			}
			if(powersupply_dok())
				charger_online++;
			if(powersupply_uok())
				charger_online++;
			charger_status = powersupply_chg();
			if(!charger_online||!charger_status)
				android_bootmode = eBootModeNormal;

			//FIXME,if battery alert sensed,only enabled charger mode???
		}
	}
       
	

	
#endif

	return 0;
}


#ifdef CONFIG_IMX_UDC

void udc_pins_setting(void)
{
	mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_GPIO_1__USBOTG_ID));	
	mxc_iomux_v3_setup_pad(MX6X_IOMUX(PAD_EIM_D22__GPIO_3_22));

	/* USB_OTG_PWR = 0 */
	gpio_direction_output(USB_OTG_PWR, 0);

	//Set GPIO_1 as OTG_ID pin
	mxc_iomux_set_gpr_register(1, 13, 1, 1);

}
#endif

int misc_init_r (void)
{
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
