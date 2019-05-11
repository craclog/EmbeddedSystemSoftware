#include "./dev_driver.h"

// #define DEBUG

static struct struct_timer_data {
	struct timer_list timer;
	int count;
} fpga_timer;

/* *****************************************************************
 *              8bit        8bit    8bit            8bit
 * gdata : [time interval | count | start index | start value]
 * 
 * Time interval : 0.1 ~ 10 secs
 * Count : 1 ~ 100
 * Start index : 0 ~ 3
 * Start value : 1 ~ 8
 * 
 * *****************************************************************
 * Write data on FND, LED, DOT, TEXT LCD devices
 * return 1 if success
 */
ssize_t iom_fpga_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what){
    int data;
    const char *tmp = gdata;
    
    if(copy_from_user(&data, tmp, length))
        return -EFAULT;

    /* decrypt gdata (data stream) */
    time_interval = data >> 24;
    count = (data >> 16) & 0xFF;
    fnd_idx = (data >> 8) & 0xFF;
    start_val = fnd_val = data & 0xFF;
    
    /* initialize lcd buffers */
    strncpy(id_buf, "20131579        ", LCD_LINE_BUF_LEN);
    strncpy(name_buf, "KiyoungYoon     ", LCD_LINE_BUF_LEN);
    id_buf[LCD_LINE_BUF_LEN] = 0;
    name_buf[LCD_LINE_BUF_LEN] = 0;
    id_dir = RIGHT;
    name_dir = RIGHT;
    /* set timer */
    set_timer();
    /* update devices */
    update_fpga_all();

#ifdef DEBUG
    printk("iom_fpga_write::Data is...\n");
    printk("time interval : %d\n", time_interval);
    printk("count : %d\n", count);
    printk("fnd_idx : %d\n", fnd_idx);
    printk("fnd_val : %d\n", fnd_val);
#endif 

    return 1;
}
/*
 * when dev_device open ,call this function 
 * return 0 if success
 */
int iom_fpga_open(struct inode *minode, struct file *mfile){
    if(fpga_usage != 0) 
        return -EBUSY;
	fpga_usage = 1;
    return 0;
}
/*
 * when dev_device open ,call this function 
 * return 0 if success.
 */
int iom_fpga_release(struct inode *minode, struct file *mfile){
    fpga_usage = 0;
    return 0;
}
/*
 * Calls this function whenever a process tries to do an ioctl on device file.
 * return 0 if success.
 */
long iom_fpga_ioctl(struct file *inode, unsigned int ioctl_num, unsigned long param) {

    const char *tmp;
    
    switch(ioctl_num){
    case IOCTL_SET_DATA:
        tmp = (const char *)param;
        iom_fpga_write(inode, tmp, 4, 0);
        break;
    }
    return 0;
}
/*
 * Set and add timer.
 */
void set_timer(void){
    del_timer_sync(&fpga_timer.timer);
    fpga_timer.count = count - 1;
    fpga_timer.timer.expires = get_jiffies_64() + (time_interval * HZ / 10);
	fpga_timer.timer.data = (unsigned long)&fpga_timer;
	fpga_timer.timer.function	= timer_periodic;
    add_timer(&fpga_timer.timer);
}
/*
 * When timer expires, call this function.
 * This function called [count-1] times.
 */
static void timer_periodic(unsigned long timeout) {
	struct struct_timer_data *p_data = (struct struct_timer_data*)timeout;

	p_data->count--;
	if( p_data->count < 0 ) {
        clear_fpga_all();   /* init devices */
		return;
	}
    update_fpga_data(); /* update device data */
    update_fpga_all(); /* update devices */

	fpga_timer.timer.expires = get_jiffies_64() + (time_interval * HZ / 10);
	fpga_timer.timer.data = (unsigned long)&fpga_timer;
	fpga_timer.timer.function = timer_periodic;

	add_timer(&fpga_timer.timer);
}
void update_fpga_data(void){
    
    int i;
    /* Shift Upper LCD text (id) */
    if(id_dir == RIGHT){
        for(i=LCD_LINE_BUF_LEN - 1; i>0; i--){
            id_buf[i] = id_buf[i - 1];
        }
        id_buf[0] = ' ';
        if(id_buf[LCD_LINE_BUF_LEN - 1] != ' ') 
            id_dir = LEFT;
    } else{ /* LEFT */
        for(i=1; i < LCD_LINE_BUF_LEN; i++){
            id_buf[i - 1] = id_buf[i];
        }
        id_buf[LCD_LINE_BUF_LEN - 1] = ' ';
        if(id_buf[0] != ' ')
            id_dir = RIGHT;
    }
    /* Shift Lower LCD text (name) */
    if(name_dir == RIGHT){
        for(i=LCD_LINE_BUF_LEN - 1; i>0; i--){
            name_buf[i] = name_buf[i - 1];
        }
        name_buf[0] = ' ';
        if(name_buf[LCD_LINE_BUF_LEN - 1] != ' ') 
            name_dir = LEFT;
    } else{ /* LEFT */
        for(i=1; i < LCD_LINE_BUF_LEN; i++){
            name_buf[i - 1] = name_buf[i];
        }
        name_buf[LCD_LINE_BUF_LEN - 1] = ' ';
        if(name_buf[0] != ' ')
            name_dir = RIGHT;
    }
    /* Update fnd_value and index */
    fnd_val = fnd_val % 8 + 1;
    if(fnd_val == start_val)
        fnd_idx = (fnd_idx + 1) % 4;
    
}
/********************
 * Update devices
 ********************
 * Update lcd device.
 */
void update_fpga_lcd(void){
    
    strncpy(lcd_buf, id_buf, LCD_LINE_BUF_LEN);
    lcd_buf[LCD_LINE_BUF_LEN] = 0;
    strncat(lcd_buf, name_buf, LCD_LINE_BUF_LEN);
    lcd_buf[LCD_MAX_LEN] = 0;

    iom_fpga_text_lcd_write(lcd_buf, LCD_MAX_LEN);
}
/*
 * increase counter and update fnd device.
 */
void update_fpga_fnd(void){
    unsigned char data[4];
    int i;  
    for(i=0; i<4; i++)
        data[i] = 0;  
    data[(int)fnd_idx] = fnd_val;  
    iom_fpga_fnd_write(data, 4);
}
/*
 * Update led device.
 */
void update_fpga_led(void){
    unsigned char data;
    data = 1 << (8 - fnd_val);
    iom_led_write(data);
}
/*
 * Update dot device.
 */
void update_fpga_dot(void){
    iom_fpga_dot_write(fpga_number[(int)fnd_val], DOT_LEN);
}
/*
 * Update all devices.
 * status : FIRST_UPDATE or NTH_UPDATE
 */
void update_fpga_all(void){
    update_fpga_lcd();
    update_fpga_fnd();
    update_fpga_led();
    update_fpga_dot();
}
/**********************
 * initialize devices 
 **********************
 * Clear lcd device.
 */
void clear_fpga_lcd(void){
    memset(lcd_buf, 0, LCD_MAX_LEN);
    iom_fpga_text_lcd_write(lcd_buf, LCD_MAX_LEN);
}
/*
 * Clear fnd device.
 */
void clear_fpga_fnd(void){
    unsigned char data[4];
    int i;
    for(i=0; i<4; i++)
        data[i] = 0;
    iom_fpga_fnd_write(data, 4);
}
/*
 * Clear led device.
 */
void clear_fpga_led(void){
    iom_led_write(0);
}
/*
 * Clear dot device.
 */
void clear_fpga_dot(void){
    iom_fpga_dot_write(fpga_set_blank, DOT_LEN);
}
/*
 * Clear all devices.
 */
void clear_fpga_all(void){
    clear_fpga_lcd();
    clear_fpga_fnd();
    clear_fpga_led();
    clear_fpga_dot();
}

/*
 * when write led device, call this function 
 * return non-negative value if success
 */
ssize_t iom_led_write(unsigned char value) {
    unsigned short _s_value;
    _s_value = (unsigned short)value;
    outw(_s_value, (unsigned int)iom_fpga_led_addr);
    return 1;
}
/*
 * when write fnd device, call this function 
 * return non-negative value if success
 */
ssize_t iom_fpga_fnd_write(unsigned char value[4], size_t length) {
    unsigned short int value_short = 0;
    value_short = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
    outw(value_short,(unsigned int)iom_fpga_fnd_addr);	    
	return length;
}
/*
 * when write dot device, call this function 
 * return non-negative value if success
 */
ssize_t iom_fpga_dot_write(unsigned char value[10], size_t length) {
	int i;
	unsigned short int _s_value;
	for(i=0;i<length;i++) {
        _s_value = value[i] & 0x7F;
		outw(_s_value,(unsigned int)iom_fpga_dot_addr+i*2);
    }
	return length;
}
/*
 * when write text lcd device, call this function 
 * return non-negative value if success
 */
ssize_t iom_fpga_text_lcd_write(unsigned char value[33], size_t length) {
    int i;
    unsigned short int _s_value = 0;
    for (i = 0; i < length; i++) {
        _s_value = ((value[i] & 0xFF) << 8) | (value[i + 1] & 0xFF);
        outw(_s_value, (unsigned int)iom_fpga_text_lcd_addr + i);
        i++;
    }
    return length;
}
/*
 * When fpga dev open, call this function.
 * Use io map to control devices.
 * return 0 if success.
 */
int __init iom_fpga_init(void) {
	int result;

	result = register_chrdev(IOM_FPGA_MAJOR, IOM_FPGA_NAME, &iom_fpga_fops);
	if(result < 0) {
		printk(KERN_WARNING"Can't get any major\n");
		return result;
	}
	iom_fpga_led_addr = ioremap(IOM_FPGA_LED_ADDRESS, 0x1);
	iom_fpga_fnd_addr = ioremap(IOM_FPGA_FND_ADDRESS, 0x4);
	iom_fpga_dot_addr = ioremap(IOM_FPGA_DOT_ADDRESS, 0x10);
	iom_fpga_text_lcd_addr = ioremap(IOM_FPGA_TEXT_LCD_ADDRESS, 0x32);
	init_timer(&fpga_timer.timer);    
	printk("init module, %s major number : %d\n", IOM_FPGA_NAME, IOM_FPGA_MAJOR);
	
	return 0;
}
/*
 * When fpga dev close, call this function.
 * unmap devices
 */
void __exit iom_fpga_exit(void) 
{
	printk("exit module\n");
   	iounmap(iom_fpga_led_addr);
	iounmap(iom_fpga_fnd_addr);
	iounmap(iom_fpga_dot_addr);
	iounmap(iom_fpga_text_lcd_addr);
	del_timer_sync(&fpga_timer.timer);

	unregister_chrdev(IOM_FPGA_MAJOR, IOM_FPGA_NAME);
}

module_init(iom_fpga_init);
module_exit(iom_fpga_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("KiyoungYoon");
