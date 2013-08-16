#include <common.h>
#include <command.h>
#include <part.h>
#include <fat.h>
#include <lcd.h>
#include <bmpmanager.h>

#define MAX_BUFFER_SIZE             ( 4096 )
#define MAX_CMD_LEN                 ( 256 )

#ifndef CONFIG_AUTOUPDATER_FILE_NAME
#define CONFIG_AUTOUPDATER_FILE_NAME "/autoupdate/update.txt"
#endif
#ifndef CONFIG_AUTOUPDATER_SEQUENCER
#define CONFIG_AUTOUPDATER_SEQUENCER \
	{"mmcstart;mmc sw_dev 0;mmc rescan","mmc",0}, \
	{"usb start","usb",0}, 
#endif
typedef struct {
    const char* devprepare;
    const char* devname;
    const int devidx;
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
	                    void* bmp = CONFIG_SYS_LOAD_ADDR;				
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

int inline __autoupdate_mode_detect (void) {return 0;}
int autoupdate_mode_detect(void)__attribute__((weak, alias("__autoupdate_mode_detect")));

int check_autoupdate_mode(void){	
	if (autoupdate_check_and_clean_flag()||autoupdate_mode_detect())
		do_autoupdate(NULL, 0, 0, 0);
}

