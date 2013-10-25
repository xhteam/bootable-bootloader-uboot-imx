/* 
* (C) Copyright 2012-2013  Raymond Wang, Quester Technology,Inc.
* See file CREDITS for list of people who contributed to this
* project.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston,
* MA 02111-1307 USA
*/
/*
 * This file can auto update from sparse description file (uboot script ) or standard fastboot image zip file.
 * 
*/
#include <common.h>
#include <command.h>
#include <part.h>
#include <fat.h>
#include <lcd.h>
#include <bmpmanager.h>
#include <malloc.h>


#define MAX_BUFFER_SIZE             ( 4096 )
#define MAX_CMD_LEN                 ( 256 )

#ifndef CONFIG_AUTOUPDATE_ROOT
#define CONFIG_AUTOUPDATE_ROOT	"/autoupdate/"
#endif
#ifndef CONFIG_AUTOUPDATER_FILE_NAME
#define CONFIG_AUTOUPDATER_FILE_NAME CONFIG_AUTOUPDATE_ROOT"update.txt"
#endif
#ifndef CONFIG_AUTOUPDATER_SEQUENCER
#define CONFIG_AUTOUPDATER_SEQUENCER \
	{"mmcstart;mmc sw_dev 0;mmc rescan","mmc",0}, \
	{"usb start","usb",0}, 
#endif
typedef struct {
    char* devprepare;
    char* devname;
    int devidx;
}AUTO_UPDATE_DEVICE_DESC_T;


extern int autoupdate_check_and_clean_flag(void);


static void parse_cmd_and_run( char *buf, int size )
{
    char        command[ MAX_CMD_LEN ];
    int         i, j, count;

    count = 0;

    for( i = 0; i < size; i++ )
    {
        switch( buf[ i ] )
        {
            case '\n':
                command[ count ] = '\0';
                for( j = 0; j < count; j++ )
                {
                    if( command[ j ] == '#' )
                        command[ j ] = '\0';
                }

                if( strlen( command ) )
                    run_command( command, 0 );

                count = 0;
                break;

            case '\r':
                /* Do nothing , just skip it */
                break;

            case ' ':
                if( count )
                    command[ count++ ] = buf[ i ];
                break;

            default:
                command[ count++ ] = buf[ i ];
        }
    }
}

static int lcd_draw_bmp(u8* bmp_image,int mode){
    int ret=0;
	bmp_image_t *bmp = (bmp_image_t *) bmp_image;
	int screen_w;
	int screen_h;
	int x,y;

	screen_w = panel_info.vl_col;
	screen_h = panel_info.vl_row;
	if (mode)
	{
		x = y = 0;
	}
	else
	{
        unsigned long width, height;
	    width = height = 0;
        if (((bmp->header.signature[0] == 'B') &&
              (bmp->header.signature[1] == 'M'))) {
            width = le32_to_cpu (bmp->header.width);
            height = le32_to_cpu (bmp->header.height);
        }
		
		//fix issue that height value may be negative 
		if(height<0) height=-height;
		//avoid bad bmp draw on screen
		if(width>screen_w||height>screen_h)
			return -1;

		x = max(0, (screen_w- width) / 2);
		y = max(0, (screen_h- height) / 2);
	}

	#if defined(CONFIG_LCD)
	extern int lcd_display_bitmap (ulong, int, int);

	ret = lcd_display_bitmap ((unsigned long)bmp_image, x, y);
	#elif defined(CONFIG_VIDEO)
	extern int video_display_bitmap (ulong, int, int);

	ret = video_display_bitmap ((unsigned long)bmp_image, x, y);
	#else
	# error lcd_draw_bmp() requires CONFIG_LCD or CONFIG_VIDEO
	#endif

	return ret;
}

static int do_autoupdate( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[] )
{
    AUTO_UPDATE_DEVICE_DESC_T devs[] = {
		CONFIG_AUTOUPDATER_SEQUENCER
    };
	block_dev_desc_t    *dev_desc = NULL;
	int                 size;
	char                *buffer=NULL;
	char* descfile=CONFIG_AUTOUPDATER_FILE_NAME;
	int dev_count = ARRAY_SIZE(devs);
	int dev=0;
	int count=10;

	if(argc>1)
		descfile=argv[1];

    if(!dev_count) return -1;
    do {
        if(devs[dev].devprepare)
            run_command( devs[dev].devprepare, 0 );
    	dev_desc = get_dev( devs[dev].devname, devs[dev].devidx);
    	if( dev_desc == NULL )
    		printf("can't get desc file from [%s:%d]\n",devs[dev].devname,devs[dev].devidx);
    	else
    	{
    		if( fat_register_device( dev_desc, 1 ) != 0 )
    			printf ("\n*No fat device found on %s:%d*\n", devs[dev].devname,1);
    		else
    		{
    			if(!buffer) buffer=malloc(MAX_BUFFER_SIZE);
			if(!buffer){
				printf("out of memory");
			}
    			else if (file_fat_detectfs() != 0) {
				printf("file_fat_detectfs failed\n");				
    			}else {	
    				for(;count>0;count--)
					udelay(1000);
					
	    			size = file_fat_read( descfile, buffer, MAX_BUFFER_SIZE );
					if(size<=0){
						char device_desc[256];
						//try device desc file
						sprintf(device_desc,"/autoupdate/update_%s.txt",devs[dev].devname);
						descfile = device_desc;
						size = file_fat_read( descfile, buffer, MAX_BUFFER_SIZE );
						
					}
	    			if( size && (size!=-1))
	    			{    			    
	                    #if defined(CONFIG_VIDEO)||defined(CONFIG_LCD)
	                    void* bmp = (void*)CONFIG_SYS_LOAD_ADDR;
	                    if(!bmp_manager_readbmp("bmp.autoupdater",bmp,0)){
							#ifdef CONFIG_CMD_BMP
	                        lcd_draw_bmp(bmp,0);
							#endif
	                    }
	                    #endif
	    				buffer[ size ] = '\0';
	    				parse_cmd_and_run( buffer, size );
						free(buffer);
	    				return 0;
	    			}    
    			}
    		}
    	}
    	dev++;
	}while(dev<dev_count);

	free(buffer);

	return -1;
}

U_BOOT_CMD(
	autoupdate,	2,	1,	do_autoupdate,
	"autoupdate - run autoupdater\n",
	"Read"
	CONFIG_AUTOUPDATER_FILE_NAME "from usb/sd disk then update system"
);



#include <linux/ctype.h>
#include <zipfile/zipfile.h>
#include <fastboot.h>

#define OP_DOWNLOAD   1
#define OP_COMMAND    2
#define OP_QUERY      3
#define OP_NOTICE     4
#define OP_FORMAT     5
#define OP_DOWNLOAD_SPARSE 6


#define CMD_SIZE 64

typedef struct Action Action;
struct Action
{
    unsigned op;
    Action *next;

    char cmd[CMD_SIZE];
    const char *prod;
    void *data;
    unsigned size;

    const char *msg;
    int (*func)(Action *a, int status, char *resp);

};

static int match(char *str, const char **value, unsigned count)
{
    unsigned n;

    for (n = 0; n < count; n++) {
        const char *val = value[n];
        int len = strlen(val);
        int match;

        if ((len > 1) && (val[len-1] == '*')) {
            len--;
            match = !strncmp(val, str, len);
        } else {
            match = !strcmp(val, str);
        }

        if (match) return 1;
    }

    return 0;
}

static int cb_check(Action *a, int status, char *resp, int invert)
{
    const char **value = a->data;
    unsigned count = a->size;
    unsigned n;
    int yes;

    if (status) {
        printf("FAILED (%s)\n", resp);
        return status;
    }

    if (a->prod) {
        if (strcmp(a->prod, CONFIG_FASTBOOT_PRODUCT_NAME_STR) != 0) {
            printf("IGNORE, product is %s required only for %s \n",
                    CONFIG_FASTBOOT_PRODUCT_NAME_STR);
            return 0;
        }
    }

    yes = match(resp, value, count);
    if (invert) yes = !yes;

    if (yes) {
        printf("OKAY \n");
        return 0;
    }

    printf("FAILED\n\n");
    printf("Device %s is '%s'.\n", a->cmd + 7, resp);
    printf("Update %s '%s'",
            invert ? "rejects" : "requires", value[0]);
    for (n = 1; n < count; n++) {
        printf(" or '%s'", value[n]);
    }
    printf(".\n\n");
    return -1;
}

static int cb_require(Action *a, int status, char *resp)
{
    return cb_check(a, status, resp, 0);
}

static int cb_reject(Action *a, int status, char *resp)
{
    return cb_check(a, status, resp, 1);
}

int fb_queue_require(const char *prod, const char *var,
		int invert, unsigned nvalues, const char **value)
{
    Action a;
	int result=0;
    a.prod = prod;
    a.data = value;
    a.size = nvalues;
    a.func = invert ? cb_reject : cb_require;
	if (!strcmp(var, "version")) {
		result =  a.func(&a,0,FASTBOOT_VERSION);
	} else if (!strcmp(var,"product")) {
		result = a.func(&a,0,CONFIG_FASTBOOT_PRODUCT_NAME_STR);
	} else if (!strcmp(var,"serialno")) {
		result =  a.func(&a,0,CONFIG_FASTBOOT_SERIAL_NUM);
	}
	return result;
}


static char *strip(char *s)
{
    int n;
    while(*s && isspace(*s)) s++;
    n = strlen(s);
    while(n-- > 0) {
        if(!isspace(s[n])) break;
        s[n] = 0;
    }
    return s;
}

#define MAX_OPTIONS 32
static int setup_requirement_line(char *name)
{
    char *val[MAX_OPTIONS];
    const char **out;
    char *prod = NULL;
    unsigned n, count;
    char *x;
    int invert = 0;

    if (!strncmp(name, "reject ", 7)) {
        name += 7;
        invert = 1;
    } else if (!strncmp(name, "require ", 8)) {
        name += 8;
        invert = 0;
    } else if (!strncmp(name, "require-for-product:", 20)) {
        // Get the product and point name past it
        prod = name + 20;
        name = strchr(name, ' ');
        if (!name) return -1;
        *name = 0;
        name += 1;
        invert = 0;
    }

    x = strchr(name, '=');
    if (x == 0) return 0;
    *x = 0;
    val[0] = x + 1;

    for(count = 1; count < MAX_OPTIONS; count++) {
        x = strchr(val[count - 1],'|');
        if (x == 0) break;
        *x = 0;
        val[count] = x + 1;
    }

    name = strip(name);
    for(n = 0; n < count; n++) val[n] = strip(val[n]);

    name = strip(name);
    if (name == 0) return -1;

        /* work around an unfortunate name mismatch */
    if (!strcmp(name,"board")) name = "product";

    out = malloc(sizeof(char*) * count);
    if (out == 0) return -1;

    for(n = 0; n < count; n++) {
        out[n] = strdup(strip(val[n]));
        if (out[n] == 0) return -1;
    }

    return fb_queue_require(prod, name, invert, n, out);
}

static int setup_requirements(char *data, unsigned sz)
{
    char *s;

    s = data;
    while (sz-- > 0) {
        if(*s == '\n') {
            *s++ = 0;
            if (setup_requirement_line(data)) {
                return -1;
            }
            data = s;
        } else {
            s++;
        }
    }
	return 0;
}


static void *unzip_file(zipfile_t zip, const char *name, unsigned *sz)
{
    void *data;
    zipentry_t entry;
    unsigned datasz;

    entry = lookup_zipentry(zip, name);
    if (entry == NULL) {
        printf( "archive does not contain '%s'\n", name);
        return 0;
    }


    *sz = get_zipentry_size(entry);

    datasz = *sz * 1.001;
    data =CONFIG_FASTBOOT_TRANSFER_BUF;

    if(data == 0) {
        printf( "failed to allocate %d bytes\n", *sz);
        return 0;
    }

	printf("loading file %s ...wait",name);
    if (decompress_zipentry(entry, data, datasz)) {
        printf( "failed to unzip '%s' from archive\n", name);
        return 0;
    }
	printf("\rloading file %s ...ok    \n",name);
    return data;
}

static void *load_file(const char* interface,int dev,int part,const char *fn, unsigned *_sz)
{
    char *data;
    int sz;
	char command[128];
	char* env;
	ulong addr;
    data = 0;
	env = getenv("loadaddr");
	if (env != NULL)
		addr = simple_strtoul (env, NULL, 16);
	else
		addr = CONFIG_SYS_LOAD_ADDR;

	//currently only fat fs supported
	//"<interface> <dev[:part]>  <addr> <filename> [bytes]"
	sprintf(command,"fatload %s %d:%d %lx %s",interface,dev,part,addr,fn);

	run_command(command,0);
	env = getenv("filesize");
	if (env != NULL){
		data = addr;
		sz = simple_strtoul (env, NULL, 16);
	}

    if(_sz) *_sz = sz;
    return data;

oops:
    return 0;
}


#ifdef CONFIG_FASTBOOT_STORAGE_EMMC_SATA
/*copy from fastboot.c*/
#include <mmc.h>
#include <sata.h>
enum {
    PTN_MBR_INDEX = 0,
    PTN_BOOTLOADER_INDEX,
    PTN_KERNEL_INDEX,
    PTN_URAMDISK_INDEX,
    PTN_SYSTEM_INDEX,
    PTN_RECOVERY_INDEX
};
#define MMC_SATA_BLOCK_SIZE 512

/*
 * imx family android layout
 * mbr -  0 ~ 0x3FF byte
 * bootloader - 0x400 ~ 0xFFFFF byte
 * kernel - 0x100000 ~ 5FFFFF byte
 * uramedisk - 0x600000 ~ 0x6FFFFF  supposing 1M temporarily
 * SYSTEM partition - /dev/mmcblk0p2  or /dev/sda2
 * RECOVERY parittion - dev/mmcblk0p4 or /dev/sda4
 */
/*
  *Quester comment,FSL IVT is fetched in fixed address for SD/MMC/eMMC/eSD/SDXC(0x200 offset).
  * Bootloader contains IVT but bootloader image generated add 1K bytes padding,For simlicity,we check
  * the header if start in IVT format. So Change ANDROID_BOOTLOADER_OFFSET to 0 and 
  * ANDROID_BOOTLOADER_SIZE to 0x100000 (1MB)
  *
*/
#define ANDROID_MBR_OFFSET	    0
#define ANDROID_MBR_SIZE	    0x200
#define ANDROID_BOOTLOADER_OFFSET   0/*0x400 */
#define ANDROID_BOOTLOADER_SIZE	    0x100000 /*0xFFC00*/
#define ANDROID_KERNEL_OFFSET	    0x100000
#define ANDROID_KERNEL_SIZE	    0x500000
#define ANDROID_URAMDISK_OFFSET	    0x600000
#define ANDROID_URAMDISK_SIZE	    0x100000


/**
   @mmc_dos_partition_index: the partition index in mbr.
   @mmc_partition_index: the boot partition or user partition index,
   not related to the partition table.
 */
static int __setup_ptable_mmc_partition(int ptable_index,
				      int mmc_dos_partition_index,
				      int mmc_partition_index,
				      const char *name,
				      block_dev_desc_t *dev_desc,
				      fastboot_ptentry *ptable)
{
	disk_partition_t info;
	strcpy(ptable[ptable_index].name, name);

	if (get_partition_info(dev_desc,
			       mmc_dos_partition_index, &info)) {
		printf("Bad partition index:%d for partition:%s\n",
		       mmc_dos_partition_index, name);
		return -1;
	} else {
		ptable[ptable_index].start = info.start;
		ptable[ptable_index].length = info.size;
		ptable[ptable_index].partition_id = mmc_partition_index;
	}
	return 0;
}


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

static int __init_mmc_sata_ptable(void)
{
	int i;
	int devid=-1;
#ifdef CONFIG_CMD_SATA
	int sata_device_no;
#endif
	int boot_partition = -1, user_partition = -1;
	/* mmc boot partition: -1 means no partition, 0 user part., 1 boot part.
	 * default is no partition, for emmc default user part, except emmc*/
	struct mmc *mmc;
	block_dev_desc_t *dev_desc;
	char *fastboot_env;
	fastboot_ptentry ptable[PTN_RECOVERY_INDEX + 1];

	fastboot_env = getenv("fastboot_dev");
	/* sata case in env */
	if (fastboot_env && !strcmp(fastboot_env, "sata")) {
#ifdef CONFIG_CMD_SATA
		puts("flash target is SATA\n");
		if (sata_initialize())
			return -1;
		sata_device_no = CONFIG_FASTBOOT_SATA_NO;
		if (sata_device_no >= CONFIG_SYS_SATA_MAX_DEVICE) {
			printf("Unknown SATA(%d) device for fastboot\n",
				sata_device_no);
			return -1;
		}
		devid = dev_desc = sata_get_dev(sata_device_no);
#else
		puts("SATA isn't buildin\n");
		return -1;
#endif
	} else {
		int mmc_no = 0;

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
		devid = mmc_no;
	}

	memset((char *)ptable, 0,
		    sizeof(fastboot_ptentry) * (PTN_RECOVERY_INDEX + 1));
	/* MBR */
	strcpy(ptable[PTN_MBR_INDEX].name, "mbr");
	ptable[PTN_MBR_INDEX].start = ANDROID_MBR_OFFSET / dev_desc->blksz;
	ptable[PTN_MBR_INDEX].length = ANDROID_MBR_SIZE / dev_desc->blksz;
	ptable[PTN_MBR_INDEX].partition_id = user_partition;
	/* Bootloader */
	strcpy(ptable[PTN_BOOTLOADER_INDEX].name, "bootloader");
	ptable[PTN_BOOTLOADER_INDEX].start =
				ANDROID_BOOTLOADER_OFFSET / dev_desc->blksz;
	ptable[PTN_BOOTLOADER_INDEX].length =
				 ANDROID_BOOTLOADER_SIZE / dev_desc->blksz;
	ptable[PTN_BOOTLOADER_INDEX].partition_id = boot_partition;

	__setup_ptable_mmc_partition(PTN_KERNEL_INDEX,
				   CONFIG_ANDROID_BOOT_PARTITION_MMC,
				   user_partition, "boot", dev_desc, ptable);
	__setup_ptable_mmc_partition(PTN_RECOVERY_INDEX,
				   CONFIG_ANDROID_RECOVERY_PARTITION_MMC,
				   user_partition,
				   "recovery", dev_desc, ptable);
	__setup_ptable_mmc_partition(PTN_SYSTEM_INDEX,
				   CONFIG_ANDROID_SYSTEM_PARTITION_MMC,
				   user_partition,
				   "system", dev_desc, ptable);

	for (i = 0; i <= PTN_RECOVERY_INDEX; i++)
		if(strlen(ptable[i].name))
			fastboot_flash_add_ptn(&ptable[i]);

	return devid;
}
#endif

//apply bootloader patch for device dest
//only apply for SD/MMC boot device
static void patch_on_bootloader(char* source,char* dest,char *length){
	unsigned char* source_buf=simple_strtoul(source, NULL, 16);
	unsigned int dest_start = simple_strtoul(dest, NULL, 16);
	int no_padding=0;
	//dest normally pointer to dest device partition start sector
	if(source_buf){
		//check if there is padding of source image
		if((source_buf[0]==0xD1)&&((source_buf[3]==0x40)||
			(source_buf[3]==0x41)))
			no_padding=1;
	}
	//we should change dest for padding or not
	if(dest_start){
		//should step on padding bytes
		if(!no_padding){
			sprintf(source,"0x%x",source_buf+0x400);
		}
	}else{
		if(no_padding){
			//jump over dest in 0x200
			dest_start+=0x400/MMC_SATA_BLOCK_SIZE;//
			sprintf(dest,"0x%x",dest_start);
		}
	}
	//ignore length since it's calculated freely
}

static int fb_queue_flash(int devid,const char *ptname, void *data, unsigned sz)
{
	printf("sending '%s' (%d KB)", ptname, sz / 1024);
	printf("writing '%s'", ptname);
	if (sz) {
		struct fastboot_ptentry *ptn;
		char source[32], dest[32];
		char length[32], slot_no[32];
		char part_no[32];

		/* Next is the partition name */
		ptn = fastboot_flash_find_ptn(ptname);
		if (!ptn) {
			//update possible bmp 
			if(!memcmp(ptname,"bmp.",4)){
				printf("FIXME:try to update logo\n");
			}
			printf("Partition:'%s' does not exist\n", ptn->name);
		}  else {
				char *mmc_write[5] = {"mmc", "write",
					NULL, NULL, NULL};
				char *mmc_dev[4] = {"mmc", "dev", NULL, NULL};

				printf("writing to partition '%s'\n", ptn->name);

				if(ptn->name&&!strcmp(ptn->name,"bootloader")){
					printf("patch on bootloader through fastboot\n");
					patch_on_bootloader(source,dest,length);
				}
				mmc_dev[2] = slot_no;
				mmc_dev[3] = part_no;
				mmc_write[2] = source;
				mmc_write[3] = dest;
				mmc_write[4] = length;

				sprintf(slot_no, "%d",devid);
				sprintf(source, "0x%x", (unsigned int)CONFIG_FASTBOOT_TRANSFER_BUF);
				/* partition no */
				sprintf(part_no, "%d",
					    ptn->partition_id);
				/* block offset */
				sprintf(dest, "0x%x", ptn->start);
				/* block count */

				//block size alignment
				sz = (sz +	MMC_SATA_BLOCK_SIZE - 1) /	MMC_SATA_BLOCK_SIZE;
				sprintf(length, "0x%x", sz);

				printf("Initializing '%s'\n", ptn->name);
				if (do_mmcops(NULL, 0, 4, mmc_dev))
					printf("FAIL:Init of MMC card\n");
				else
					printf("OKAY\n");

				printf("Writing '%s'\n", ptn->name);
				if (do_mmcops(NULL, 0, 5, mmc_write)) {
					printf("Writing '%s' FAILED!\n", ptn->name);
					return -1;
				} else {
					printf("Writing '%s' DONE!\n", ptn->name);
					return 0;
				}
			}

		}
}


static void do_update_signature(zipfile_t zip, char *fn)
{
    void *data;
    unsigned sz;
    //data = unzip_file(zip, fn, &sz);
    //if (data == 0) return;
    //fb_queue_download("signature", data, sz);
    //fb_queue_command("signature", "installing signature");
}

static void fb_queue_erase(const char *ptn)
{
	//FIXME???
    //Action *a;
    //a = queue_action(OP_COMMAND, "erase:%s", ptn);
   // a->msg = mkmsg("erasing '%s'", ptn);
}


#define die(...) do{printf(__VA_ARGS__);return -1;}while(0)

static int do_update(char* interface,int dev,int part,char *pkg, int erase_first){
	unsigned long zsize,sz;
	void *zdata;
	void *data;
	zipfile_t zip;
	int devid=-1;

    zdata = load_file(interface,dev,part,pkg, &zsize);
    if (zdata == 0) die("failed to load '%s': ", pkg);

    zip = init_zipfile(zdata, zsize);
    if(zip == 0) die("failed to access zipdata in '%s'");

    data = unzip_file(zip, "android-info.txt", &sz);
    if (data == 0) {
        char *tmp;
            /* fallback for older zipfiles */
        data = unzip_file(zip, "android-product.txt", &sz);
        if ((data == 0) || (sz < 1)) {
            die("update package has no android-info.txt or android-product.txt");
        }
        tmp = malloc(sz + 128);
        if (tmp == 0) die("out of memory");
        sprintf(tmp,"board=%sversion-baseband=0.66.04.19\n",(char*)data);
        data = tmp;
        sz = strlen(tmp);
    }

    if(setup_requirements(data, sz)){
		printf("requirements not matched,ignore\n");
		return -1;
	}

	//setup mmc/sata partition table
	devid = __init_mmc_sata_ptable();
	if(devid<0){
		printf("failed to init partition table\n");
		return devid;
	}

    data = unzip_file(zip, "boot.img", &sz);
    if (data == 0) die("update package missing boot.img");
    do_update_signature(zip, "boot.sig");
    if (erase_first ) {
        fb_queue_erase("boot");
    }
    fb_queue_flash(devid,"boot", data, sz);

    data = unzip_file(zip, "recovery.img", &sz);
    if (data != 0) {
        do_update_signature(zip, "recovery.sig");
        if (erase_first ) {
            fb_queue_erase("recovery");
        }
        fb_queue_flash(devid,"recovery", data, sz);
    }

    data = unzip_file(zip, "system.img", &sz);
    if (data == 0) die("update package missing system.img");
    do_update_signature(zip, "system.sig");
    if (erase_first ) {
        fb_queue_erase("system");
    }
    fb_queue_flash(devid,"system", data, sz);

	//reset partition table
	fastboot_flash_reset_ptn();

	return 0;
}

/*
 * If pkg name supplied,use this package to update
 * If pkg name not supplie,search all zip files in /autoupdate then verify each zipfile (board.prop)
*/
static int do_update_package( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[] ){
	AUTO_UPDATE_DEVICE_DESC_T devs[] = {
		CONFIG_AUTOUPDATER_SEQUENCER
    };
	char* pkg=NULL;
	int erase_first = 1;
	block_dev_desc_t    *dev_desc = NULL;
	int dev_count = ARRAY_SIZE(devs);
	int dev=0;
	int part=1;
	int update_ok=0;
	char* interface;

    if(!dev_count) return -1;

	if(argc > 1)
		pkg=argv[1];
	if(argc > 2){
		erase_first = simple_strtoul(argv[2], NULL, 10);
		erase_first = (erase_first>0)?1:0;
	}

	//detect fatfs
	for(;dev<dev_count;dev++){
		interface = NULL;
        if(devs[dev].devprepare)
            run_command( devs[dev].devprepare, 0 );
		dev_desc = get_dev( devs[dev].devname, devs[dev].devidx);
		if( dev_desc == NULL )
			printf("can't get desc file from [%s:%d]\n",devs[dev].devname,devs[dev].devidx);
		else
		{
			if( fat_register_device( dev_desc, part) != 0 ){
				printf ("\n*No fat device found on %s:%d*\n", devs[dev].devname,part);
			}else{
				int ret=-1;
				interface = devs[dev].devname;

				//everything ok
				if(pkg){
					printf("autoupdate package %s",pkg);
					ret = do_update(interface,devs[dev].devidx,part,pkg,erase_first);
				}else {
					//auto search???? in ideal path?
					pkg = CONFIG_AUTOUPDATE_ROOT CONFIG_FASTBOOT_PRODUCT_NAME_STR ".zip";
					ret = do_update(interface,devs[dev].devidx,part,pkg,erase_first);
					if(ret){
						printf("no %s found on [%s %d:%d]\n",pkg,interface,devs[dev].devidx,part);
					}
					pkg = CONFIG_AUTOUPDATE_ROOT "autoupdate" ".zip";
					ret = do_update(interface,devs[dev].devidx,part,pkg,erase_first);
					if(ret){
						printf("no %s found on [%s %d:%d]\n",pkg,interface,devs[dev].devidx,part);
					}
					pkg = CONFIG_AUTOUPDATE_ROOT "update" ".zip";
					ret = do_update(interface,devs[dev].devidx,part,pkg,erase_first);
					if(ret){
						printf("no %s found on [%s %d:%d]\n",pkg,interface,devs[dev].devidx,part);
					}
				}
				if(!ret){
					printf("update okay on pkg %s [%s %d:%d]\n",pkg,interface,devs[dev].devidx,part);
					update_ok = 1;
					break;
				}
			}
		}
    }

	//trying legacy autoupdater once again
	if(!update_ok)
		update_ok = run_command("autoupdate",0)?0:1;
	//always return success
	return update_ok?0:-1;
}


U_BOOT_CMD(
	updatepackage,	3,	1,	do_update_package,
	"updatepackage - run autoupdater\n",
	"updatepackage [package] -- run package update from usb/sd disk"
);

int inline __autoupdate_mode_detect (void) {return 0;}
int autoupdate_mode_detect(void)__attribute__((weak, alias("__autoupdate_mode_detect")));

int check_autoupdate_mode(void){	
	if (autoupdate_check_and_clean_flag()||autoupdate_mode_detect()){
		printf("autoupdate mode detected\n");
		do_update_package(NULL, 0, 0, 0);
	}
}

