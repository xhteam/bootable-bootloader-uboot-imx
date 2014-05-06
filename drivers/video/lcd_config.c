/*
 * (C) Copyright 2012-2013 Freescale Semiconductor, Inc.
 */
#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <mipi_dsi.h>
#include "mipi_common.h"
#include "mipi_display.h"


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

static void
msleep(int count)
{
	int i;
	for (i = 0; i < count; i++)
		udelay(1000);
}

static void mipi_dsi_write_register(u32 reg, u32 val)
{
	writel(val, MIPI_DSI_IPS_BASE_ADDR + reg);
}

static void mipi_dsi_read_register(u32 reg, u32 *val)
{
	*val = readl(MIPI_DSI_IPS_BASE_ADDR + reg);
}

static int mipi_dsi_pkt_write(u8 data_type, const u32 *buf, int len)
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
static int mipi_dsi_pkt_read(u8 data_type, u32 *buf, int len)
{
	u32		val;
	int		read_len = 0;
	uint32_t	timeout = 0;

	if (!len) {
		printf("%s, len = 0 invalid error!\n", __func__);
		return -EINVAL;
	}

	val = data_type | ((*buf & DSI_GEN_HDR_DATA_MASK)
		<< DSI_GEN_HDR_DATA_SHIFT);
	memset(buf, 0, len);
	mipi_dsi_write_register(MIPI_DSI_GEN_HDR, val);

	/* wait for cmd to sent out */
	mipi_dsi_read_register( MIPI_DSI_CMD_PKT_STATUS, &val);
	while ((val & DSI_CMD_PKT_STATUS_GEN_RD_CMD_BUSY) !=
			 DSI_CMD_PKT_STATUS_GEN_RD_CMD_BUSY) {
		msleep(1);
		timeout++;
		if (timeout == MIPI_DSI_REG_RW_TIMEOUT)
			return -EIO;
		mipi_dsi_read_register(MIPI_DSI_CMD_PKT_STATUS,
			&val);
	}
	/* wait for entire response stroed in FIFO */
	while ((val & DSI_CMD_PKT_STATUS_GEN_RD_CMD_BUSY) ==
			 DSI_CMD_PKT_STATUS_GEN_RD_CMD_BUSY) {
		msleep(1);
		timeout++;
		if (timeout == MIPI_DSI_REG_RW_TIMEOUT)
			return -EIO;
		mipi_dsi_read_register( MIPI_DSI_CMD_PKT_STATUS,
			&val);
	}

	mipi_dsi_read_register( MIPI_DSI_CMD_PKT_STATUS, &val);
	while (!(val & DSI_CMD_PKT_STATUS_GEN_PLD_R_EMPTY)) {
		mipi_dsi_read_register( MIPI_DSI_GEN_PLD_DATA, buf);
		read_len += DSI_GEN_PLD_DATA_BUF_SIZE;
		buf++;
		mipi_dsi_read_register( MIPI_DSI_CMD_PKT_STATUS,
			&val);
		if (read_len == (DSI_GEN_PLD_DATA_BUF_ENTRY *
					DSI_GEN_PLD_DATA_BUF_SIZE))
			break;
	}

	if ((len <= read_len) &&
		((len + DSI_GEN_PLD_DATA_BUF_SIZE) >= read_len))
		return 0;

	printf("actually read_len:%d != len:%d.\n", read_len, len);
	return -ERANGE;
}


/************* Initial LCD Driver ****************/
#ifndef DSI_CMD_BUF_MAXSIZE
#define DSI_CMD_BUF_MAXSIZE 32
#endif
static inline int mipi_generic_write(u32* buf,int l){
	int err;
	err = mipi_dsi_pkt_write(MIPI_DSI_GENERIC_LONG_WRITE,
					buf, l);
	CHECK_RETCODE(err);
	return err;
}
#define W_COM_1A_0P(o) \
	do{ \
		u32 buf[DSI_CMD_BUF_MAXSIZE];\
		buf[0]=o;\
		mipi_generic_write(buf,1);\
	}while(0)

#define W_COM_1A_1P(o,p) \
	do{ \
		u32 buf[DSI_CMD_BUF_MAXSIZE];\
		buf[0]=o|(p<<8);\
		mipi_generic_write(buf,2);\
	}while(0)
#define W_COM_1A_2P(o,p1,p2) \
	do{ \
		u32 buf[DSI_CMD_BUF_MAXSIZE];\
		buf[0]=o|(p1<<8)|(p2<<16);\
		mipi_generic_write(buf,3);\
	}while(0)
#define W_COM_1A_3P(o,p1,p2,p3) \
		do{ \
			u32 buf[DSI_CMD_BUF_MAXSIZE];\
			buf[0]=o|(p1<<8)|(p2<<16)|(p3<<24);\
			mipi_generic_write(buf,4);\
		}while(0)
#define W_COM_1A_4P(o,p1,p2,p3,p4) \
	do{ \
		u32 buf[DSI_CMD_BUF_MAXSIZE];\
		buf[0]=o|(p1<<8)|(p2<<16)|(p3<<24);\
		buf[1]=p4;\
		mipi_generic_write(buf,5);\
	}while(0)

#define W_COM_1A_10P(o,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10) \
	do{ \
		u32 buf[DSI_CMD_BUF_MAXSIZE];\
		buf[0]=o|(p1<<8)|(p2<<16)|(p3<<24);\
		buf[1]=p4|(p5<<8)|(p6<<16)|(p7<<24);\
		buf[2]=p8|(p9<<8)|(p10<<16);\
		mipi_generic_write(buf,11);\
	}while(0)
#define W_COM_1A_12P(o,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12) \
	do{ \
		u32 buf[DSI_CMD_BUF_MAXSIZE];\
		buf[0]=o|(p1<<8)|(p2<<16)|(p3<<24);\
		buf[1]=p4|(p5<<8)|(p6<<16)|(p7<<24);\
		buf[2]=p8|(p9<<8)|(p10<<16)|(p11<<24);\
		buf[3]=p12;\
		mipi_generic_write(buf,13);\
	}while(0)
#define W_COM_1A_14P(o,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14) \
	do{ \
		u32 buf[DSI_CMD_BUF_MAXSIZE];\
		buf[0]=o|(p1<<8)|(p2<<16)|(p3<<24);\
		buf[1]=p4|(p5<<8)|(p6<<16)|(p7<<24);\
		buf[2]=p8|(p9<<8)|(p10<<16)|(p11<<24);\
		buf[3]=p12|(p13<<8)|(p14<<16);\
		mipi_generic_write(buf,15);\
	}while(0)

#define W_COM_1A_15P(o,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15) \
	do{ \
		u32 buf[DSI_CMD_BUF_MAXSIZE];\
		buf[0]=o|(p1<<8)|(p2<<16)|(p3<<24);\
		buf[1]=p4|(p5<<8)|(p6<<16)|(p7<<24);\
		buf[2]=p8|(p9<<8)|(p10<<16)|(p11<<24);\
		buf[3]=p12|(p13<<8)|(p14<<16)|(p15<<24);\
		mipi_generic_write(buf,16);\
	}while(0)
#define W_COM_1A_16P(o,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16) \
	do{ \
		u32 buf[DSI_CMD_BUF_MAXSIZE];\
		buf[0]=o|(p1<<8)|(p2<<16)|(p3<<24);\
		buf[1]=p4|(p5<<8)|(p6<<16)|(p7<<24);\
		buf[2]=p8|(p9<<8)|(p10<<16)|(p11<<24);\
		buf[3]=p12|(p13<<8)|(p14<<16)|(p15<<24);\
		buf[4]=p16;\
		mipi_generic_write(buf,17);\
	}while(0)

static int otm9605a_detect(void){
	u32 read_data;
	int i, j, err;
	u32 buf[32];
	//read ID
	buf[0] = 0xDA;
	err =  mipi_dsi_pkt_read(MIPI_DSI_GENERIC_READ_REQUEST_2_PARAM,
			buf, 0x4);
	if (!err && ((buf[0] & 0xff) == 0x40)) {
		printf("otm9605 found\n");
		return 1;
	}
	return 0;
}
static int otm9605a_init(void){
	W_COM_1A_1P(0x00,0x00);
	W_COM_1A_3P(0xFF,0x96,0x05,0x01);
	W_COM_1A_1P(0x00,0x80);
	W_COM_1A_2P(0xFF,0x96,0x05);

	W_COM_1A_1P(0x00,0x92);
	W_COM_1A_2P(0xFF,0x10,0x02);

	W_COM_1A_1P(0x00,0x00);
	W_COM_1A_1P(0x00,0x00);
	W_COM_1A_1P(0x00,0x00);
	W_COM_1A_1P(0x00,0x00);
	W_COM_1A_1P(0x00,0x00);
	W_COM_1A_1P(0x00,0x00);
	W_COM_1A_1P(0x00,0x00);
	W_COM_1A_1P(0x00,0x00);

	W_COM_1A_1P(0x00,0xB4);
	W_COM_1A_1P(0xC0,0x50);

	W_COM_1A_1P(0x00,0x89);
	W_COM_1A_1P(0xC0,0x01);
	W_COM_1A_1P(0x00,0xA0);
	W_COM_1A_1P(0xC1,0x00);
	W_COM_1A_1P(0x00,0x00);
	W_COM_1A_1P(0xA0,0x00);
	W_COM_1A_1P(0x00,0xA2);
	W_COM_1A_3P(0xC0,0x01,0x10,0x00);
	W_COM_1A_1P(0x00,0x91);
	W_COM_1A_1P(0xC5,0x77);

	W_COM_1A_1P(0x00,0x80);
	W_COM_1A_1P(0xD6,0x58);
	W_COM_1A_1P(0x00,0x00);
	W_COM_1A_1P(0xD7,0x00);
	W_COM_1A_1P(0x00,0x00);
	W_COM_1A_2P(0xD8,0x6f,0x6f);
	W_COM_1A_1P(0x00,0x00);
	W_COM_1A_1P(0xD9,0x2A);

	////////////// CE improve code ///////////////////
	W_COM_1A_1P(0x00,0x80);
	W_COM_1A_2P(0xC1,0x36,0x66);
	W_COM_1A_1P(0x00,0xb1);
	W_COM_1A_1P(0xC5,0x28);
	W_COM_1A_1P(0x00,0xB2);
	W_COM_1A_4P(0xF5,0x15,0x00,0x15,0x00);

	////////////// CE improve code end ///////////////

	W_COM_1A_1P(0x00,0x00);
	W_COM_1A_16P(0xE1,0x00,0x0F,0x15,0x0E,0x07,0x0E,0x0B,0x09,0x04,0x08,0x0F,0x0A,0x11,0x12,0x0A,0x03);
	W_COM_1A_1P(0x00,0x00);
	W_COM_1A_16P(0xE2,0x00,0x0F,0x15,0x0E,0x07,0x0E,0x0B,0x09,0x04,0x08,0x0F,0x0A,0x11,0x12,0x0A,0x03);

	W_COM_1A_1P(0x00,0x80);
	W_COM_1A_10P(0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);
	W_COM_1A_1P(0x00,0x90);
	W_COM_1A_15P(0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);
	W_COM_1A_1P(0x00,0xA0);
	W_COM_1A_15P(0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);
	W_COM_1A_1P(0x00,0xB0);
	W_COM_1A_10P(0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);
	W_COM_1A_1P(0x00,0xC0);
	W_COM_1A_15P(0xCB,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);
	W_COM_1A_1P(0x00,0xD0);
	W_COM_1A_15P(0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00);
	W_COM_1A_1P(0x00,0xE0);
	W_COM_1A_10P(0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);
	W_COM_1A_1P(0x00,0xF0);
	W_COM_1A_10P(0xCB,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
	W_COM_1A_1P(0x00,0x80);
	W_COM_1A_10P(0xCC,0x00,0x00,0x09,0x0B,0x01,0x25,0x26,0x00,0x00,0x00);
	W_COM_1A_1P(0x00,0x90);
	W_COM_1A_15P(0xCC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0A,0x0C,0x02);
	W_COM_1A_1P(0x00,0xA0);
	W_COM_1A_15P(0xCC,0x25,0x26,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);
	W_COM_1A_1P(0x00,0x80);
	W_COM_1A_12P(0xCE,0x86,0x01,0x06,0x85,0x01,0x06,0x0F,0x00,0x00,0x0f,0x00,0x00);
	W_COM_1A_1P(0x00,0x90);
	W_COM_1A_14P(0xCE,0xF0,0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00,0x00,0x00);
	W_COM_1A_1P(0x00,0xA0);
	W_COM_1A_14P(0xCE,0x18,0x05,0x03,0xC0,0x00,0x06,0x00,0x18,0x04,0x03,0xC1,0x00,0x06,0x00);
	W_COM_1A_1P(0x00,0xB0);
	W_COM_1A_14P(0xCE,0x18,0x03,0x03,0xC2,0x00,0x06,0x00,0x18,0x02,0x03,0xC3,0x00,0x06,0x00);
	W_COM_1A_1P(0x00,0xC0);
	W_COM_1A_14P(0xCE,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00);
	W_COM_1A_1P(0x00,0xD0);
	W_COM_1A_14P(0xCE,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00);
	W_COM_1A_1P(0x00,0x80);
	W_COM_1A_14P(0xCF,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00);
	W_COM_1A_1P(0x00,0x90);
	W_COM_1A_14P(0xCF,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00);
	W_COM_1A_1P(0x00,0xA0);
	W_COM_1A_14P(0xCF,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00);
	W_COM_1A_1P(0x00,0xB0);
	W_COM_1A_14P(0xCF,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00);
	W_COM_1A_1P(0x00,0xC0);
	W_COM_1A_10P(0xCF,0x02,0x02,0x10,0x10,0x00,0x00,0x01,0x81,0x00,0x08);

	////////////////////// CABC CODE /////////////////////////////////


	////////////////////// CABC CODE END //////////////////////////////
	//////////////////////////////////////
	W_COM_1A_1P(0x00,0xB1);
	W_COM_1A_1P(0xC5,0x28);

	W_COM_1A_1P(0x00,0x80);
	W_COM_1A_1P(0xC4,0x9C);

	W_COM_1A_1P(0x00,0xC0);
	W_COM_1A_1P(0xC5,0x00);

	W_COM_1A_1P(0x00,0xB2);
	W_COM_1A_4P(0xF5,0x15,0x00,0x15,0x00);

	W_COM_1A_1P(0x00,0x93);
	W_COM_1A_1P(0xC5,0x03);

	W_COM_1A_1P(0x00,0x80);
	W_COM_1A_2P(0xC1,0x36,0x66);

	W_COM_1A_1P(0x00,0x89);
	W_COM_1A_1P(0xC0,0x01);

	W_COM_1A_1P(0x00,0xA0);
	W_COM_1A_1P(0xC1,0x00);

	W_COM_1A_1P(0x00,0xC5);
	W_COM_1A_1P(0xB0,0x03);
	///////////////////////////////////////
	W_COM_1A_1P(0x00,0x00);
//	W_COM_1A_1P(0xFF,0xFF,0xFF,0xFF);



	W_COM_1A_0P(0x11);
	msleep(120);
	W_COM_1A_0P(0x29);

	return 0;
}

static int nt35517_detect(void){
	unsigned long read_data;
	int i, j, err;
	u32 buf[32];

	buf[0] = 0xF0 | (0x55 << 8) | (0xAA<<16) |(0x52 << 24) ;
	buf[1] = 0x08 | (0x00<<8);
	err = mipi_dsi_pkt_write(MIPI_DSI_GENERIC_LONG_WRITE,
					buf, 0x6);

	//read ID
	buf[0] = MIPI_DSI_MAX_RET_PACK_SIZE;
	err = mipi_dsi_pkt_write(
				MIPI_DSI_SET_MAXIMUM_RETURN_PACKET_SIZE,
				buf, 0);
	buf[0] = 0x4 ;
	err =  mipi_dsi_pkt_read(MIPI_DSI_GENERIC_READ_REQUEST_2_PARAM,
			buf, 0x4);
	if (!err && ((buf[0] & 0xffff) == 0x8000)) {
		printf("nt35517 found!\n");
		return 1;
	}
	return 0;
}
static int nt35517_init(void){
	unsigned long read_data;
	int i, j, err;
	u32 buf[32];
	//*************************************
	// Select CMD2, Page 0
	//*************************************
	//REGW 0xF0, 0x55, 0xAA, 0x52, 0x08, 0x00
	buf[0] = 0xF0 | (0x55 << 8) | (0xAA<<16) |(0x52 << 24) ;
	buf[1] = 0x08 | (0x00<<8);
	err = mipi_dsi_pkt_write(MIPI_DSI_GENERIC_LONG_WRITE,
					buf, 0x6);
	CHECK_RETCODE(err);

	/*
	 *Forward Scan		CTB=CRL=0
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


	/* Set pixel format:24bpp ,add by allenyao*/
	buf[0] = 0x3A;
	buf[0] |= (0x70 << 8);
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM,
			buf, 0);
	CHECK_RETCODE(err);

	// Display Timing: Dual 8-phase 4-overlap
	//REGW 0xC9, 0x61, 0x06, 0x0D, 0x17, 0x17, 0x00
	buf[0] = 0xC9 |(0x61 << 8) |(0x06 << 16) |(0x0D << 24)	;
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

	// VMSEL 0: 0xBE00	;  1 : 0xBF00
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
	//REGW 0xD1,0x00,0x00,0x00, 0x62,0x00,0x90,0x00,	0xAE,0x00,0xC5,0x00,	0xEA,0x01,0x07,0x01,	0x34
	buf[0] = 0xD1 |(0x00 << 8) |(0x00 << 16)|(0x00 << 24);
	buf[1] = 0x62 |(0x00 << 8) |(0x90 << 16)|(0x00 << 24);
	buf[2] = 0xAE |(0x00 << 8) |(0xC5 << 16)|(0x00 << 24);
	buf[3] = 0xEA |(0x01 << 8) |(0x07 << 16)|(0x01 << 24);
	buf[4] = 0x34 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xD2,0x01,0x58,0x01, 0x8F,0x01,0xBB,0x01,	0xFF,0x02,0x37,0x02,	0x39,0x02,0x6E,0x02,	0xA9
	buf[0] = 0xD2 |(0x01 << 8) |(0x58 << 16)|(0x01 << 24);
	buf[1] = 0x8F |(0x01 << 8) |(0xBB << 16)|(0x01 << 24);
	buf[2] = 0xFF |(0x02 << 8) |(0x37 << 16)|(0x02 << 24);
	buf[3] = 0x39 |(0x02 << 8) |(0x6E << 16)|(0x02 << 24);
	buf[4] = 0xA9 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xD3,0x02,0xD0,0x03, 0x07,0x03,0x2C,0x03,	0x5F,0x03,0x81,0x03 ,0xAC,0x03,0xC3,0x03,	0xDD
	buf[0] = 0xD3 |(0x02 << 8) |(0xD0 << 16)|(0x03 << 24);
	buf[1] = 0x07 |(0x03 << 8) |(0x2C << 16)|(0x03 << 24);
	buf[2] = 0x5F |(0x03 << 8) |(0x81 << 16)|(0x03 << 24);
	buf[3] = 0xAC |(0x03 << 8) |(0xC3 << 16)|(0x03 << 24);
	buf[4] = 0xDD ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xD4,0x03,0xF6,0x03, 0xFB
	buf[0] = 0xD4 |(0x03 << 8) |(0xF6 << 16)|(0x03 << 24);
	buf[1] = 0xFB ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x5);
	CHECK_RETCODE(err);

//G(+) MCR cmd
	//REGW 0xD5,0x00,0x00,0x00, 0x62,0x00,0x90,0x00,	0xAE,0x00,0xC5,0x00,	0xEA,0x01,0x07,0x01,	0x34
	buf[0] = 0xD5 |(0x00 << 8) |(0x00 << 16)|(0x00 << 24);
	buf[1] = 0x62 |(0x00 << 8) |(0x90 << 16)|(0x00 << 24);
	buf[2] = 0xAE |(0x00 << 8) |(0xC5 << 16)|(0x00 << 24);
	buf[3] = 0xEA |(0x01 << 8) |(0x07 << 16)|(0x01 << 24);
	buf[4] = 0x34 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xD6,0x01,0x58,0x01, 0x8F,0x01,0xBB,0x01,	0xFF,0x02,0x37,0x02,	0x39,0x02,0x6E,0x02,	0xA9
	buf[0] = 0xD6 |(0x01 << 8) |(0x58 << 16)|(0x01 << 24);
	buf[1] = 0x8F |(0x01 << 8) |(0xBB << 16)|(0x01 << 24);
	buf[2] = 0xFF |(0x02 << 8) |(0x37 << 16)|(0x02 << 24);
	buf[3] = 0x39 |(0x02 << 8) |(0x6E << 16)|(0x02 << 24);
	buf[4] = 0xA9 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xD7,0x02,0xD0,0x03, 0x07,0x03,0x2C,0x03,	0x5F,0x03,0x81,0x03,	0xAC,0x03,0xC3,0x03,	0xDD
	buf[0] = 0xD7 |(0x02 << 8) |(0xD0 << 16)|(0x03 << 24);
	buf[1] = 0x07 |(0x03 << 8) |(0x2C << 16)|(0x03 << 24);
	buf[2] = 0x5F |(0x03 << 8) |(0x81 << 16)|(0x03 << 24);
	buf[3] = 0xAC |(0x03 << 8) |(0xC3 << 16)|(0x03 << 24);
	buf[4] = 0xDD ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xD8,0x03,0xF6,0x03, 0xFB
	buf[0] = 0xD8 |(0x03 << 8) |(0xF6 << 16)|(0x03 << 24);
	buf[1] = 0xFB ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x5);
	CHECK_RETCODE(err);

//B(+) MCR cmd
	//REGW 0xD9,0x00,0x00,0x00, 0x62,0x00,0x90,0x00,	0xAE,0x00,0xC5,0x00,	0xEA,0x01,0x07,0x01,	0x34
	buf[0] = 0xD9 |(0x00 << 8) |(0x00 << 16)|(0x00 << 24);
	buf[1] = 0x62 |(0x00 << 8) |(0x90 << 16)|(0x00 << 24);
	buf[2] = 0xAE |(0x00 << 8) |(0xC5 << 16)|(0x00 << 24);
	buf[3] = 0xEA |(0x01 << 8) |(0x07 << 16)|(0x01 << 24);
	buf[4] = 0x34 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xDD,0x01,0x58,0x01, 0x8F,0x01,0xBB,0x01,	0xFF,0x02,0x37,0x02,	0x39,0x02,0x6E,0x02,	0xA9
	buf[0] = 0xDD |(0x01 << 8) |(0x58 << 16)|(0x01 << 24);
	buf[1] = 0x8F |(0x01 << 8) |(0xBB << 16)|(0x01 << 24);
	buf[2] = 0xFF |(0x02 << 8) |(0x37 << 16)|(0x02 << 24);
	buf[3] = 0x39 |(0x02 << 8) |(0x6E << 16)|(0x02 << 24);
	buf[4] = 0xA9 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xDE,0x02,0xD0,0x03, 0x07,0x03,0x2C,0x03,	0x5F,0x03,0x81,0x03,	0xAC,0x03,0xC3,0x03,	0xDD
	buf[0] = 0xDE |(0x02 << 8) |(0xD0 << 16)|(0x03 << 24);
	buf[1] = 0x07 |(0x03 << 8) |(0x2C << 16)|(0x03 << 24);
	buf[2] = 0x5F |(0x03 << 8) |(0x81 << 16)|(0x03 << 24);
	buf[3] = 0xAC |(0x03 << 8) |(0xC3 << 16)|(0x03 << 24);
	buf[4] = 0xDD ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xDF,0x03,0xF6,0x03, 0xFB
	buf[0] = 0xDF |(0x03 << 8) |(0xF6 << 16)|(0x03 << 24);
	buf[1] = 0xFB ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x5);
	CHECK_RETCODE(err);

//R(-) MCR cmd
	//REGW 0xE0,0x00,0x00,0x00, 0x62,0x00,0x90,0x00,	0xAE,0x00,0xC5,0x00,	0xEA,0x01,0x07,0x01,	0x34
	buf[0] = 0xE0 |(0x00 << 8) |(0x00 << 16)|(0x00 << 24);
	buf[1] = 0x62 |(0x00 << 8) |(0x90 << 16)|(0x00 << 24);
	buf[2] = 0xAE |(0x00 << 8) |(0xC5 << 16)|(0x00 << 24);
	buf[3] = 0xEA |(0x01 << 8) |(0x07 << 16)|(0x01 << 24);
	buf[4] = 0x34 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xE1,0x01,0x58,0x01, 0x8F,0x01,0xBB,0x01,	0xFF,0x02,0x37,0x02,	0x39,0x02,0x6E,0x02,	0xA9
	buf[0] = 0xE1 |(0x01 << 8) |(0x58 << 16)|(0x01 << 24);
	buf[1] = 0x8F |(0x01 << 8) |(0xBB << 16)|(0x01 << 24);
	buf[2] = 0xFF |(0x02 << 8) |(0x37 << 16)|(0x02 << 24);
	buf[3] = 0x39 |(0x02 << 8) |(0x6E << 16)|(0x02 << 24);
	buf[4] = 0xA9 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xE2,0x02,0xD0,0x03, 0x07,0x03,0x2C,0x03,	0x5F,0x03,0x81,0x03,	0xAC,0x03,0xC3,0x03,	0xDD
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
	//REGW 0xE4,0x00,0x00,0x00, 0x62,0x00,0x90,0x00,	0xAE,0x00,0xC5,0x00,	0xEA,0x01,0x07,0x01,	0x34
	buf[0] = 0xE4 |(0x00 << 8) |(0x00 << 16)|(0x00 << 24);
	buf[1] = 0x62 |(0x00 << 8) |(0x90 << 16)|(0x00 << 24);
	buf[2] = 0xAE |(0x00 << 8) |(0xC5 << 16)|(0x00 << 24);
	buf[3] = 0xEA |(0x01 << 8) |(0x07 << 16)|(0x01 << 24);
	buf[4] = 0x34 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xE5,0x01,0x58,0x01, 0x8F,0x01,0xBB,0x01,	0xFF,0x02,0x37,0x02,	0x39,0x02,0x6E,0x02,	0xA9
	buf[0] = 0xE5 |(0x01 << 8) |(0x58 << 16)|(0x01 << 24);
	buf[1] = 0x8F |(0x01 << 8) |(0xBB << 16)|(0x01 << 24);
	buf[2] = 0xFF |(0x02 << 8) |(0x37 << 16)|(0x02 << 24);
	buf[3] = 0x39 |(0x02 << 8) |(0x6E << 16)|(0x02 << 24);
	buf[4] = 0xA9 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xE6,0x02,0xD0,0x03, 0x07,0x03,0x2C,0x03,	0x5F,0x03,0x81,0x03,	0xAC,0x03,0xC3,0x03,	0xDD
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
	//REGW 0xE8,0x00,0x00,0x00, 0x62,0x00,0x90,0x00,	0xAE,0x00,0xC5,0x00,	0xEA,0x01,0x07,0x01,	0x34
	buf[0] = 0xE8 |(0x00 << 8) |(0x00 << 16)|(0x00 << 24);
	buf[1] = 0x62 |(0x00 << 8) |(0x90 << 16)|(0x00 << 24);
	buf[2] = 0xAE |(0x00 << 8) |(0xC5 << 16)|(0x00 << 24);
	buf[3] = 0xEA |(0x01 << 8) |(0x07 << 16)|(0x01 << 24);
	buf[4] = 0x34 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xE9,0x01,0x58,0x01, 0x8F,0x01,0xBB,0x01,	0xFF,0x02,0x37,0x02,	0x39,0x02,0x6E,0x02,	0xA9
	buf[0] = 0xE9 |(0x01 << 8) |(0x58 << 16)|(0x01 << 24);
	buf[1] = 0x8F |(0x01 << 8) |(0xBB << 16)|(0x01 << 24);
	buf[2] = 0xFF |(0x02 << 8) |(0x37 << 16)|(0x02 << 24);
	buf[3] = 0x39 |(0x02 << 8) |(0x6E << 16)|(0x02 << 24);
	buf[4] = 0xA9 ;
	err = mipi_dsi_pkt_write( MIPI_DSI_GENERIC_LONG_WRITE,
		buf, 0x11);
	CHECK_RETCODE(err);
	//REGW 0xEA,0x02,0xD0,0x03, 0x07,0x03,0x2C,0x03,	0x5F,0x03,0x81,0x03,	0xAC,0x03,0xC3,0x03,	0xDD
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

	return err;
}

struct mipi_panel_info{
	const char* name;
	int (*detect)(void);
	int (*init)(void);
};
static struct mipi_panel_info mipi_panels[]={
	{"OT-QHD",otm9605a_detect,otm9605a_init},
	{"NT-QHD",nt35517_detect,nt35517_init},
};

int mipi_panel_init(char** panel_name){
	int i;
	for(i=0;i<sizeof(mipi_panels)/sizeof(mipi_panels[0]);i++){
		if(mipi_panels[i].detect()){
			mipi_panels[i].init();
			*panel_name = mipi_panels[i].name;
			return 0;
		}
	}
	//reach here means no right panel detected
	printf("no mipi panel detectd\n");
	return 1;
}

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
	
	return err;
}
