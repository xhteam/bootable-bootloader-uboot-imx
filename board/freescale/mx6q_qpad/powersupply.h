/*
 * porting from kernel qpower driver
*/
#ifndef POWERSUPPLY_H
#define POWERSUPPLY_H
typedef struct qpower_charger_pdata
{	
	int cen;	/* Charger Enable input */
	int dok;	/* DC(Adapter) Power OK output */
	int uok;	/* USB Power OK output */
	int chg;	/* Charger status output */
	int flt;	/* Fault output */
	int dcm;	/* Current-Limit Mode input (1: DC, 2: USB) */
	int usus;	/* USB Suspend Input (1: suspended) */
	int det;	/* Battery Detection*/

	int fuelgauge_bus;
	int fuelgauge_addr;
}qpower_charger_pdata;

int powersupply_init(qpower_charger_pdata * pdata);

//return dc status
int powersupply_dok(void);

//return dc status
int powersupply_uok(void);

//return charge status
int powersupply_chg(void);

//return charge fault status
int powersupply_flt(void);

//return battery probe status
int powersupply_bat(void);





#endif 

