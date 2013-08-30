/*
 * Freescale BCB support for android
*/
#include <common.h>
#include <config.h>
#include <malloc.h>
#include <asm/io.h>
#include <usbdevice.h>
#include <mmc.h>
#include <sata.h>
#include <bootloader.h>

#ifndef CONFIG_ANDROID_MISC_PARTITION_MMC
#define CONFIG_ANDROID_MISC_PARTITION_MMC 8
#endif

/*
   Get mmc control number from passed string, eg, "mmc1" mean device 1. Only
   support "mmc0" to "mmc9" currently. It will be treated as device 0 for
   other string.
*/
static int get_mmc_no(char *env_str)
{
	int digit = 0;
	unsigned char a;

	if (env_str && (strlen(env_str) >= 4) &&
	    !strncmp(env_str, "mmc", 3)) {
		a = env_str[3];
		if (a >= '0' && a <= '9')
			digit = a - '0';
	}

	return digit;
}

//currently only mmc device support
static int find_bcb_dev(block_dev_desc_t** bcb){
	int boot_partition = -1, user_partition = -1;
	block_dev_desc_t *dev_desc;	
	char *fastboot_env = getenv("fastboot_dev");
	int mmc_no = 0;		
	/* mmc boot partition: -1 means no partition, 0 user part., 1 boot part.
	 * default is no partition, for emmc default user part, except emmc*/
	struct mmc *mmc;

	mmc_no = get_mmc_no(fastboot_env);

	printf("flash target is MMC:%d\n", mmc_no);
	mmc = find_mmc_device(mmc_no);
	if (mmc && mmc_init(mmc))
		printf("MMC card init failed!\n");

	dev_desc = get_dev("mmc", mmc_no);
	if (NULL == dev_desc) {
		printf("** Block device MMC %d not supported\n",
			mmc_no);
		return -1;
	}

	/* multiple boot paritions for eMMC 4.3 later */
	if (mmc->part_config != MMCPART_NOAVAILABLE) {
		boot_partition = 1;
		user_partition = 0;
	}


	*bcb = dev_desc;


	return 0;

	
}
#define BCB_BUF_SIZE 2048
int get_bootloader_message(struct bootloader_message *out){
	disk_partition_t info;
	block_dev_desc_t* bcb_dev;
	char command[128];
	char* bcbbuf=malloc(BCB_BUF_SIZE);

	if(!bcbbuf)
		return -1;

	if(find_bcb_dev(&bcb_dev)){
		free(bcbbuf);
		return -1;
	}
	
	if (get_partition_info(bcb_dev,
			       CONFIG_ANDROID_MISC_PARTITION_MMC, &info)) {
		printf("Bad partition index:%d for partition:%s\n",
		       CONFIG_ANDROID_MISC_PARTITION_MMC, "misc");
		free(bcbbuf);
		return -1;
	} 
	sprintf(command,"mmc dev 0x%x 0x%x",bcb_dev->dev,0);
	run_command(command,0);
	
	sprintf(command,"mmc read 0x%x 0x%x 0x%x",bcbbuf,info.start,BCB_BUF_SIZE);

	run_command(command,0);
	memcpy(out,bcbbuf,sizeof(struct bootloader_message));
	free(bcbbuf);

	return 0;
	
}
int set_bootloader_message(const struct bootloader_message *in){
	disk_partition_t info;
	block_dev_desc_t* bcb_dev;
	char command[128];
	char* bcbbuf=malloc(BCB_BUF_SIZE);

	if(!bcbbuf)
		return -1;

	if(find_bcb_dev(&bcb_dev)){
		free(bcbbuf);
		return -1;
	}
	
	if (get_partition_info(bcb_dev,
			       CONFIG_ANDROID_MISC_PARTITION_MMC, &info)) {
		printf("Bad partition index:%d for partition:%s\n",
		       CONFIG_ANDROID_MISC_PARTITION_MMC, "misc");
		free(bcbbuf);
		return -1;
	} 
	sprintf(command,"mmc dev 0x%x 0x%x",bcb_dev->dev,0);
	run_command(command,0);

	memset(bcbbuf,0,BCB_BUF_SIZE);
	memcpy(bcbbuf,in,sizeof(struct bootloader_message));
	
	sprintf(command,"mmc write 0x%x 0x%x 0x%x",bcbbuf,info.start,BCB_BUF_SIZE);

	run_command(command,0);
	free(bcbbuf);

	return 0;
	
}

