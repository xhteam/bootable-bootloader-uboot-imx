#include <common.h>
#include <malloc.h>
#include <linux/types.h>
#include <asm/io.h>
#if CONFIG_I2C_MXC
#include <i2c.h>
#endif

#include "powersupply.h"

/*
 *All max17058 register is 16bit,
*/
#define REG_OFFSET_VCELL	0x02
#define REG_OFFSET_SOC		0x04
#define REG_OFFSET_MODE		0x06
#define REG_OFFSET_VER		0x08
#define REG_OFFSET_HIBERNATE 0x0A
#define REG_OFFSET_CONFIG	0x0C
#define REG_OFFSET_OCV		0x0E
#define REG_OFFSET_VRESET	0x18
#define REG_OFFSET_STATUS	0x1A
#define REG_OFFSET_CMD		0xFE


#define REG_OFFSET_TABLE_ACCESS	0x3E
#define REG_OFFSET_TABLE_START	0x40
#define REG_OFFSET_TABLE_END	0x7F


//REG VCELL
#define REG_VCELL_SCALE 		78125 /*78.125uV/cell*/

//REG SOC
#define REG_SOC_SCALE			256		/*1%/256*/

//REG MODE
#define REG_MODE_QUICKSTART 	0x4000
#define REG_MODE_SLEEPENABLE    0x2000

//REG CONFIG
#define REG_CONFIG_RCOMP_MASK		0xFF00
#define REG_CONFIG_SLEEP			0x80
#define REG_CONFIG_ALERT			0x20
#define REG_CONFIG_ALERT_THRS_MASK	0x1F

//REG VRESET

//REG STATUS
#define REG_STATUS_RI				0x100	/*Reset Indicator*/	


//REG TABLE LOCK/UNLOCK
#define REG_TABLE_UNLOCK_PATTERN		0x4A57
#define REG_TABLE_LOCK_PATTERN			0x0000
#define REG_TABLE_SIZE				64

#define MAX17058_POLLING_DELAY		(1000)   
#define MAX17058_BATTERY_FULL		100


static qpower_charger_pdata psd;
static int qpower_init;
int powersupply_init(qpower_charger_pdata * pdata){
	if(pdata){
		memcpy(&psd,pdata,sizeof(qpower_charger_pdata));


		if(psd.dok)
			gpio_direction_input(psd.dok);
		if(psd.uok)
			gpio_direction_input(psd.uok);
		if(psd.chg)
			gpio_direction_input(psd.chg);
		if(psd.flt)
			gpio_direction_input(psd.flt);
		if(psd.det)
			gpio_direction_input(psd.det);
		
		i2c_init(CONFIG_SYS_I2C_SPEED, 0);

		qpower_init++;
		return 0;
	}
	return -1;
}

static int chip_write_word(uchar chip, int reg, u16 value)
{
	value = __swab16(value);
    return i2c_write(chip,reg,1,&value,2);
}

static int chip_read_word(uchar chip, int reg)
{
	int value;
	int ret;

	i2c_write(chip,reg,1,0,0);

	ret = i2c_read(chip, reg,1,&value,2);
	if(!ret)
	    return __swab16((uint16_t)(value&0xffff));
	return -1;
}

//return dc status
int powersupply_dok(void){
	int dok=0;
	if(psd.dok)
		dok=(gpio_get_value(psd.dok)>0)?0:1;

	return dok;
	
}
int powersupply_uok(void){
	int uok=0;
	if(psd.uok)
		uok=(gpio_get_value(psd.uok)>0)?0:1;

	return uok;	
}


//return charge status
int powersupply_chg(void){
	int chg=0;
	if(psd.chg){
		chg=(gpio_get_value(psd.chg)>0)?0:1;
	}

	return chg;	
}

//return charge fault status
int powersupply_flt(void){
	int flt=0;
	if(psd.flt)
		flt=(gpio_get_value(psd.flt)>0)?0:1;

	return flt;	
}

//return battery probe status
int powersupply_bat(void){
	return 1;
}

//return battery soc
int powersupply_soc(void){
	int soc;
	if(!psd.fuelgauge_addr||i2c_probe(psd.fuelgauge_addr)){
		printf("fuelgauge not found\n");
		return -1;
	}
	
 	soc =  chip_read_word(psd.fuelgauge_addr, REG_OFFSET_SOC);//16bit  read

 	if(soc>=0)	soc >>=9;

	if(soc>100)
		soc=100;
	return soc;
 	
}


