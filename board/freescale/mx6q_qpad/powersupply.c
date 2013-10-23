#include <common.h>
#include <malloc.h>
#include "powersupply.h"

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
		
		qpower_init++;
		return 0;
	}
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

