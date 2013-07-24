#ifndef _BMP_MANAGER_H
#define _BMP_MANAGER_H


typedef struct bmp_obj{
	char name[32];
	unsigned long start;
	unsigned long size;
}bmp_t;

//bmp manager utilize flash driver to fetch bmp data,part is flash partition name
int bmp_manager_init(const char* part);
int bmp_manager_getbmp(const char* name,bmp_t* bmp);

//read /write bmp data
long bmp_manager_readbmp(const char* name,void* data,unsigned long size);
int bmp_manager_writebmp(const char* name,void* data,unsigned long size);


#endif

