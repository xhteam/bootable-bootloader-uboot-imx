dcd_hdr:          .word 0x428002D2 /* Tag=0xD2, Len=68*8 + 4 + 4, Ver=0x40 */
write_dcd_cmd:    .word 0x042402CC /* Tag=0xCC, Len=68*8 + 4, Param=0x04 */

/* DCD */
/*DDR clk to 400MHz*/
MXC_DCD_ITEM(1, CCM_BASE_ADDR + 0x18, 0x60324)

//DDR IO Type
MXC_DCD_ITEM(2, IOMUXC_BASE_ADDR + 0x798, 0x00080000)
MXC_DCD_ITEM(3, IOMUXC_BASE_ADDR + 0x758, 0x00000000)

//CLOCK
MXC_DCD_ITEM(4, IOMUXC_BASE_ADDR + 0x588, 0x00000030)
MXC_DCD_ITEM(5, IOMUXC_BASE_ADDR + 0x594, 0x00000030)
//ADDRESS
MXC_DCD_ITEM(6, IOMUXC_BASE_ADDR + 0x56c, 0x00000030)
MXC_DCD_ITEM(7, IOMUXC_BASE_ADDR + 0x578, 0x00000030)
MXC_DCD_ITEM(8, IOMUXC_BASE_ADDR + 0x74c, 0x00000030)
//Control
MXC_DCD_ITEM(9, IOMUXC_BASE_ADDR + 0x57c, 0x00000030)

MXC_DCD_ITEM(10, IOMUXC_BASE_ADDR + 0x58c, 0x00000000)
MXC_DCD_ITEM(11, IOMUXC_BASE_ADDR + 0x59c, 0x00000030)
MXC_DCD_ITEM(12, IOMUXC_BASE_ADDR + 0x5a0, 0x00000030)
MXC_DCD_ITEM(13, IOMUXC_BASE_ADDR + 0x78c, 0x00000030)

//Data Strobes
MXC_DCD_ITEM(14, IOMUXC_BASE_ADDR + 0x750, 0x00020000)
MXC_DCD_ITEM(15, IOMUXC_BASE_ADDR + 0x5a8, 0x00003030)
MXC_DCD_ITEM(16, IOMUXC_BASE_ADDR + 0x5b0, 0x00003030)
MXC_DCD_ITEM(17, IOMUXC_BASE_ADDR + 0x524, 0x00003030)
MXC_DCD_ITEM(18, IOMUXC_BASE_ADDR + 0x51c, 0x00003030)

//Data
MXC_DCD_ITEM(19, IOMUXC_BASE_ADDR + 0x784, 0x00000030)
MXC_DCD_ITEM(20, IOMUXC_BASE_ADDR + 0x788, 0x00000030)
MXC_DCD_ITEM(21, IOMUXC_BASE_ADDR + 0x794, 0x00000030)
MXC_DCD_ITEM(22, IOMUXC_BASE_ADDR + 0x79c, 0x00000030)

MXC_DCD_ITEM(23, IOMUXC_BASE_ADDR + 0x5ac, 0x00000030)
MXC_DCD_ITEM(24, IOMUXC_BASE_ADDR + 0x5b4, 0x00000030)
MXC_DCD_ITEM(25, IOMUXC_BASE_ADDR + 0x528, 0x00000030)
MXC_DCD_ITEM(26, IOMUXC_BASE_ADDR + 0x520, 0x00000030)

//=============================================================================			
// DDR Controller Registers			
//=============================================================================			
// Manufacturer:	SamSung(eMCP)		
// Device Part Number:	KMKUS000
// Clock Freq.: 	400MHz		
// MMDC channels: 	MMDC0		
// Density per CS in Gb: 	4		
// Chip Selects used:	2		
// Number of Banks:	8		
// Row address:    	14		
// Column address: 	10		
// Data bus width	32		
//=============================================================================			
MXC_DCD_ITEM(27, MMDC_P0_BASE_ADDR + 0x01c, 0x00008000)
MXC_DCD_ITEM(28, MMDC_P0_BASE_ADDR + 0x85c, 0x1B5F0107)

/*========================================================================*/
/* Calibration setup.*/
/**/
/*========================================================================*/
MXC_DCD_ITEM(29, MMDC_P0_BASE_ADDR + 0x800, 0xa1390003)
MXC_DCD_ITEM(30, MMDC_P0_BASE_ADDR + 0x890, 0x00300000)
MXC_DCD_ITEM(31, MMDC_P1_BASE_ADDR + 0x8bc, 0x00055555)
MXC_DCD_ITEM(32, MMDC_P0_BASE_ADDR + 0x8b8, 0x00000800)
MXC_DCD_ITEM(33, MMDC_P0_BASE_ADDR + 0x81c, 0x33333333)
MXC_DCD_ITEM(34, MMDC_P0_BASE_ADDR + 0x820, 0x33333333)
MXC_DCD_ITEM(35, MMDC_P0_BASE_ADDR + 0x824, 0x33333333)
MXC_DCD_ITEM(36, MMDC_P0_BASE_ADDR + 0x828, 0x33333333)

/*
 * Read and write data delay, per byte.
 * For optimized DDR operation it is recommended to run mmdc_calibration
 * on your board, and replace 4 delay register assigns with resulted values
 * Note:
 * a. DQS gating is not relevant for LPDDR2. DSQ gating calibration section
 *    should be skipped, or the write/read calibration comming after that
 *    will stall
 * b. The calibration code that runs for both MMDC0 & MMDC1 should be used.
 */

MXC_DCD_ITEM(37, MMDC_P0_BASE_ADDR + 0x848, 0x40404040)
MXC_DCD_ITEM(38, MMDC_P0_BASE_ADDR + 0x850, 0x40404040)

/*dqs gating dis*/
MXC_DCD_ITEM(39, MMDC_P0_BASE_ADDR + 0x83c, 0x20000000)

/*frc_msr*/
MXC_DCD_ITEM(40, MMDC_P0_BASE_ADDR + 0x8b8, 0x00000800)

/*========================================================================*/
/* Calibration setup end*/
/*========================================================================*/

MXC_DCD_ITEM(41, MMDC_P0_BASE_ADDR + 0xc, 0x33374133)
MXC_DCD_ITEM(42, MMDC_P0_BASE_ADDR + 0x4, 0x20036)
MXC_DCD_ITEM(43, MMDC_P0_BASE_ADDR + 0x10, 0x100A82)
MXC_DCD_ITEM(44, MMDC_P0_BASE_ADDR + 0x14, 0x93)
MXC_DCD_ITEM(45, MMDC_P0_BASE_ADDR + 0x18, 0x1688)
MXC_DCD_ITEM(46, MMDC_P0_BASE_ADDR + 0x2c, 0xf9f26d2)
MXC_DCD_ITEM(47, MMDC_P0_BASE_ADDR + 0x30, 0x20e)
MXC_DCD_ITEM(48, MMDC_P0_BASE_ADDR + 0x38, 0x190778)
MXC_DCD_ITEM(49, MMDC_P0_BASE_ADDR + 0x8, 0x0)
MXC_DCD_ITEM(50, MMDC_P0_BASE_ADDR + 0x40, 0x4f)
MXC_DCD_ITEM(51, MMDC_P0_BASE_ADDR + 0x0, 0xc3110000)

/* Channel0 : Configure DDR device:*/
//CS0
MXC_DCD_ITEM(53, MMDC_P0_BASE_ADDR + 0x01c, 0x003f8030)
MXC_DCD_ITEM(54, MMDC_P0_BASE_ADDR + 0x01c, 0xff0a8030)
MXC_DCD_ITEM(55, MMDC_P0_BASE_ADDR + 0x01c, 0x82018030)
MXC_DCD_ITEM(56, MMDC_P0_BASE_ADDR + 0x01c, 0x04028030)
MXC_DCD_ITEM(57, MMDC_P0_BASE_ADDR + 0x01c, 0x02038030)
//CS1			
MXC_DCD_ITEM(58, MMDC_P0_BASE_ADDR + 0x01c, 0x003f8038)
MXC_DCD_ITEM(59, MMDC_P0_BASE_ADDR + 0x01c, 0xff0a8038)
MXC_DCD_ITEM(60, MMDC_P0_BASE_ADDR + 0x01c, 0x82018038)
MXC_DCD_ITEM(61, MMDC_P0_BASE_ADDR + 0x01c, 0x04028038)
MXC_DCD_ITEM(62, MMDC_P0_BASE_ADDR + 0x01c, 0x02038038)

/*######################################################*/
/*final DDR setup, before operation start:*/
MXC_DCD_ITEM(63, MMDC_P0_BASE_ADDR + 0x800, 0xa1310003)
MXC_DCD_ITEM(64, MMDC_P0_BASE_ADDR + 0x020, 0x00001800)
MXC_DCD_ITEM(65, MMDC_P0_BASE_ADDR + 0x818, 0x00000000)
MXC_DCD_ITEM(66, MMDC_P0_BASE_ADDR + 0x004, 0x00025576)
MXC_DCD_ITEM(67, MMDC_P0_BASE_ADDR + 0x404, 0x00011006)
MXC_DCD_ITEM(68, MMDC_P0_BASE_ADDR + 0x01c, 0x00000000)
/*###################put the ddr script here ######################*/