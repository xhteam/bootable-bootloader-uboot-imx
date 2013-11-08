#include <common.h>
#include <command.h>
#include <bmpmanager.h>
//#include <boot/flash.h>

#include <asm/byteorder.h>
#include <nand.h>
#include <fastboot.h>
#include <environment.h>

/*
 * For eMMC based storage,it's sector based
 *
*/
#ifndef BMPMANAGER_SECTOR_START
#define BMPMANAGER_SECTOR_START			8192
#endif

#ifndef BMPMANAGER_SECTOR_END
#define BMPMANAGER_SECTOR_END			16384
#endif

#define BMPMANAGER_SECTOR_CNT			(BMPMANAGER_SECTOR_END-BMPMANAGER_SECTOR_START)

#define BMPMANAGER_SECTOR_SIZE			(512)

#ifndef BMPMANAGER_LOAD_ADDR
#define BMPMANAGER_LOAD_ADDR            0x20000000
#endif

#ifndef BMPMANAGER_LOAD_ADDR
#error "please define BMPMANAGER_LOAD_ADDR for bmp manager"
#endif
#define alignment_down(a, size) ((a/size)*size)
#define alignment_up(a, size) (((a+size-1)/size)*size) 


#define DEFAULT_BMPMNGR_PART "splash"

#define BMP_MNGR_MAGIC 0x626d6772 /*BMGR*/
#define BMP_MNGR_MAX_ELEMENTS 10
//store size is packed to BMPMANAGER_SECTOR_SIZE bytes
typedef struct bmp_store{
	unsigned long magic;
	unsigned long count;
	bmp_t bmps[BMP_MNGR_MAX_ELEMENTS];//max 10 pictures is enough for us	
}__attribute__((aligned (BMPMANAGER_SECTOR_SIZE))) bmp_store_t;

typedef struct bmpmanager_obj{
	bmp_store_t store;
	unsigned int init;
	//ptentry*   part; //we not use part
}bmpmanager_t ;

static bmpmanager_t mngr;


static void bmp_manager_dump(void){
	bmp_store_t* store = &mngr.store;
    int i;
    //dump all 
	printf("==============================\n");
	printf("found %u bmps\n",store->count);
	for(i=0;i<store->count;i++){
		printf("%s,start:0x%lx,size:0x%lx\n",store->bmps[i].name,
			store->bmps[i].start,
			store->bmps[i].size);
	}
	printf("==============================\n");

}

static int bmp_manager_reset(){
	char cmdbuffer[128];
    bmp_store_t* store = &mngr.store;
	unsigned long startblks,readblks;
	startblks=BMPMANAGER_SECTOR_END-alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE)/BMPMANAGER_SECTOR_SIZE;
	readblks=alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE)/BMPMANAGER_SECTOR_SIZE;

	#if 0
	ptentry* bmppart = mngr.part;
	if(!bmppart)	{
	    bmppart = flash_find_ptn(DEFAULT_BMPMNGR_PART);
	    mngr.part = bmppart;
    }
	if(!bmppart) {
    	return -1;
	}
	#endif

    store->magic = BMP_MNGR_MAGIC;
    store->count = 0;
		

    //writeback 
    	//flash_write(bmppart,bmppart->length-alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE),
        	//store,alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE));
    sprintf(cmdbuffer,"mmc write 0x%x 0x%x 0x%x",(void*)store,startblks,readblks);
    printf("%s\n",cmdbuffer);
    run_command(cmdbuffer,0);

    return 0;
}
//bmp manager utilize flash driver to fetch bmp data,part is flash partition name
int bmp_manager_init(const char* part){
	bmp_store_t* store = &mngr.store;
	int i;
	char cmdbuffer[128];
	unsigned long  startblks,readblks;
	startblks=BMPMANAGER_SECTOR_END-alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE)/BMPMANAGER_SECTOR_SIZE;
	readblks=alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE)/BMPMANAGER_SECTOR_SIZE;
	#if 0
	ptentry* bmppart = flash_find_ptn(part?part:DEFAULT_BMPMNGR_PART);
	if(!bmppart) {
    	return -1;
	}
	

	mngr.part = bmppart;
	//read from last location
	flash_read(bmppart,
		bmppart->length-alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE),
		(void*)store,alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE));
	#endif //direct use mmc commond.cannot use flash_find_ptn

	sprintf(cmdbuffer,"mmc read 0x%x 0x%x 0x%x",(void*)store,startblks,readblks);
    printf("%s\n",cmdbuffer);
    run_command(cmdbuffer,0);
	
		
	if(store->magic!=BMP_MNGR_MAGIC){ 
		int ret;
		int bmpsize=0;
		char head[BMPMANAGER_SECTOR_SIZE];
		//never inited
		store->magic = BMP_MNGR_MAGIC;
		store->count = 0;

		//check start already store one splash bmp
		
			//ret = flash_read(bmppart,0,head,BMPMANAGER_SECTOR_SIZE);  //use mmc commond
		sprintf(cmdbuffer,"mmc read 0x%x 0x%x 0x%x",head,BMPMANAGER_SECTOR_START,1);
    	printf("%s\n",cmdbuffer);
    	run_command(cmdbuffer,0);
		
		if(!ret&&(head[0]==0x42)&&(head[1]==0x4d))
		{
			bmpsize=(head[5]<<24)|(head[4]<<16)|(head[3]<<8)|head[2];
			store->count++;
			store->bmps[0].start = 0;
			store->bmps[0].size  = alignment_up(bmpsize,BMPMANAGER_SECTOR_SIZE);
			memset(store->bmps[0].name,0,32);
			strcpy(store->bmps[0].name,"splash");

		}
		//writeback 
		#if 0 // use mmc commond--allenyao
		flash_write(bmppart,bmppart->length-alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE),
			store,alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE));
		#endif
		sprintf(cmdbuffer,"mmc write 0x%x 0x%x 0x%x",(void*)store,startblks,readblks);
    	printf("%s\n",cmdbuffer);
    	run_command(cmdbuffer,0);
		
	}
	if(store->count>BMP_MNGR_MAX_ELEMENTS) store->count=10;
	//safe bmpname handle
    for(i=0;i<store->count;i++){
        store->bmps[i].name[32-1] = '\0';
    }
	
	mngr.init++;

	return 0;
}
int bmp_manager_getbmp(const char* name,bmp_t* bmp){
	bmp_store_t* store = &mngr.store;
	int i;
	char* altername=NULL;	
	if(!mngr.init)
		bmp_manager_init(0);
	if(!strcmp(name,"splash"))
		altername = "bmp.splash";
	else if(!strcmp(name,"bmp.splash"))
		altername = "splash";
	for(i=0;i<store->count&&i<BMP_MNGR_MAX_ELEMENTS;i++){
		if(!strcmp(store->bmps[i].name,name)){
			if(bmp)	memcpy(bmp,&store->bmps[i],sizeof(bmp_t));
			return 0;
		}
	}
	//reach here means not found 
	if(altername){
		for(i=0;i<store->count&&i<BMP_MNGR_MAX_ELEMENTS;i++){
			if(!strcmp(store->bmps[i].name,altername)){
				if(bmp) memcpy(bmp,&store->bmps[i],sizeof(bmp_t));
				return 0;
			}
		}
	}
	return -1;
	
}

 long bmp_manager_readbmp(const char* name,void* data,unsigned long size){
	//ptentry* entry;
	char cmdbuffer[128];
	bmp_t bmp;	
	unsigned long  startblks,blks;
	int ret=0;
	if(!mngr.init)
		bmp_manager_init(0);
	//entry = mngr.part;
	if(!name) return -1;
	ret = bmp_manager_getbmp(name,&bmp);
	if(ret<0) return ret;
		//return flash_read(entry,bmp.start,data,(size>bmp.size)?bmp.size:size);  // use mmc commond--allenyao
	//convert bmp.start bmp.size to mmc blks,assume block size is 512B
	startblks = BMPMANAGER_SECTOR_START+(bmp.start/BMPMANAGER_SECTOR_SIZE);
	blks = ((size>bmp.size)?bmp.size:size)/BMPMANAGER_SECTOR_SIZE;
	sprintf(cmdbuffer,"mmc read 0x%x 0x%x 0x%x",data,startblks,blks);
    printf("%s\n",cmdbuffer);
    run_command(cmdbuffer,0);
	return ret;
}

int bmp_manager_writebmp(const char* name,void* data,unsigned long size){
	char cmdbuffer[128];
	bmp_store_t* store = &mngr.store;
	
	unsigned long  start,cnt;
	//ptentry* entry;
	bmp_t bmp;
	int i;
	if(!mngr.init)
		bmp_manager_init(0);	
	//entry = mngr.part;		
	//if(!entry||!name) return -1;;
	if(!name) return -1;
	if(!bmp_manager_getbmp(name,&bmp)){
		//update ,some complicated but change to be simple
		//we used more higher memory space to store latter bmp data
		//0x1500000 normally as initrd memory
		int location=0;
		unsigned long used,space;
		used=0;
		for(i=0;i<store->count;i++){
			if(!strcmp(store->bmps[i].name,name)) break;
			used+=store->bmps[i].size;
		}
		location = i;
		if(store->count>(location+1)){
			/*
			 * Handling following case
			 ------------------------------------------------------------
			 |bmp1  |bmp2 |foundedbmp|bmp4|                                       |header|
			 ------------------------------------------------------------
			 after updated
			 ------------------------------------------------------------
			 |bmp1  |bmp2 |bmp4|updatedbmp|                                       |header|
			 ------------------------------------------------------------
			*/
			//read latter bmp to temp memory
			unsigned long movestart = store->bmps[location+1].start;
			unsigned long movesize=0;
			unsigned long mystart = bmp.start;
			unsigned long mysize = bmp.size;
			for(i=location+1;i<store->count;i++){
				movesize+=store->bmps[i].size;
			}
			//test space is enough
				//space = entry->length-used-movesize-alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE);//--allenyao
			space=BMPMANAGER_SECTOR_SIZE*BMPMANAGER_SECTOR_CNT-used-alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE);
			size = alignment_up(size,BMPMANAGER_SECTOR_SIZE);
			if(space<size) return -1;
			
			//start to move
			//FIXME: hard code for higher memory 
				//flash_read(entry,movestart,(void*)BMPMANAGER_LOAD_ADDR,movesize);//use mmc command --allenyao
			start = BMPMANAGER_SECTOR_START+(movestart/BMPMANAGER_SECTOR_SIZE);
			cnt = movesize/BMPMANAGER_SECTOR_SIZE;
			sprintf(cmdbuffer,"mmc read 0x%x 0x%x 0x%x",(void*)BMPMANAGER_LOAD_ADDR,start,cnt);
    		printf("%s\n",cmdbuffer);
    		run_command(cmdbuffer,0);
			

			//update store
			store->count--;
			memmove(&store->bmps[location],&store->bmps[location+1],(store->count-location)*sizeof(bmp_t));
			for(i=location;i<store->count;i++){
				store->bmps[i].start-=mysize;
			}

			//write moved data back
				//flash_write(entry,mystart,(void*)BMPMANAGER_LOAD_ADDR,movesize);  //use mmc command --allenyao
				//flash_write(entry,mystart+movesize,data,size);
			
			start = BMPMANAGER_SECTOR_START+(mystart/BMPMANAGER_SECTOR_SIZE);
			cnt = movesize/BMPMANAGER_SECTOR_SIZE;
			sprintf(cmdbuffer,"mmc write 0x%x 0x%x 0x%x",(void*)BMPMANAGER_LOAD_ADDR,start,cnt);
    		printf("%s\n",cmdbuffer);
    		run_command(cmdbuffer,0);


			//update new bmp 
			start = BMPMANAGER_SECTOR_START+((movesize+mystart)/BMPMANAGER_SECTOR_SIZE);
			cnt = size/BMPMANAGER_SECTOR_SIZE;
			sprintf(cmdbuffer,"mmc write 0x%x 0x%x 0x%x",data,start,cnt);
    		printf("%s\n",cmdbuffer);
    		run_command(cmdbuffer,0);
			

			//update header now
			memset(store->bmps[store->count].name,0,32);		
			strncpy(store->bmps[store->count].name,name,31);
			store->bmps[store->count].start = movestart+movesize-mysize;
			store->bmps[store->count].size = size;
			store->count++;
			//update store
			  //not use flash_write--allenyao
			//flash_write(entry,entry->length-alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE),
				//store,alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE));
			
			start = BMPMANAGER_SECTOR_END-(alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE)/BMPMANAGER_SECTOR_SIZE);
			cnt = (alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE)/BMPMANAGER_SECTOR_SIZE);
			sprintf(cmdbuffer,"mmc write 0x%x 0x%x 0x%x",store,start,cnt);
    		printf("%s\n",cmdbuffer);
    		run_command(cmdbuffer,0);

		
			
		}else{//the last one
			/*
			 * Handling following case
			 ------------------------------------------------------------
			 |bmp1  |bmp2 |foundedbmp|		                                    |header|
			 ------------------------------------------------------------
			 after updated
			 ------------------------------------------------------------
			 |bmp1  |bmp2 |updatedbmp|		                                     |header|
			 ------------------------------------------------------------
			*/		
			printf("last one?\n");
			space=BMPMANAGER_SECTOR_SIZE*BMPMANAGER_SECTOR_CNT-used-alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE);
			size = alignment_up(size,BMPMANAGER_SECTOR_SIZE);
			if(space<size) return -1;
			//write bmp data
				//flash_write(entry,used,data,size);//not use this ,use mmc command
			
			start = BMPMANAGER_SECTOR_START+(used/BMPMANAGER_SECTOR_SIZE);
			cnt = size/BMPMANAGER_SECTOR_SIZE;
			sprintf(cmdbuffer,"mmc write 0x%x 0x%x 0x%x",data,start,cnt);
    		printf("%s\n",cmdbuffer);
    		run_command(cmdbuffer,0);
			//update new size
			store->bmps[store->count-1].size = size;
			
			//writeback 
				//flash_write(entry,entry->length-alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE),
					//store,alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE));
			
			start = BMPMANAGER_SECTOR_END-(alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE)/BMPMANAGER_SECTOR_SIZE);
			cnt = (alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE)/BMPMANAGER_SECTOR_SIZE);
			sprintf(cmdbuffer,"mmc write 0x%x 0x%x 0x%x",store,start,cnt);
    		printf("%s\n",cmdbuffer);
    		run_command(cmdbuffer,0);
			
		}
		
		
	}else {
		unsigned long used,space;
		int i;
		if(store->count>=BMP_MNGR_MAX_ELEMENTS) return -1;
		//calcute left space
		for(i=0,used=0;i<store->count;i++){
			used+=store->bmps[i].size;
		}
		space=BMPMANAGER_SECTOR_SIZE*BMPMANAGER_SECTOR_CNT-used-alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE);
		size = alignment_up(size,BMPMANAGER_SECTOR_SIZE);
		if(space<size) return -1;//no enough space

		start = BMPMANAGER_SECTOR_START+(used/BMPMANAGER_SECTOR_SIZE);
		cnt = size/BMPMANAGER_SECTOR_SIZE;

		//write bmp data
			//flash_write(entry,used,data,size);
		sprintf(cmdbuffer,"mmc write 0x%x 0x%x 0x%x",data,start,cnt);
    	printf("%s\n",cmdbuffer);
    	run_command(cmdbuffer,0);
		//update manager
		memset(store->bmps[store->count].name,0,32);		
		strncpy(store->bmps[store->count].name,name,31);
		store->bmps[store->count].start = used;
		store->bmps[store->count].size = size;
		store->count++;

		//writeback 
			//flash_write(entry,entry->length-alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE),
				//store,alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE));
		
		start = BMPMANAGER_SECTOR_END-(alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE)/BMPMANAGER_SECTOR_SIZE);
		cnt = (alignment_up(sizeof(bmp_store_t),BMPMANAGER_SECTOR_SIZE)/BMPMANAGER_SECTOR_SIZE);
		sprintf(cmdbuffer,"mmc write 0x%x 0x%x 0x%x",store,start,cnt);
    	printf("%s\n",cmdbuffer);
    	run_command(cmdbuffer,0);
		
	}

	return 0;
}


static int do_bmpmanager(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    if(argc<2){
		bmp_manager_init(0);
        bmp_manager_dump();
    }else {
        //allow short name
    	if (!strcmp(argv[1],"reset")) {
    		bmp_manager_reset();
    	}else if(!strcmp(argv[1],"read")&&argc>=4){
    		char* bmpname = argv[2];
    		unsigned long addr,size;
			addr = simple_strtoul(argv[3], NULL, 0);
			size = 0xA00000;
			if(argc>4)
				size = simple_strtoul(argv[4], NULL, 0);
			printf("reading %s to %#x ... %d\n",bmpname,addr,bmp_manager_readbmp(bmpname,addr,size));			
    	}else if(!strcmp(argv[1],"write")&&argc>=5){
    		char* bmpname = argv[2];
    		unsigned long addr,size;
			addr = simple_strtoul(argv[3], NULL, 0);
			size = simple_strtoul(argv[4], NULL, 0);
			printf("writing %s from %#x ... %d\n",bmpname,addr,bmp_manager_writebmp(bmpname,addr,size));			
    	}else {
    	    printf ("Usage:\n%s\n", cmdtp->usage);
    	    return 1;
    	}
	}
	return 0;
}

U_BOOT_CMD(
	bmpmngr,	5,	1,	do_bmpmanager,
	"bmpmngr [info|reset|write|read] <bmpname> <addr> <size>     - manipulate bmp managed\n",
	""
);

