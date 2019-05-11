#include <linux/kernel.h>
#include <linux/uaccess.h>

/*
 * encrypt some data to one data-stream.  
 * return 4byte data stream.
 */
asmlinkage int sys_mkstream(char time_interval, char count, char* start_option){
	int i, ret = 0;
	char start_ops[4];
	char start_idx=0, start_val=0;

	copy_from_user(start_ops, start_option, 4);

	for(i=0; i<4; i++){
		if(start_ops[i] != '0'){
			start_idx = (char)i;
			start_val = (char)(start_ops[i] - '0');
		}
	}

	ret = (time_interval << 24) | (count << 16) | (start_idx << 8) | (start_val);
	return ret;
}
