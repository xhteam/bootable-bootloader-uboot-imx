#include <common.h>
#include <malloc.h>
#include "powersupply.h"

static qpower_charger_pdata psd;
static int qpower_init;
int powersupply_init(qpower_charger_pdata * pdata){
	if(pdata){
		memcpy(&psd,pdata,sizeof(qpower_charger_pdata));


		
		qpower_init++;
		return 0;
	}
	return -1;
}

//return dc status
int powersupply_dok(void){
	return 1;
	
}

//return charge status
int powersupply_chg(void){
	return 1;
}

//return charge fault status
int powersupply_flt(void){
	return 0;
}

//return battery probe status
int powersupply_bat(void){
	return 1;
}

