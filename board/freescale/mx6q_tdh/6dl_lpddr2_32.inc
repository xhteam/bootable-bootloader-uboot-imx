dcd_hdr:          .word 0x404802D2 /* Tag=0xD2, Len=72*8 + 4 + 4, Ver=0x40 */
write_dcd_cmd:    .word 0x044402CC /* Tag=0xCC, Len=72*8 + 4, Param=0x04 */

/* DCD */
/*DDR clk to 400MHz*/
MXC_DCD_ITEM(1, CCM_BASE_ADDR + 0x18, 0x60324)

//DDR IO Type
MXC_DCD_ITEM(2, IOMUXC_BASE_ADDR + 0x774, 0x00080000)
MXC_DCD_ITEM(3, IOMUXC_BASE_ADDR + 0x754, 0x00000000)

//CLOCK
MXC_DCD_ITEM(4, IOMUXC_BASE_ADDR + 0x4ac, 0x00000038)
MXC_DCD_ITEM(5, IOMUXC_BASE_ADDR + 0x4b0, 0x00000038)
//ADDRESS
MXC_DCD_ITEM(6, IOMUXC_BASE_ADDR + 0x464, 0x00000038)
MXC_DCD_ITEM(7, IOMUXC_BASE_ADDR + 0x490, 0x00000038)
MXC_DCD_ITEM(8, IOMUXC_BASE_ADDR + 0x74c, 0x00000038)
//Control
MXC_DCD_ITEM(9, IOMUXC_BASE_ADDR + 0x494, 0x00000038)
MXC_DCD_ITEM(10, IOMUXC_BASE_ADDR + 0x4a0, 0x00000000)
MXC_DCD_ITEM(11, IOMUXC_BASE_ADDR + 0x4b4, 0x00000038)
MXC_DCD_ITEM(12, IOMUXC_BASE_ADDR + 0x4b8, 0x00000038)
MXC_DCD_ITEM(13, IOMUXC_BASE_ADDR + 0x76c, 0x00000038)

//Data Strobes
MXC_DCD_ITEM(14, IOMUXC_BASE_ADDR + 0x750, 0x00020000)
MXC_DCD_ITEM(15, IOMUXC_BASE_ADDR + 0x4bc, 0x00003038)
MXC_DCD_ITEM(16, IOMUXC_BASE_ADDR + 0x4c0, 0x00003038)
MXC_DCD_ITEM(17, IOMUXC_BASE_ADDR + 0x4c4, 0x00003038)
MXC_DCD_ITEM(18, IOMUXC_BASE_ADDR + 0x4c8, 0x00003038)

//Data
MXC_DCD_ITEM(19, IOMUXC_BASE_ADDR + 0x760, 0x00020000)
MXC_DCD_ITEM(20, IOMUXC_BASE_ADDR + 0x764, 0x00000038)
MXC_DCD_ITEM(21, IOMUXC_BASE_ADDR + 0x770, 0x00000038)
MXC_DCD_ITEM(22, IOMUXC_BASE_ADDR + 0x778, 0x00000038)
MXC_DCD_ITEM(23, IOMUXC_BASE_ADDR + 0x77c, 0x00000038)

MXC_DCD_ITEM(24, IOMUXC_BASE_ADDR + 0x470, 0x00000038)
MXC_DCD_ITEM(25, IOMUXC_BASE_ADDR + 0x474, 0x00000038)
MXC_DCD_ITEM(26, IOMUXC_BASE_ADDR + 0x478, 0x00000038)
MXC_DCD_ITEM(27, IOMUXC_BASE_ADDR + 0x47c, 0x00000038)

//=============================================================================			
// DDR Controller Registers			
//=============================================================================			
// Manufacturer:	SamSung			
// Device Part Number:	KMKUS000VM			
// Clock Freq.: 	400MHz		
// MMDC channels: 	MMDC0		
// Density per CS in Gb: 	4		
// Chip Selects used:	2		
// Number of Banks:	8		
// Row address:    	14		
// Column address: 	10		
// Data bus width	32		
//=============================================================================	
//MDSCR		
MXC_DCD_ITEM(28, MMDC_P0_BASE_ADDR + 0x01c, 0x00008000)
//MPZQLP2CTL
MXC_DCD_ITEM(29, MMDC_P0_BASE_ADDR + 0x85c, 0x1B4700C7)

/*========================================================================*/
/* Calibration setup.*/
/**/
/*========================================================================*/
//DDR_PHY_P0_MPZQHWCTRL
MXC_DCD_ITEM(30, MMDC_P0_BASE_ADDR + 0x800, 0xA1390003)

//ca bus abs delay
MXC_DCD_ITEM(31, MMDC_P0_BASE_ADDR + 0x890, 0x00400000)


MXC_DCD_ITEM(32, MMDC_P0_BASE_ADDR + 0x8b8, 0x00000800)

//read data bit delay: (3 is the reccommended default value, although out of reset value is 0)			
MXC_DCD_ITEM(33, MMDC_P0_BASE_ADDR + 0x81c, 0x33333333)
MXC_DCD_ITEM(34, MMDC_P0_BASE_ADDR + 0x820, 0x33333333)
MXC_DCD_ITEM(35, MMDC_P0_BASE_ADDR + 0x824, 0x33333333)
MXC_DCD_ITEM(36, MMDC_P0_BASE_ADDR + 0x828, 0x33333333)
//write data bit delay: (3 is the reccommended default value, although out of reset value is 0)			
MXC_DCD_ITEM(37, MMDC_P0_BASE_ADDR + 0x82c, 0xF3333333)
MXC_DCD_ITEM(38, MMDC_P0_BASE_ADDR + 0x830, 0xF3333333)
MXC_DCD_ITEM(39, MMDC_P0_BASE_ADDR + 0x834, 0xF3333333)
MXC_DCD_ITEM(40, MMDC_P0_BASE_ADDR + 0x838, 0xF3333333)

//MXC_DCD_ITEM(44, MMDC_P0_BASE_ADDR + 0x8c0, 0x24911492)
//Read calibration
MXC_DCD_ITEM(41, MMDC_P0_BASE_ADDR + 0x848, 0x3C363636)

//Write calibration
MXC_DCD_ITEM(42, MMDC_P0_BASE_ADDR + 0x850, 0x34343A3A)


/*dqs gating dis*/
MXC_DCD_ITEM(43, MMDC_P0_BASE_ADDR + 0x83c, 0x20000000)
MXC_DCD_ITEM(44, MMDC_P0_BASE_ADDR + 0x840, 0x00000000)
//For i.mx6qd parts of versions A & B (v1.0, v1.1), uncomment the following lines. For version C (v1.2), keep commented			
//setmem /32	0x021b08c0 =	0x24911492	// fine tune SDCLK duty cyc to low - seen to improve measured duty cycle of i.mx6
//setmem /32	0x021b48c0 =	0x24911492	
			
// Complete calibration by forced measurement:                 			

/*frc_msr*/
MXC_DCD_ITEM(45, MMDC_P0_BASE_ADDR + 0x8b8, 0x00000800)

/*========================================================================*/
/* Calibration setup end*/
/*========================================================================*/


// MMDC0_MDCFG0
MXC_DCD_ITEM(46, MMDC_P0_BASE_ADDR + 0xc, 0x33374135)
/*MMDC0_MDPDC - where is tCKSRX and tCKSRE defined in LPDDR2 data sheet?????*/
MXC_DCD_ITEM(47, MMDC_P0_BASE_ADDR + 0x4, 0x00020036)

// MMDC0_MDCFG1
MXC_DCD_ITEM(48, MMDC_P0_BASE_ADDR + 0x10, 0x00100A83)
// MMDC0_MDCFG2
MXC_DCD_ITEM(49, MMDC_P0_BASE_ADDR + 0x14, 0x00000093)
//MDMISC: consider reducing RALAT if your 528MHz board design allow that. Lower RALAT benefits: 			
//a. better operation at low frequency, for LPDDR2 freq < 100MHz, change RALAT to 3			
//b. Small performence improvment 	
MXC_DCD_ITEM(50, MMDC_P0_BASE_ADDR + 0x18, 0x00001688)
MXC_DCD_ITEM(51, MMDC_P0_BASE_ADDR + 0x2c, 0x0F9F26D2)
MXC_DCD_ITEM(52, MMDC_P0_BASE_ADDR + 0x30, 0x0000020e)
//MXC_DCD_ITEM(53, MMDC_P0_BASE_ADDR + 0x1c, 0x00008000)

MXC_DCD_ITEM(53, MMDC_P0_BASE_ADDR + 0x38, 0x00190778)

MXC_DCD_ITEM(54, MMDC_P0_BASE_ADDR + 0x8, 0x0)

MXC_DCD_ITEM(55, MMDC_P0_BASE_ADDR + 0x40, 0x00000017)
//MXC_DCD_ITEM(56, MMDC_P0_BASE_ADDR + 0x400, 0x11420000)
MXC_DCD_ITEM(56, MMDC_P0_BASE_ADDR + 0x00, 0xC3110000)

/* Channel0 : Configure DDR device:*/
//CS0
/*MRW: BA=0 CS=0 MR_ADDR=63 MR_OP=0  //reset*/
MXC_DCD_ITEM(57, MMDC_P0_BASE_ADDR + 0x01c, 0x003f8030)
/*MRW: BA=0 CS=0 MR_ADDR=10 MR_OP=ff /zq*/
MXC_DCD_ITEM(58, MMDC_P0_BASE_ADDR + 0x01c, 0xFF0A8030)
/*MRW: BA=0 CS=0 MR_ADDR=1  MR_OP=c2*/
MXC_DCD_ITEM(59, MMDC_P0_BASE_ADDR + 0x01c, 0x62018030)
/*MRW: BA=0 CS=0 MR_ADDR=2  MR_OP=4. tcl=6, tcwl=3*/
MXC_DCD_ITEM(60, MMDC_P0_BASE_ADDR + 0x01c, 0x06028030)
/*MRW: BA=0 CS=0 MR_ADDR=3  MR_OP=2.drive=240/6*/
MXC_DCD_ITEM(61, MMDC_P0_BASE_ADDR + 0x01c, 0x01038030)
//CS1		
/* MRW: BA=0 CS=1 MR_ADDR=63 MR_OP=0*/
/*reset*/
//MXC_DCD_ITEM(64, MMDC_P0_BASE_ADDR + 0x01c, 0x003f8038)
/*MRW: BA=0 CS=1 MR_ADDR=10 MR_OP=ff*/
MXC_DCD_ITEM(62, MMDC_P0_BASE_ADDR + 0x01c, 0xFF0A8038)
/*MRW: BA=0 CS=1 MR_ADDR=1  MR_OP=c2*/
MXC_DCD_ITEM(63, MMDC_P0_BASE_ADDR + 0x01c, 0x62018038)
/*MRW: BA=0 CS=1 MR_ADDR=2  MR_OP=4. tcl=6, tcwl=3*/
MXC_DCD_ITEM(64, MMDC_P0_BASE_ADDR + 0x01c, 0x06028038)
/*MRW: BA=0 CS=1 MR_ADDR=3  MR_OP=2.drive=240/6*/
MXC_DCD_ITEM(65, MMDC_P0_BASE_ADDR + 0x01c, 0x01038038)

/*######################################################*/
/*final DDR setup, before operation start:*/
MXC_DCD_ITEM(66, MMDC_P0_BASE_ADDR + 0x800, 0xA1390003)
MXC_DCD_ITEM(67, MMDC_P0_BASE_ADDR + 0x020, 0x00001800)
MXC_DCD_ITEM(68, MMDC_P0_BASE_ADDR + 0x818, 0x00000000)
/*DDR_PHY_P0_MPMUR0, frc_msr*/
MXC_DCD_ITEM(69, MMDC_P0_BASE_ADDR + 0x8b8, 0x00000800)
MXC_DCD_ITEM(70, MMDC_P0_BASE_ADDR + 0x004, 0x00025576)
MXC_DCD_ITEM(71, MMDC_P0_BASE_ADDR + 0x404, 0x00011006)
MXC_DCD_ITEM(72, MMDC_P0_BASE_ADDR + 0x01c, 0x00000000)
/*###################put the ddr script here ######################*/

