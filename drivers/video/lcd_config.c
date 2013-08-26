/*
 * (C) Copyright 2012-2013 Freescale Semiconductor, Inc.
 */

#include "mipi_common.h"
#include <asm/io.h>

#define MIPI_DSI_MAX_RET_PACK_SIZE				(0x4)

#define HX8369BL_MAX_BRIGHT		(255)
#define HX8369BL_DEF_BRIGHT		(255)

#define HX8369_MAX_DPHY_CLK					(800)
#define HX8369_ONE_DATA_LANE					(0x1)
#define HX8369_TWO_DATA_LANE					(0x2)

#define HX8369_CMD_SETEXTC					(0xB9)
#define HX8369_CMD_SETEXTC_LEN					(0x4)
#define HX8369_CMD_SETEXTC_PARAM_1				(0x6983ff)

#define HX8369_CMD_GETHXID					(0xF4)
#define HX8369_CMD_GETHXID_LEN					(0x4)
#define HX8369_ID						(0x69)
#define HX8369_ID_MASK						(0xFF)

#define HX8369_CMD_SETDISP					(0xB2)
#define HX8369_CMD_SETDISP_LEN					(16)
#define HX8369_CMD_SETDISP_1_HALT				(0x00)
#define HX8369_CMD_SETDISP_2_RES_MODE				(0x23)
#define HX8369_CMD_SETDISP_3_BP					(0x03)
#define HX8369_CMD_SETDISP_4_FP					(0x03)
#define HX8369_CMD_SETDISP_5_SAP				(0x70)
#define HX8369_CMD_SETDISP_6_GENON				(0x00)
#define HX8369_CMD_SETDISP_7_GENOFF				(0xff)
#define HX8369_CMD_SETDISP_8_RTN				(0x00)
#define HX8369_CMD_SETDISP_9_TEI				(0x00)
#define HX8369_CMD_SETDISP_10_TEP_UP				(0x00)
#define HX8369_CMD_SETDISP_11_TEP_LOW				(0x00)
#define HX8369_CMD_SETDISP_12_BP_PE				(0x03)
#define HX8369_CMD_SETDISP_13_FP_PE				(0x03)
#define HX8369_CMD_SETDISP_14_RTN_PE				(0x00)
#define HX8369_CMD_SETDISP_15_GON				(0x01)

#define HX8369_CMD_SETCYC					(0xB4)
#define HX8369_CMD_SETCYC_LEN					(6)
#define HX8369_CMD_SETCYC_PARAM_1				(0x5f1d00)
#define HX8369_CMD_SETCYC_PARAM_2				(0x060e)

#define HX8369_CMD_SETGIP					(0xD5)
#define HX8369_CMD_SETGIP_LEN					(27)
#define HX8369_CMD_SETGIP_PARAM_1				(0x030400)
#define HX8369_CMD_SETGIP_PARAM_2				(0x1c050100)
#define HX8369_CMD_SETGIP_PARAM_3				(0x00030170)
#define HX8369_CMD_SETGIP_PARAM_4				(0x51064000)
#define HX8369_CMD_SETGIP_PARAM_5				(0x41000007)
#define HX8369_CMD_SETGIP_PARAM_6				(0x07075006)
#define HX8369_CMD_SETGIP_PARAM_7				(0x040f)

#define HX8369_CMD_SETPOWER					(0xB1)
#define HX8369_CMD_SETPOWER_LEN					(20)
#define HX8369_CMD_SETPOWER_PARAM_1				(0x340001)
#define HX8369_CMD_SETPOWER_PARAM_2				(0x0f0f0006)
#define HX8369_CMD_SETPOWER_PARAM_3				(0x3f3f322a)
#define HX8369_CMD_SETPOWER_PARAM_4				(0xe6013a07)
#define HX8369_CMD_SETPOWER_PARAM_5				(0xe6e6e6e6)

#define HX8369_CMD_SETVCOM					(0xB6)
#define HX8369_CMD_SETVCOM_LEN					(3)
#define HX8369_CMD_SETVCOM_PARAM_1				(0x5656)

#define HX8369_CMD_SETPANEL					(0xCC)
#define HX8369_CMD_SETPANEL_PARAM_1				(0x02)

#define HX8369_CMD_SETGAMMA					(0xE0)
#define HX8369_CMD_SETGAMMA_LEN					(35)
#define HX8369_CMD_SETGAMMA_PARAM_1				(0x221d00)
#define HX8369_CMD_SETGAMMA_PARAM_2				(0x2e3f3d38)
#define HX8369_CMD_SETGAMMA_PARAM_3				(0x0f0d064a)
#define HX8369_CMD_SETGAMMA_PARAM_4				(0x16131513)
#define HX8369_CMD_SETGAMMA_PARAM_5				(0x1d001910)
#define HX8369_CMD_SETGAMMA_PARAM_6				(0x3f3d3822)
#define HX8369_CMD_SETGAMMA_PARAM_7				(0x0d064a2e)
#define HX8369_CMD_SETGAMMA_PARAM_8				(0x1315130f)
#define HX8369_CMD_SETGAMMA_PARAM_9				(0x191016)

#define HX8369_CMD_SETMIPI					(0xBA)
#define HX8369_CMD_SETMIPI_LEN					(14)
#define HX8369_CMD_SETMIPI_PARAM_1				(0xc6a000)
#define HX8369_CMD_SETMIPI_PARAM_2				(0x10000a00)
#define HX8369_CMD_SETMIPI_ONELANE				(0x10 << 24)
#define HX8369_CMD_SETMIPI_TWOLANE				(0x11 << 24)
#define HX8369_CMD_SETMIPI_PARAM_3				(0x00026f30)
#define HX8369_CMD_SETMIPI_PARAM_4				(0x4018)

#define HX8369_CMD_SETPIXEL_FMT					(0x3A)
#define HX8369_CMD_SETPIXEL_FMT_24BPP				(0x77)
#define HX8369_CMD_SETPIXEL_FMT_18BPP				(0x66)
#define HX8369_CMD_SETPIXEL_FMT_16BPP				(0x55)

#define HX8369_CMD_SETCLUMN_ADDR				(0x2A)
#define HX8369_CMD_SETCLUMN_ADDR_LEN				(5)
#define HX8369_CMD_SETCLUMN_ADDR_PARAM_1			(0xdf0000)
#define HX8369_CMD_SETCLUMN_ADDR_PARAM_2			(0x01)

#define HX8369_CMD_SETPAGE_ADDR					(0x2B)
#define HX8369_CMD_SETPAGE_ADDR_LEN				(5)
#define HX8369_CMD_SETPAGE_ADDR_PARAM_1				(0x1f0000)
#define HX8369_CMD_SETPAGE_ADDR_PARAM_2				(0x03)

#define HX8369_CMD_WRT_DISP_BRIGHT				(0x51)
#define HX8369_CMD_WRT_DISP_BRIGHT_PARAM_1			(0xFF)

#define HX8369_CMD_WRT_CABC_MIN_BRIGHT				(0x5E)
#define HX8369_CMD_WRT_CABC_MIN_BRIGHT_PARAM_1			(0x20)

#define HX8369_CMD_WRT_CABC_CTRL				(0x55)
#define HX8369_CMD_WRT_CABC_CTRL_PARAM_1			(0x1)

#define HX8369_CMD_WRT_CTRL_DISP				(0x53)
#define HX8369_CMD_WRT_CTRL_DISP_PARAM_1			(0x24)

#define CHECK_RETCODE(ret)					\
do {								\
	if (ret < 0) {						\
			printf("%s ERR: ret:%d, line:%d.\n",		\
			__func__, ret, __LINE__);		\
		return ret;					\
	}							\
} while (0)

#define MIPI_DSI_GEN_PLD_DATA 0x038
#define MIPI_DSI_CMD_PKT_STATUS 0x03c
#define MIPI_DSI_GEN_HDR 0x034
#define DSI_CMD_PKT_STATUS_GEN_PLD_W_FULL (0x1<<3)
#define DSI_CMD_PKT_STATUS_GEN_CMD_FULL (0x1<<1)
#define DSI_CMD_PKT_STATUS_GEN_CMD_EMPTY (0x1<<0)
#define DSI_CMD_PKT_STATUS_GEN_PLD_W_EMPTY (0x1<<2)
#define DSI_GEN_PLD_DATA_BUF_SIZE (0x4)
#define MIPI_DSI_REG_RW_TIMEOUT (20)
#define DSI_GEN_HDR_DATA_MASK (0xffff)
#define DSI_GEN_HDR_DATA_SHIFT 8
#define MIPI_DSI_GENERIC_LONG_WRITE 0x29
#define MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM 0x23
#define MIPI_DCS_EXIT_SLEEP_MODE 0x11
#define MIPI_DSI_GENERIC_SHORT_WRITE_1_PARAM 0x13
#define MIPI_DCS_SET_DISPLAY_ON 0x29

void mipi_dsi_write_register(u32 reg, u32 val)
{
	writel(val, MIPI_DSI_IPS_BASE_ADDR + reg);
}

void mipi_dsi_read_register(u32 reg, u32 *val)
{
	*val = readl(MIPI_DSI_IPS_BASE_ADDR + reg);
}

int mipi_dsi_pkt_write(u8 data_type, const u32 *buf, int len)
{
	u32 val;
	u32 status = 0;
	int write_len = len;
	uint32_t	timeout = 0;

	if (len) {
		/* generic long write command */
		while (len / DSI_GEN_PLD_DATA_BUF_SIZE) {
			mipi_dsi_write_register(MIPI_DSI_GEN_PLD_DATA, *buf);
			buf++;
			len -= DSI_GEN_PLD_DATA_BUF_SIZE;
			mipi_dsi_read_register(MIPI_DSI_CMD_PKT_STATUS,
								&status);
			while ((status & DSI_CMD_PKT_STATUS_GEN_PLD_W_FULL) ==
					 DSI_CMD_PKT_STATUS_GEN_PLD_W_FULL) {
				msleep(1);
				timeout++;
				if (timeout == MIPI_DSI_REG_RW_TIMEOUT)
					return -1;
				mipi_dsi_read_register(MIPI_DSI_CMD_PKT_STATUS,
								&status);
			}
		}
		/* write the remainder bytes */
		if (len > 0) {
			while ((status & DSI_CMD_PKT_STATUS_GEN_PLD_W_FULL) ==
					 DSI_CMD_PKT_STATUS_GEN_PLD_W_FULL) {
				msleep(1);
				timeout++;
				if (timeout == MIPI_DSI_REG_RW_TIMEOUT)
					return -1;
				mipi_dsi_read_register(MIPI_DSI_CMD_PKT_STATUS,
								&status);
			}
			mipi_dsi_write_register(MIPI_DSI_GEN_PLD_DATA, *buf);
		}

		val = data_type | ((write_len & DSI_GEN_HDR_DATA_MASK)
			<< DSI_GEN_HDR_DATA_SHIFT);
	} else {
		/* generic short write command */
		val = data_type | ((*buf & DSI_GEN_HDR_DATA_MASK)
			<< DSI_GEN_HDR_DATA_SHIFT);
	}

	mipi_dsi_read_register(MIPI_DSI_CMD_PKT_STATUS, &status);
	while ((status & DSI_CMD_PKT_STATUS_GEN_CMD_FULL) ==
			 DSI_CMD_PKT_STATUS_GEN_CMD_FULL) {
		msleep(1);
		timeout++;
		if (timeout == MIPI_DSI_REG_RW_TIMEOUT)
			return -1;
		mipi_dsi_read_register(MIPI_DSI_CMD_PKT_STATUS,
				&status);
	}
	mipi_dsi_write_register(MIPI_DSI_GEN_HDR, val);

	mipi_dsi_read_register(MIPI_DSI_CMD_PKT_STATUS, &status);
	while (!((status & DSI_CMD_PKT_STATUS_GEN_CMD_EMPTY) ==
			 DSI_CMD_PKT_STATUS_GEN_CMD_EMPTY) ||
			!((status & DSI_CMD_PKT_STATUS_GEN_PLD_W_EMPTY) ==
			DSI_CMD_PKT_STATUS_GEN_PLD_W_EMPTY)) {
		msleep(1);
		timeout++;
		if (timeout == MIPI_DSI_REG_RW_TIMEOUT)
			return -1;
		mipi_dsi_read_register(MIPI_DSI_CMD_PKT_STATUS,
				&status);
	}
	return 0;
}

/************* Initial LCD Driver ****************/
int MIPILCD_ICINIT(void)
{
	unsigned long read_data;
	int i, j, err;
	u32 buf[32];
#if 0
	buf[0] = HX8369_CMD_SETEXTC | (HX8369_CMD_SETEXTC_PARAM_1 << 8);
	err = mipi_dsi_pkt_write(0x29,
					buf, HX8369_CMD_SETEXTC_LEN);
	CHECK_RETCODE(err);
	buf[0] = MIPI_DSI_MAX_RET_PACK_SIZE;
	err = mipi_dsi_pkt_write(0x37,
				buf, 0);
	CHECK_RETCODE(err);

	/* set LCD resolution as 480RGBx800, DPI interface,
	 * display operation mode: RGB data bypass GRAM mode.
	 */
	buf[0] = HX8369_CMD_SETDISP | (HX8369_CMD_SETDISP_1_HALT << 8) |
			(HX8369_CMD_SETDISP_2_RES_MODE << 16) |
			(HX8369_CMD_SETDISP_3_BP << 24);
	buf[1] = HX8369_CMD_SETDISP_4_FP | (HX8369_CMD_SETDISP_5_SAP << 8) |
			 (HX8369_CMD_SETDISP_6_GENON << 16) |
			 (HX8369_CMD_SETDISP_7_GENOFF << 24);
	buf[2] = HX8369_CMD_SETDISP_8_RTN | (HX8369_CMD_SETDISP_9_TEI << 8) |
			 (HX8369_CMD_SETDISP_10_TEP_UP << 16) |
			 (HX8369_CMD_SETDISP_11_TEP_LOW << 24);
	buf[3] = HX8369_CMD_SETDISP_12_BP_PE |
			(HX8369_CMD_SETDISP_13_FP_PE << 8) |
			 (HX8369_CMD_SETDISP_14_RTN_PE << 16) |
			 (HX8369_CMD_SETDISP_15_GON << 24);
	err = mipi_dsi_pkt_write(MIPI_DSI_GENERIC_LONG_WRITE,
						buf, HX8369_CMD_SETDISP_LEN);
	CHECK_RETCODE(err);

	/* Set display waveform cycle */
	buf[0] = HX8369_CMD_SETCYC | (HX8369_CMD_SETCYC_PARAM_1 << 8);
	buf[1] = HX8369_CMD_SETCYC_PARAM_2;
	err = mipi_dsi_pkt_write(MIPI_DSI_GENERIC_LONG_WRITE,
						buf, HX8369_CMD_SETCYC_LEN);
	CHECK_RETCODE(err);

	/* Set GIP timing output control */
	buf[0] = HX8369_CMD_SETGIP | (HX8369_CMD_SETGIP_PARAM_1 << 8);
	buf[1] = HX8369_CMD_SETGIP_PARAM_2;
	buf[2] = HX8369_CMD_SETGIP_PARAM_3;
	buf[3] = HX8369_CMD_SETGIP_PARAM_4;
	buf[4] = HX8369_CMD_SETGIP_PARAM_5;
	buf[5] = HX8369_CMD_SETGIP_PARAM_6;
	buf[6] = HX8369_CMD_SETGIP_PARAM_7;
	err = mipi_dsi_pkt_write(MIPI_DSI_GENERIC_LONG_WRITE, buf,
				HX8369_CMD_SETGIP_LEN);
	CHECK_RETCODE(err);

	/* Set power: standby, DC etc. */
	buf[0] = HX8369_CMD_SETPOWER | (HX8369_CMD_SETPOWER_PARAM_1 << 8);
	buf[1] = HX8369_CMD_SETPOWER_PARAM_2;
	buf[2] = HX8369_CMD_SETPOWER_PARAM_3;
	buf[3] = HX8369_CMD_SETPOWER_PARAM_4;
	buf[4] = HX8369_CMD_SETPOWER_PARAM_5;
	err = mipi_dsi_pkt_write(MIPI_DSI_GENERIC_LONG_WRITE, buf,
				HX8369_CMD_SETPOWER_LEN);
	CHECK_RETCODE(err);

	/* Set VCOM voltage. */
	buf[0] = HX8369_CMD_SETVCOM | (HX8369_CMD_SETVCOM_PARAM_1 << 8);
	err = mipi_dsi_pkt_write(MIPI_DSI_GENERIC_LONG_WRITE, buf,
				HX8369_CMD_SETVCOM_LEN);
	CHECK_RETCODE(err);

	/* Set Panel: BGR/RGB or Inversion. */
	buf[0] = HX8369_CMD_SETPANEL | (HX8369_CMD_SETPANEL_PARAM_1 << 8);
	err = mipi_dsi_pkt_write(MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM, buf, 0);
	CHECK_RETCODE(err);

	/* Set gamma curve related setting */
	buf[0] = HX8369_CMD_SETGAMMA | (HX8369_CMD_SETGAMMA_PARAM_1 << 8);
	buf[1] = HX8369_CMD_SETGAMMA_PARAM_2;
	buf[2] = HX8369_CMD_SETGAMMA_PARAM_3;
	buf[3] = HX8369_CMD_SETGAMMA_PARAM_4;
	buf[4] = HX8369_CMD_SETGAMMA_PARAM_5;
	buf[5] = HX8369_CMD_SETGAMMA_PARAM_6;
	buf[7] = HX8369_CMD_SETGAMMA_PARAM_7;
	buf[7] = HX8369_CMD_SETGAMMA_PARAM_8;
	buf[8] = HX8369_CMD_SETGAMMA_PARAM_9;
	err = mipi_dsi_pkt_write(MIPI_DSI_GENERIC_LONG_WRITE, buf,
				HX8369_CMD_SETGAMMA_LEN);
	CHECK_RETCODE(err);

	/* Set MIPI: DPHYCMD & DSICMD, data lane number */
	buf[0] = HX8369_CMD_SETMIPI | (HX8369_CMD_SETMIPI_PARAM_1 << 8);
	buf[1] = HX8369_CMD_SETMIPI_PARAM_2;
	buf[2] = 0x11026f30;
/*	if (lcd_config.data_lane_num == HX8369_ONE_DATA_LANE)
		buf[2] |= HX8369_CMD_SETMIPI_ONELANE;
	else
		buf[2] |= HX8369_CMD_SETMIPI_TWOLANE;
*/	buf[3] = HX8369_CMD_SETMIPI_PARAM_4;
	err = mipi_dsi_pkt_write(MIPI_DSI_GENERIC_LONG_WRITE, buf,
				HX8369_CMD_SETMIPI_LEN);
	CHECK_RETCODE(err);

	/* Set pixel format:24bpp */
	buf[0] = 0x773a;
/*	switch (lcd_config.dpi_fmt) {
	case MIPI_RGB565_PACKED:
	case MIPI_RGB565_LOOSELY:
	case MIPI_RGB565_CONFIG3:
		buf[0] |= (HX8369_CMD_SETPIXEL_FMT_16BPP << 8);
		break;

	case MIPI_RGB666_LOOSELY:
	case MIPI_RGB666_PACKED:
		buf[0] |= (HX8369_CMD_SETPIXEL_FMT_18BPP << 8);
		break;

	case MIPI_RGB888:
		buf[0] |= (HX8369_CMD_SETPIXEL_FMT_24BPP << 8);
		break;

	default:
		buf[0] |= (HX8369_CMD_SETPIXEL_FMT_24BPP << 8);
		break;
	}
*/	err = mipi_dsi_pkt_write(MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM,
			buf, 0);
	CHECK_RETCODE(err);

	/* Set column address: 0~479 */
	buf[0] = HX8369_CMD_SETCLUMN_ADDR |
		(HX8369_CMD_SETCLUMN_ADDR_PARAM_1 << 8);
	buf[1] = HX8369_CMD_SETCLUMN_ADDR_PARAM_2;
	err = mipi_dsi_pkt_write(MIPI_DSI_GENERIC_LONG_WRITE,
				buf, HX8369_CMD_SETCLUMN_ADDR_LEN);
	CHECK_RETCODE(err);

	/* Set page address: 0~799 */
	buf[0] = HX8369_CMD_SETPAGE_ADDR |
		(HX8369_CMD_SETPAGE_ADDR_PARAM_1 << 8);
	buf[1] = HX8369_CMD_SETPAGE_ADDR_PARAM_2;
	err = mipi_dsi_pkt_write(MIPI_DSI_GENERIC_LONG_WRITE,
					buf, HX8369_CMD_SETPAGE_ADDR_LEN);
	CHECK_RETCODE(err);

	/* Set display brightness related */
	buf[0] = HX8369_CMD_WRT_DISP_BRIGHT |
			(HX8369_CMD_WRT_DISP_BRIGHT_PARAM_1 << 8);
	err = mipi_dsi_pkt_write(MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM,
		buf, 0);
	CHECK_RETCODE(err);

	buf[0] = HX8369_CMD_WRT_CABC_CTRL |
		(HX8369_CMD_WRT_CABC_CTRL_PARAM_1 << 8);
	err = mipi_dsi_pkt_write(MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM,
		buf, 0);
	CHECK_RETCODE(err);

	buf[0] = HX8369_CMD_WRT_CTRL_DISP |
		(HX8369_CMD_WRT_CTRL_DISP_PARAM_1 << 8);
	err = mipi_dsi_pkt_write(MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM,
		buf, 0);
	CHECK_RETCODE(err);
	/* exit sleep mode and set display on */
	buf[0] = MIPI_DCS_EXIT_SLEEP_MODE;
	err = mipi_dsi_pkt_write(MIPI_DSI_GENERIC_SHORT_WRITE_1_PARAM,
		buf, 0);
	CHECK_RETCODE(err);
	/* To allow time for the supply voltages
	 * and clock circuits to stabilize.
	 */
	msleep(5);
	buf[0] = MIPI_DCS_SET_DISPLAY_ON;
	err = mipi_dsi_pkt_write(MIPI_DSI_GENERIC_SHORT_WRITE_1_PARAM,
		buf, 0);
	CHECK_RETCODE(err);

	/* update backlight, used as a delay function */
	buf[0] = 0xff51;
	err = mipi_dsi_pkt_write(MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM,
				buf, 0);
	CHECK_RETCODE(err);
#endif
	printf("com to %s--init--allenyao\n",__func__);
	//u32 buf[DSI_CMD_BUF_MAXSIZE];
	//int err;

	//*************************************
	// Select CMD2, Page 0
	//*************************************
	//REGW 0xF0, 0x55, 0xAA, 0x52, 0x08, 0x00
	buf[0] = 0xF0 | (0x55 << 8) | (0xAA<<16) |(0x52 << 24) ;
	buf[1] = 0x08 | (0x00<<8);
	err = mipi_dsi_pkt_write(MIPI_DSI_GENERIC_LONG_WRITE,
					buf, 0x6);
	CHECK_RETCODE(err);

	//read ID
	#if 0
	buf[0] = MIPI_DSI_MAX_RET_PACK_SIZE;
	err = mipi_dsi_pkt_write(
				MIPI_DSI_SET_MAXIMUM_RETURN_PACKET_SIZE,
				buf, 0);
	CHECK_RETCODE(err);
	buf[0] = 0x4 ;
	err =  mipi_dsi_pkt_read(mipi_dsi,
			MIPI_DSI_GENERIC_READ_REQUEST_2_PARAM,
			buf, 0x4);

	if (!err ) {
		printk("MIPI DSI LCD ID:0x%x.\n", buf[0]);
	} 
	
	#if 1
	if (!err && ((buf[0] & 0xffff) == 0x8000)) {
		printf("MIPI DSI LCD ID:0x%x.\n", buf[0]);
	} else {
		printf("mipi_dsi_pkt_read err:%d, data:0x%x.\n",err, buf[0]);
		printf("MIPI DSI LCD not detected!\n");
		return err;
	}
	#endif
	#endif


	/* 
	 *Forward Scan      CTB=CRL=0
	 */
	buf[0] = 0xB1 | (0x7C << 8) |(0x00 << 16) ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
						buf, 0x3);
	CHECK_RETCODE(err);


	/*Source hold time */
	buf[0] = 0xB6 | (0x05 << 8);
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
						buf, 0x2);
	CHECK_RETCODE(err);

	// Gate EQ control
	buf[0] = 0xB7 | (0x72 << 8) | (0x72<<16);
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE, buf,
				0x3);
	CHECK_RETCODE(err);

	// Source EQ control (Mode 2)
	buf[0] = 0xB8 | (0x01 << 8) | (0x06 << 16) | (0x05 << 24) ;
	buf[1] = 0x04;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE, buf,
				0x5);
	CHECK_RETCODE(err);

	// Bias Current
	buf[0] = 0xBB | (0x55 << 8);
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE, buf,
				0x2);
	CHECK_RETCODE(err);

	// Inversion
	buf[0] = 0xBC | (0x00 << 8) | (0x00 << 16) | (0x00 << 24);
	err = mipi_dsi_pkt_write(
		MIPI_DSI_GENERIC_LONG_WRITE, buf, 0x4);
	CHECK_RETCODE(err);

	//Frame Rate
	buf[0] = 0xBD | (0x01 << 8)| (0x4E << 16)| (0x10 << 24);
	buf[1] = 0x20 | (0x01<<8);
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE, buf,
				0x6);
	CHECK_RETCODE(err);

	#if 0 //have fix in pannel
	/* Set MIPI: DPHYCMD & DSICMD, data lane number */
	buf[0] = HX8369_CMD_SETMIPI | (HX8369_CMD_SETMIPI_PARAM_1 << 8);
	buf[1] = HX8369_CMD_SETMIPI_PARAM_2;
	buf[2] = HX8369_CMD_SETMIPI_PARAM_3;
	if (lcd_config.data_lane_num == HX8369_ONE_DATA_LANE)
		buf[2] |= HX8369_CMD_SETMIPI_ONELANE;
	else
		buf[2] |= HX8369_CMD_SETMIPI_TWOLANE;
	buf[3] = HX8369_CMD_SETMIPI_PARAM_4;
	err = mipi_dsi_pkt_write(mipi_dsi, MIPI_DSI_GENERIC_LONG_WRITE, buf,
				HX8369_CMD_SETMIPI_LEN);
	CHECK_RETCODE(err);
	#endif

	#if 1
	/* Set pixel format:24bpp ,add by allenyao*/
	buf[0] = 0x3A;
	buf[0] |= (0x70 << 8);
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM,
			buf, 0);
	CHECK_RETCODE(err);
	#endif

	// Display Timing: Dual 8-phase 4-overlap
	//REGW 0xC9, 0x61, 0x06, 0x0D, 0x17, 0x17, 0x00
	buf[0] = 0xC9 |(0x61 << 8) |(0x06 << 16) |(0x0D << 24)  ;
	buf[1] = 0x17 | (0x17 << 8)| (0x00 << 8);
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
				buf, 0x7);
	CHECK_RETCODE(err);

	//*************************************
	// Select CMD2, Page 1
	//*************************************
	//REGW 0xF0, 0x55, 0xAA, 0x52, 0x08, 0x01
	buf[0] = 0xF0 |(0x55 << 8) |(0xAA << 16)|(0x52 << 24);
	buf[1] = 0x08 |(0x01 <<8);
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
					buf, 0x6);
	CHECK_RETCODE(err);

	// AVDD: 5.5V
	//REGW 0xB0, 0x0A, 0x0A, 0x0A
	buf[0] = 0xB0 | (0x0A << 8) |(0x0A << 16) |(0x0A << 24);
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x4);
	CHECK_RETCODE(err);

	// AVEE: -5.5V
	//REGW 0xB1, 0x0A, 0x0A, 0x0A
	buf[0] = 0xB1 | (0x0A << 8)| (0x0A << 16)| (0x0A << 24);
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x4);
	CHECK_RETCODE(err);
	
	// VCL: -3.5V
	//REGW 0xB2, 0x02, 0x02, 0x02
	buf[0] = 0xB2 |(0x02 << 8) |(0x02 << 16)|(0x02 << 24);
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x4);
	CHECK_RETCODE(err);

	// VGH: 15.0V
	//REGW 0xB3, 0x10, 0x10, 0x10
	buf[0] = 0xB3 |(0x10 << 8) |(0x10 << 16)|(0x10 << 24);
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x4);
	CHECK_RETCODE(err);	

	// VGLX: -10.0V
	//REGW 0xB4, 0x06, 0x06, 0x06
	buf[0] = 0xB4 |(0x06 << 8) |(0x06 << 16)|(0x06 << 24);
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x4);
	CHECK_RETCODE(err);	

	// AVDD: 3xVDDB
	//REGW 0xB6, 0x54, 0x54, 0x54
	buf[0] = 0xB6 |(0x54 << 8) |(0x54 << 16)|(0x54 << 24);
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x4);
	CHECK_RETCODE(err);	

	// AVEE: -2xVDDB
	//REGW 0xB7, 0x24, 0x24, 0x24
	buf[0] = 0xB7 |(0x24 << 8) |(0x24 << 16)|(0x24 << 24);
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x4);
	CHECK_RETCODE(err);	

	// VCL: -2xVDDB   pump clock : Hsync
	//REGW 0xB8, 0x34, 0x34, 0x34
	buf[0] = 0xB8 |(0x34 << 8) |(0x34 << 16)|(0x34 << 24);
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x4);
	CHECK_RETCODE(err);	

	// VGH: 2xAVDD-AVEE
	//REGW 0xB9, 0x34, 0x34, 0x34
	buf[0] = 0xB9 |(0x34 << 8) |(0x34 << 16)|(0x34 << 24);
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x4);
	CHECK_RETCODE(err);

	// VGLX: AVEE-AVDD
	//REGW 0xBA, 0x14, 0x14, 0x144
	buf[0] = 0xBA |(0x14 << 8) |(0x14 << 16)|(0x14 << 24);
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x4);
	CHECK_RETCODE(err);

	// Set VGMP = 5.2V / VGSP = 0V
	//REGW 0xBC, 0x00, 0xB0 , 0x00
	buf[0] = 0xBC |(0x00 << 8) |(0xB0 << 16)|(0x00 << 24);
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x4);
	CHECK_RETCODE(err);

	// Set VGMN = -5.2V / VGSN = 0V
	//REGW 0xBD, 0x00, 0xB0, 0x00
	buf[0] = 0xBD |(0x00 << 8) |(0xB0 << 16)|(0x00 << 24);
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x4);
	CHECK_RETCODE(err);

	// VMSEL 0: 0xBE00  ;  1 : 0xBF00 
	//REGW 0xC1, 0x00
	buf[0] = 0xC1 |(0x00 << 8) ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x2);
	CHECK_RETCODE(err);

	// Set VCOM_offset
	//REGW 0xBE, 0x46
	buf[0] = 0xBE |(0x46 << 8) ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x2);
	CHECK_RETCODE(err);

	// Pump:0x00 or PFM:0x50 control
	//REGW 0xC2, 0x00
	buf[0] = 0xC2 |(0x00 << 8) ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x2);
	CHECK_RETCODE(err);

	// Gamma Gradient Control
	//REGW 0xD0, 0x0F, 0x0F, 0x10, 0x10
	buf[0] = 0xD0 |(0x0F << 8) |(0x0F << 16)|(0x10 << 24);
	buf[0] = 0x10 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x5);
	CHECK_RETCODE(err);

//R(+) MCR cmd
	//REGW 0xD1,0x00,0x00,0x00,	0x62,0x00,0x90,0x00,	0xAE,0x00,0xC5,0x00,	0xEA,0x01,0x07,0x01,	0x34
	buf[0] = 0xD1 |(0x00 << 8) |(0x00 << 16)|(0x00 << 24);
	buf[1] = 0x62 |(0x00 << 8) |(0x90 << 16)|(0x00 << 24);
	buf[2] = 0xAE |(0x00 << 8) |(0xC5 << 16)|(0x00 << 24);
	buf[3] = 0xEA |(0x01 << 8) |(0x07 << 16)|(0x01 << 24);
	buf[4] = 0x34 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xD2,0x01,0x58,0x01,	0x8F,0x01,0xBB,0x01,	0xFF,0x02,0x37,0x02,	0x39,0x02,0x6E,0x02,	0xA9
	buf[0] = 0xD2 |(0x01 << 8) |(0x58 << 16)|(0x01 << 24);
	buf[1] = 0x8F |(0x01 << 8) |(0xBB << 16)|(0x01 << 24);
	buf[2] = 0xFF |(0x02 << 8) |(0x37 << 16)|(0x02 << 24);
	buf[3] = 0x39 |(0x02 << 8) |(0x6E << 16)|(0x02 << 24);
	buf[4] = 0xA9 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xD3,0x02,0xD0,0x03,	0x07,0x03,0x2C,0x03,	0x5F,0x03,0x81,0x03	,0xAC,0x03,0xC3,0x03,	0xDD
	buf[0] = 0xD3 |(0x02 << 8) |(0xD0 << 16)|(0x03 << 24);
	buf[1] = 0x07 |(0x03 << 8) |(0x2C << 16)|(0x03 << 24);
	buf[2] = 0x5F |(0x03 << 8) |(0x81 << 16)|(0x03 << 24);
	buf[3] = 0xAC |(0x03 << 8) |(0xC3 << 16)|(0x03 << 24);
	buf[4] = 0xDD ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xD4,0x03,0xF6,0x03,	0xFB
	buf[0] = 0xD4 |(0x03 << 8) |(0xF6 << 16)|(0x03 << 24);
	buf[1] = 0xFB ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x5);
	CHECK_RETCODE(err);

//G(+) MCR cmd
	//REGW 0xD5,0x00,0x00,0x00,	0x62,0x00,0x90,0x00,	0xAE,0x00,0xC5,0x00,	0xEA,0x01,0x07,0x01,	0x34
	buf[0] = 0xD5 |(0x00 << 8) |(0x00 << 16)|(0x00 << 24);
	buf[1] = 0x62 |(0x00 << 8) |(0x90 << 16)|(0x00 << 24);
	buf[2] = 0xAE |(0x00 << 8) |(0xC5 << 16)|(0x00 << 24);
	buf[3] = 0xEA |(0x01 << 8) |(0x07 << 16)|(0x01 << 24);
	buf[4] = 0x34 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xD6,0x01,0x58,0x01,	0x8F,0x01,0xBB,0x01,	0xFF,0x02,0x37,0x02,	0x39,0x02,0x6E,0x02,	0xA9
	buf[0] = 0xD6 |(0x01 << 8) |(0x58 << 16)|(0x01 << 24);
	buf[1] = 0x8F |(0x01 << 8) |(0xBB << 16)|(0x01 << 24);
	buf[2] = 0xFF |(0x02 << 8) |(0x37 << 16)|(0x02 << 24);
	buf[3] = 0x39 |(0x02 << 8) |(0x6E << 16)|(0x02 << 24);
	buf[4] = 0xA9 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xD7,0x02,0xD0,0x03,	0x07,0x03,0x2C,0x03,	0x5F,0x03,0x81,0x03,	0xAC,0x03,0xC3,0x03,	0xDD
	buf[0] = 0xD7 |(0x02 << 8) |(0xD0 << 16)|(0x03 << 24);
	buf[1] = 0x07 |(0x03 << 8) |(0x2C << 16)|(0x03 << 24);
	buf[2] = 0x5F |(0x03 << 8) |(0x81 << 16)|(0x03 << 24);
	buf[3] = 0xAC |(0x03 << 8) |(0xC3 << 16)|(0x03 << 24);
	buf[4] = 0xDD ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xD8,0x03,0xF6,0x03,	0xFB
	buf[0] = 0xD8 |(0x03 << 8) |(0xF6 << 16)|(0x03 << 24);
	buf[1] = 0xFB ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x5);
	CHECK_RETCODE(err);

//B(+) MCR cmd
	//REGW 0xD9,0x00,0x00,0x00,	0x62,0x00,0x90,0x00,	0xAE,0x00,0xC5,0x00,	0xEA,0x01,0x07,0x01,	0x34
	buf[0] = 0xD9 |(0x00 << 8) |(0x00 << 16)|(0x00 << 24);
	buf[1] = 0x62 |(0x00 << 8) |(0x90 << 16)|(0x00 << 24);
	buf[2] = 0xAE |(0x00 << 8) |(0xC5 << 16)|(0x00 << 24);
	buf[3] = 0xEA |(0x01 << 8) |(0x07 << 16)|(0x01 << 24);
	buf[4] = 0x34 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xDD,0x01,0x58,0x01,	0x8F,0x01,0xBB,0x01,	0xFF,0x02,0x37,0x02,	0x39,0x02,0x6E,0x02,	0xA9
	buf[0] = 0xDD |(0x01 << 8) |(0x58 << 16)|(0x01 << 24);
	buf[1] = 0x8F |(0x01 << 8) |(0xBB << 16)|(0x01 << 24);
	buf[2] = 0xFF |(0x02 << 8) |(0x37 << 16)|(0x02 << 24);
	buf[3] = 0x39 |(0x02 << 8) |(0x6E << 16)|(0x02 << 24);
	buf[4] = 0xA9 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xDE,0x02,0xD0,0x03,	0x07,0x03,0x2C,0x03,	0x5F,0x03,0x81,0x03,	0xAC,0x03,0xC3,0x03,	0xDD
	buf[0] = 0xDE |(0x02 << 8) |(0xD0 << 16)|(0x03 << 24);
	buf[1] = 0x07 |(0x03 << 8) |(0x2C << 16)|(0x03 << 24);
	buf[2] = 0x5F |(0x03 << 8) |(0x81 << 16)|(0x03 << 24);
	buf[3] = 0xAC |(0x03 << 8) |(0xC3 << 16)|(0x03 << 24);
	buf[4] = 0xDD ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xDF,0x03,0xF6,0x03,	0xFB
	buf[0] = 0xDF |(0x03 << 8) |(0xF6 << 16)|(0x03 << 24);
	buf[1] = 0xFB ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x5);
	CHECK_RETCODE(err);

//R(-) MCR cmd
	//REGW 0xE0,0x00,0x00,0x00,	0x62,0x00,0x90,0x00,	0xAE,0x00,0xC5,0x00,	0xEA,0x01,0x07,0x01,	0x34
	buf[0] = 0xE0 |(0x00 << 8) |(0x00 << 16)|(0x00 << 24);
	buf[1] = 0x62 |(0x00 << 8) |(0x90 << 16)|(0x00 << 24);
	buf[2] = 0xAE |(0x00 << 8) |(0xC5 << 16)|(0x00 << 24);
	buf[3] = 0xEA |(0x01 << 8) |(0x07 << 16)|(0x01 << 24);
	buf[4] = 0x34 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xE1,0x01,0x58,0x01,	0x8F,0x01,0xBB,0x01,	0xFF,0x02,0x37,0x02,	0x39,0x02,0x6E,0x02,	0xA9
	buf[0] = 0xE1 |(0x01 << 8) |(0x58 << 16)|(0x01 << 24);
	buf[1] = 0x8F |(0x01 << 8) |(0xBB << 16)|(0x01 << 24);
	buf[2] = 0xFF |(0x02 << 8) |(0x37 << 16)|(0x02 << 24);
	buf[3] = 0x39 |(0x02 << 8) |(0x6E << 16)|(0x02 << 24);
	buf[4] = 0xA9 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xE2,0x02,0xD0,0x03,	0x07,0x03,0x2C,0x03,	0x5F,0x03,0x81,0x03,	0xAC,0x03,0xC3,0x03,	0xDD
	buf[0] = 0xE2 |(0x02 << 8) |(0xD0 << 16)|(0x03 << 24);
	buf[1] = 0x07 |(0x03 << 8) |(0x2C << 16)|(0x03 << 24);
	buf[2] = 0x5F |(0x03 << 8) |(0x81 << 16)|(0x03 << 24);
	buf[3] = 0xAC |(0x03 << 8) |(0xC3 << 16)|(0x03 << 24);
	buf[4] = 0xDD ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xE3,0x03,0xF6,0x03,0xFB
	buf[0] = 0xE3 |(0x03 << 8) |(0xF6 << 16)|(0x03 << 24);
	buf[1] = 0xFB ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x5);
	CHECK_RETCODE(err);

//G(-) MCR cmd
	//REGW 0xE4,0x00,0x00,0x00,	0x62,0x00,0x90,0x00,	0xAE,0x00,0xC5,0x00,	0xEA,0x01,0x07,0x01,	0x34
	buf[0] = 0xE4 |(0x00 << 8) |(0x00 << 16)|(0x00 << 24);
	buf[1] = 0x62 |(0x00 << 8) |(0x90 << 16)|(0x00 << 24);
	buf[2] = 0xAE |(0x00 << 8) |(0xC5 << 16)|(0x00 << 24);
	buf[3] = 0xEA |(0x01 << 8) |(0x07 << 16)|(0x01 << 24);
	buf[4] = 0x34 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xE5,0x01,0x58,0x01,	0x8F,0x01,0xBB,0x01,	0xFF,0x02,0x37,0x02,	0x39,0x02,0x6E,0x02,	0xA9
	buf[0] = 0xE5 |(0x01 << 8) |(0x58 << 16)|(0x01 << 24);
	buf[1] = 0x8F |(0x01 << 8) |(0xBB << 16)|(0x01 << 24);
	buf[2] = 0xFF |(0x02 << 8) |(0x37 << 16)|(0x02 << 24);
	buf[3] = 0x39 |(0x02 << 8) |(0x6E << 16)|(0x02 << 24);
	buf[4] = 0xA9 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xE6,0x02,0xD0,0x03,	0x07,0x03,0x2C,0x03,	0x5F,0x03,0x81,0x03,	0xAC,0x03,0xC3,0x03,	0xDD
	buf[0] = 0xE6 |(0x02 << 8) |(0xD0 << 16)|(0x03 << 24);
	buf[1] = 0x07 |(0x03 << 8) |(0x2C << 16)|(0x03 << 24);
	buf[2] = 0x5F |(0x03 << 8) |(0x81 << 16)|(0x03 << 24);
	buf[3] = 0xAC |(0x03 << 8) |(0xC3 << 16)|(0x03 << 24);
	buf[4] = 0xDD ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xE7,0x03,0xF6,0x03,0xFB
	buf[0] = 0xE7 |(0x03 << 8) |(0xF6 << 16)|(0x03 << 24);
	buf[1] = 0xFB ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x5);
	CHECK_RETCODE(err);

//B(-) MCR cmd
	//REGW 0xE8,0x00,0x00,0x00,	0x62,0x00,0x90,0x00,	0xAE,0x00,0xC5,0x00,	0xEA,0x01,0x07,0x01,	0x34
	buf[0] = 0xE8 |(0x00 << 8) |(0x00 << 16)|(0x00 << 24);
	buf[1] = 0x62 |(0x00 << 8) |(0x90 << 16)|(0x00 << 24);
	buf[2] = 0xAE |(0x00 << 8) |(0xC5 << 16)|(0x00 << 24);
	buf[3] = 0xEA |(0x01 << 8) |(0x07 << 16)|(0x01 << 24);
	buf[4] = 0x34 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xE9,0x01,0x58,0x01,	0x8F,0x01,0xBB,0x01,	0xFF,0x02,0x37,0x02,	0x39,0x02,0x6E,0x02,	0xA9
	buf[0] = 0xE9 |(0x01 << 8) |(0x58 << 16)|(0x01 << 24);
	buf[1] = 0x8F |(0x01 << 8) |(0xBB << 16)|(0x01 << 24);
	buf[2] = 0xFF |(0x02 << 8) |(0x37 << 16)|(0x02 << 24);
	buf[3] = 0x39 |(0x02 << 8) |(0x6E << 16)|(0x02 << 24);
	buf[4] = 0xA9 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xEA,0x02,0xD0,0x03,	0x07,0x03,0x2C,0x03,	0x5F,0x03,0x81,0x03,	0xAC,0x03,0xC3,0x03,	0xDD
	buf[0] = 0xEA |(0x02 << 8) |(0xD0 << 16)|(0x03 << 24);
	buf[1] = 0x07 |(0x03 << 8) |(0x2C << 16)|(0x03 << 24);
	buf[2] = 0x5F |(0x03 << 8) |(0x81 << 16)|(0x03 << 24);
	buf[3] = 0xAC |(0x03 << 8) |(0xC3 << 16)|(0x03 << 24);
	buf[4] = 0xDD ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xEB,0x03,0xF6,0x03,0xFB
	buf[0] = 0xEB |(0x03 << 8) |(0xF6 << 16)|(0x03 << 24);
	buf[1] = 0xFB ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x5);
	CHECK_RETCODE(err);

	//*************************************
	// TE On                               
	//*************************************
	//REGW 0x35,0x00
	buf[0] = 0x35 |(0x00 << 8) ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x2);
	CHECK_RETCODE(err);
	
	/* exit sleep mode and set display on */
	buf[0] = MIPI_DCS_EXIT_SLEEP_MODE;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_SHORT_WRITE_1_PARAM,
		buf, 0);
	CHECK_RETCODE(err);
	/* To allow time for the supply voltages
	 * and clock circuits to stabilize.
	 */
	msleep(120);
	buf[0] = MIPI_DCS_SET_DISPLAY_ON;
	//buf[0] = MIPI_DCS_SET_DISPLAY_OFF;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_SHORT_WRITE_1_PARAM,
		buf, 0);
	CHECK_RETCODE(err);

	//err = mipid_init_backlight(mipi_dsi);//not use by allenyao
	printf("end of %s\n",__func__);
	
	return err;
}
