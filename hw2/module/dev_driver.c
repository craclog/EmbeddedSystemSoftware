#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/version.h>
// #include <linux/timer.h>

#include "./fpga_dot_font.h"

#define IOM_FPGA_LED_ADDRESS 0x08000016 // pysical address
#define IOM_FPGA_FND_ADDRESS 0x08000004 // pysical address
#define IOM_FPGA_DOT_ADDRESS 0x08000210 // pysical address
#define IOM_FPGA_TEXT_LCD_ADDRESS 0x08000090   // pysical address - 32 Byte (16 * 2)

#define IOM_FPGA_MAJOR 242
#define IOM_FPGA_NAME "dev_driver"

#define FIRST_UPDATE 1
#define NTH_UPDATE 0
#define SUCCESS 1
#define LCD_LINE_BUF_LEN 16
#define LCD_MAX_LEN 32
#define DOT_LEN 10
#define DEBUG

#define IOCTL_SET_DATA _IOW(IOM_FPGA_MAJOR, 0, char *)

/* Global variable */
static unsigned char *iom_fpga_led_addr;
static unsigned char *iom_fpga_fnd_addr;
static unsigned char *iom_fpga_dot_addr;
static unsigned char *iom_fpga_text_lcd_addr;
static int fpga_usage = 0;
static char time_interval, count, fnd_idx, fnd_val, start_val;


static char id_buf[LCD_LINE_BUF_LEN + 1];
static char name_buf[LCD_LINE_BUF_LEN + 1];
static char lcd_buf[LCD_MAX_LEN + 1];

/* Define functions */
ssize_t iom_led_write(unsigned char value);
ssize_t iom_fpga_fnd_write(unsigned char value[4], size_t length);
ssize_t iom_fpga_dot_write(unsigned char value[10], size_t length);
ssize_t iom_fpga_text_lcd_write(unsigned char value[33], size_t length);
ssize_t iom_fpga_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
int iom_fpga_open(struct inode *minode, struct file *mfile);
int iom_fpga_release(struct inode *minode, struct file *mfile);
long iom_fpga_ioctl(struct file *inode, unsigned int ioctl_num, unsigned long param);

void set_timer(void);
static void timer_periodic(unsigned long timeout);
void update_fpga_lcd(int status);
void update_fpga_fnd(int status);
void update_fpga_led(void);
void update_fpga_dot(void);

void update_fpga_all(int status);
void clear_fpga_all(void);

// define file_operations structure
struct file_operations iom_fpga_fops =
    {
        .owner = THIS_MODULE,
	    .open = iom_fpga_open,
	    .write = iom_fpga_write,
	    .unlocked_ioctl = iom_fpga_ioctl,
        .release = iom_fpga_release,
    };

static struct struct_timer_data {
	struct timer_list timer;
	int count;
} fpga_timer;
/*
 gdata : (time interval | count | start index | start value)
*/
ssize_t iom_fpga_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what){
    int data;
    const char *tmp = gdata;
    

    if(copy_from_user(&data, tmp, length))
        return -EFAULT;

    time_interval = data >> 24;
    count = (data >> 16) & 0xFF;
    fnd_idx = (data >> 8) & 0xFF;
    start_val = fnd_val = data & 0xFF;
    
    strncpy(id_buf, "20131579        ", LCD_LINE_BUF_LEN);
    strncpy(name_buf, "KiyoungYoon     ", LCD_LINE_BUF_LEN);
    id_buf[LCD_LINE_BUF_LEN] = 0;
    name_buf[LCD_LINE_BUF_LEN] = 0;

    set_timer();
    
    update_fpga_all(FIRST_UPDATE);


#ifdef DEBUG
    printk("iom_fpga_write::Data is...\n");
    printk("time interval : %d\n", time_interval);
    printk("count : %d\n", count);
    printk("fnd_idx : %d\n", fnd_idx);
    printk("fnd_val : %d\n", fnd_val);
#endif 

    return 1; //Check
}
// when dev_device open ,call this function
int iom_fpga_open(struct inode *minode, struct file *mfile){
    if(fpga_usage != 0) 
        return -EBUSY;
	fpga_usage = 1;
    return 0;
}
// when dev_device close ,call this function
int iom_fpga_release(struct inode *minode, struct file *mfile){
    fpga_usage = 0;
    return 0;
}
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
void set_timer(void){
    del_timer_sync(&fpga_timer.timer);
    fpga_timer.count = count - 1;
    fpga_timer.timer.expires = get_jiffies_64() + (time_interval * HZ / 10);
	fpga_timer.timer.data = (unsigned long)&fpga_timer;
	fpga_timer.timer.function	= timer_periodic;
    add_timer(&fpga_timer.timer);
}
static void timer_periodic(unsigned long timeout) {
	struct struct_timer_data *p_data = (struct struct_timer_data*)timeout;

	p_data->count--;
    printk("Timer count : %d\n", p_data->count);
	if( p_data->count < 0 ) {
        /* init devices */
        clear_fpga_all();
		return;
	}
    update_fpga_all(NTH_UPDATE);

	fpga_timer.timer.expires = get_jiffies_64() + (time_interval * HZ / 10);
	fpga_timer.timer.data = (unsigned long)&fpga_timer;
	fpga_timer.timer.function = timer_periodic;

	add_timer(&fpga_timer.timer);
}
void update_fpga_lcd(int status){
    char tmp1, tmp2;
    int i;
    if(status == NTH_UPDATE){
        tmp1 = id_buf[LCD_LINE_BUF_LEN - 1];
        tmp2 = name_buf[LCD_LINE_BUF_LEN - 1];
        for(i=LCD_LINE_BUF_LEN - 1; i>0; i--){
            id_buf[i] = id_buf[i - 1];
            name_buf[i] = name_buf[i - 1];
        }
        id_buf[0] = tmp1;
        name_buf[0] = tmp2;
    }
    strncpy(lcd_buf, id_buf, LCD_LINE_BUF_LEN);
    lcd_buf[LCD_LINE_BUF_LEN] = 0;
    strncat(lcd_buf, name_buf, LCD_LINE_BUF_LEN);
    lcd_buf[LCD_MAX_LEN] = 0;

    iom_fpga_text_lcd_write(lcd_buf, LCD_MAX_LEN);
}
void update_fpga_fnd(int status){
    unsigned char data[4];
    int i;
    if(status == NTH_UPDATE){
        fnd_val = fnd_val % 8 + 1;
        if(fnd_val == start_val)
            fnd_idx = (fnd_idx + 1) % 4;
    }
    
    for(i=0; i<4; i++)
        data[i] = 0;  
    data[(int)fnd_idx] = fnd_val;  
    iom_fpga_fnd_write(data, 4);
}
void update_fpga_led(void){
    unsigned char data;
    data = 1 << (8 - fnd_val);
    iom_led_write(data);
}
void update_fpga_dot(void){
    iom_fpga_dot_write(fpga_number[(int)fnd_val], DOT_LEN);
}
void update_fpga_all(int status){
    update_fpga_lcd(status);
    update_fpga_fnd(status);
    update_fpga_led();
    update_fpga_dot();
}

/* initialize devices */
void clear_fpga_lcd(void){
    memset(lcd_buf, 0, LCD_MAX_LEN);
    iom_fpga_text_lcd_write(lcd_buf, LCD_MAX_LEN);
}
void clear_fpga_fnd(void){
    unsigned char data[4];
    int i;
    for(i=0; i<4; i++)
        data[i] = 0;
    iom_fpga_fnd_write(data, 4);
}
void clear_fpga_led(void){
    iom_led_write(0);
}
void clear_fpga_dot(void){
    iom_fpga_dot_write(fpga_set_blank, DOT_LEN);
}
void clear_fpga_all(void){
    clear_fpga_lcd();
    clear_fpga_fnd();
    clear_fpga_led();
    clear_fpga_dot();
}
/*
    Util functions
*/


// when write to led device  ,call this function
ssize_t iom_led_write(unsigned char value)
{
    unsigned short _s_value;

    _s_value = (unsigned short)value;
    outw(_s_value, (unsigned int)iom_fpga_led_addr);

    return 1;
}


// when write to fnd device  ,call this function
ssize_t iom_fpga_fnd_write(unsigned char value[4], size_t length) 
{
	unsigned short int value_short = 0;
	
    value_short = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
    outw(value_short,(unsigned int)iom_fpga_fnd_addr);	    

	return length;
}


// when write to fpga_dot device  ,call this function
ssize_t iom_fpga_dot_write(unsigned char value[10], size_t length) 
{
	int i;
	unsigned short int _s_value;
	
	for(i=0;i<length;i++)
    {
        _s_value = value[i] & 0x7F;
		outw(_s_value,(unsigned int)iom_fpga_dot_addr+i*2);
    }
	
	return length;
}


// when write to fpga_text_lcd device  ,call this function
ssize_t iom_fpga_text_lcd_write(unsigned char value[33], size_t length)
{
    int i;

    unsigned short int _s_value = 0;

    // printk("Get Size : %d / String : %s\n", length, value);

    for (i = 0; i < length; i++)
    {
        _s_value = ((value[i] & 0xFF) << 8) | (value[i + 1] & 0xFF);
        outw(_s_value, (unsigned int)iom_fpga_text_lcd_addr + i);
        i++;
    }
    // printk("lcd_write finished\n");
    return length;
}

int __init iom_fpga_init(void)
{
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
