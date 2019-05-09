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

#include "./fpga_dot_font.h"

#define IOM_LED_ADDRESS 0x08000016 // pysical address
#define IOM_FND_ADDRESS 0x08000004 // pysical address
#define IOM_FPGA_DOT_ADDRESS 0x08000210 // pysical address
#define IOM_FPGA_TEXT_LCD_ADDRESS 0x08000090   // pysical address - 32 Byte (16 * 2)

#define IOM_FPGA_MAJOR 242
#define IOM_FPGA_NAME "dev_driver"

//Global variable

static unsigned char *iom_fpga_led_addr;
static unsigned char *iom_fpga_fnd_addr;
static unsigned char *iom_fpga_dot_addr;
static unsigned char *iom_fpga_text_lcd_addr;
static int fpga_usage = 0;

// define functions...
ssize_t iom_led_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t iom_fpga_fnd_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t iom_fpga_dot_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t iom_fpga_text_lcd_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t iom_fpga_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
int iom_fpga_open(struct inode *minode, struct file *mfile);
int iom_fpga_release(struct inode *minode, struct file *mfile);

// define file_operations structure
struct file_operations iom_fpga_text_lcd_fops =
    {
        .owner : THIS_MODULE,
        .open : iom_fpga_open,
        .write : iom_fpga_write,
        .unlocked_ioctl = iom_fpga_ioctl,
        .release : iom_fpga_release
    };

ssize_t iom_fpga_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what){

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
long iom_fpga_ioctl(struct file *inode, unsigned int ioctl_num, unsigned long param){

}



// when write to led device  ,call this function
ssize_t iom_led_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what)
{
    unsigned char value;
    unsigned short _s_value;
    const char *tmp = gdata;

    if (copy_from_user(&value, tmp, 1))
        return -EFAULT;

    _s_value = (unsigned short)value;
    outw(_s_value, (unsigned int)iom_fpga_led_addr);

    return length;
}


// when write to fnd device  ,call this function
ssize_t iom_fpga_fnd_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
	int i;
	unsigned char value[4];
	unsigned short int value_short = 0;
	const char *tmp = gdata;

	if (copy_from_user(&value, tmp, 4))
		return -EFAULT;

    value_short = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
    outw(value_short,(unsigned int)iom_fpga_fnd_addr);	    

	return length;
}


// when write to fpga_dot device  ,call this function
ssize_t iom_fpga_dot_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
	int i;

	unsigned char value[10];
	unsigned short int _s_value;
	const char *tmp = gdata;

	if (copy_from_user(&value, tmp, length))
		return -EFAULT;

	for(i=0;i<length;i++)
    {
        _s_value = value[i] & 0x7F;
		outw(_s_value,(unsigned int)iom_fpga_dot_addr+i*2);
    }
	
	return length;
}


// when write to fpga_text_lcd device  ,call this function
ssize_t iom_fpga_text_lcd_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what)
{
    int i;

    unsigned short int _s_value = 0;
    unsigned char value[33];
    const char *tmp = gdata;

    if (copy_from_user(&value, tmp, length))
        return -EFAULT;

    value[length] = 0;
    printk("Get Size : %d / String : %s\n", length, value);

    for (i = 0; i < length; i++)
    {
        _s_value = (value[i] & 0xFF) << 8 | value[i + 1] & 0xFF;
        outw(_s_value, (unsigned int)iom_fpga_text_lcd_addr + i);
        i++;
    }

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
	iom_fpga_led_addr = ioremap(IOM_LED_ADDRESS, 0x1);
	iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);
	iom_fpga_dot_addr = ioremap(IOM_FPGA_DOT_ADDRESS, 0x10);
	iom_fpga_text_lcd_addr = ioremap(IOM_TEXT_LCD_ADDRESS, 0x32);

	// init_timer(&(timerInfo.timer));

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
	// del_timer_sync(&timerInfo.timer);

	unregister_chrdev(IOM_FPGA_MAJOR, IOM_FPGA_NAME);
}

module_init(iom_fpga_init);
module_exit(iom_fpga_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("KiyoungYoon");
