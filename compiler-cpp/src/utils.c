#include "utils.h"
#include <stdlib.h>

void* alloc_list[10*1024]={0};
unsigned long alloc_count= 0;

void* utils_alloc(size_t size){
	void* ptr = malloc(size);
	alloc_list[alloc_count++] = ptr;
	return ptr;
}


void utils_free(void* ptr){
	for(size_t i=0; i <  alloc_count;++i){
		if(ptr == alloc_list[i]){
			free(ptr);
			alloc_list[i]= NULL;
		}
	}
}


void utils_free_all(){
	for(size_t i=0; i  <  alloc_count;++i){
			free(alloc_list[i]);
			alloc_list[i]= NULL;
	}
}
