/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the MX6Q Sabre Lite2 Freescale board.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef MX6Q_SABRESD_ANDROID_H
#define MX6Q_SABRESD_ANDROID_H

#include "mx6q_common.h"

#define CONFIG_USB_DEVICE
#define CONFIG_IMX_UDC		       1
#define CONFIG_FASTBOOT		       1
#define CONFIG_FASTBOOT_STORAGE_EMMC_SATA
#define CONFIG_FASTBOOT_VENDOR_ID      0x18d1
#define CONFIG_FASTBOOT_PRODUCT_ID     0x0d02
#define CONFIG_FASTBOOT_BCD_DEVICE     0x311
#define CONFIG_FASTBOOT_MANUFACTURER_STR  "QuesterTech"
#define CONFIG_FASTBOOT_PRODUCT_NAME_STR "qpad"
#define CONFIG_FASTBOOT_INTERFACE_STR	 "Android fastboot"
#define CONFIG_FASTBOOT_CONFIGURATION_STR  "Android fastboot"
#define CONFIG_FASTBOOT_SERIAL_NUM	"12345"
#define CONFIG_FASTBOOT_SATA_NO		 0

/*  For system.img growing up more than 256MB, more buffer needs
*   to receive the system.img*/
#define CONFIG_FASTBOOT_TRANSFER_BUF	0x2c000000
#define CONFIG_FASTBOOT_TRANSFER_BUF_SIZE 0x14000000 /* 320M byte */

#define CONFIG_CMD_BOOTI
#define CONFIG_ANDROID_RECOVERY
/* which mmc bus is your main storage ? */
#define CONFIG_ANDROID_MAIN_MMC_BUS 3
#define CONFIG_ANDROID_BOOT_PARTITION_MMC 1
#define CONFIG_ANDROID_DATA_PARTITION_MMC 4
#define CONFIG_ANDROID_SYSTEM_PARTITION_MMC 5
#define CONFIG_ANDROID_RECOVERY_PARTITION_MMC 2
#define CONFIG_ANDROID_CACHE_PARTITION_MMC 6


#define CONFIG_ANDROID_RECOVERY_BOOTARGS_MMC NULL
#define CONFIG_ANDROID_RECOVERY_BOOTCMD_MMC  \
	"booti mmc3 recovery"
#define CONFIG_ANDROID_RECOVERY_CMD_FILE "/recovery/command"
#define CONFIG_INITRD_TAG

#undef CONFIG_LOADADDR
#undef CONFIG_RD_LOADADDR
#undef CONFIG_EXTRA_ENV_SETTINGS

#define CONFIG_LOADADDR		0x10800000	/* loadaddr env var */
#define CONFIG_RD_LOADADDR      0x11000000

#define CONFIG_INITRD_TAG

#define	CONFIG_EXTRA_ENV_SETTINGS					\
		"serialnumber=1234567890\0"			\
		"netdev=eth0\0"						\
		"ethprime=FEC0\0"					\
		"password=9dd694e7e648d04b019c56ae7a58f01400000000000000000000000000000000\0" \
		"fastboot_dev=mmc3\0"					\
		"bootcmd=booti mmc3\0"					\
		"splashimage=0x30000000\0"				\
		"splashpos=m,m\0"					\
		"lvds_num=1\0"


//add by allenyao 
#define CONFIG_CONSOLE_PASSWORD
#define CONFIG_MISC_INIT_R
#define CONFIG_MD5
#define CONFIG_AUTOBOOT_KEYED	1
#define CONFIG_AUTOBOOT_DELAY_STR "\x1b\x1b"
//#define CONFIG_CFB_CONSOLE	1
//add by allenyao end

#define CONFIG_AUTOUPDATER
#define CONFIG_AUTOUPDATER_SEQUENCER \
	{"usb start","usb",0}, 

//	{"mmcstart;mmc sw_dev 0;mmc rescan","mmc",0}, 

/*
 *Enable MIPI panel in uboot
*/
#define MIPI_DSI	1
#define CONFIG_ANDROID_BOOTMODE
#define CONFIG_CHARGER_OFF

#undef  CONFIG_LCD
#define CONFIG_VIDEO	1
#define CONFIG_VIDEO_LOGO 1	
#define CONFIG_VIDEO_BMP_LOGO
#define VIDEO_FB_16BPP_PIXEL_SWAP
#define CONFIG_CFB_CONSOLE	           1
#define CONFIG_CONSOLE_EXTRA_INFO
#define CONFIG_VGA_AS_SINGLE_DEVICE

#ifdef  CONFIG_LCD 
#define CONFIG_LCD_INFO 1
#endif

/*I2C port for FuelGauge*/
#define CONFIG_FUELGAUGE_I2C_PORT 	I2C1_BASE_ADDR


#endif
