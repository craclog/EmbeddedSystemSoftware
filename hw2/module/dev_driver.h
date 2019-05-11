#ifndef __DEV_DRIVER_H__
#define __DEV_DRIVER_H__

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


#define IOM_FPGA_LED_ADDRESS 0x08000016 // pysical address
#define IOM_FPGA_FND_ADDRESS 0x08000004 // pysical address
#define IOM_FPGA_DOT_ADDRESS 0x08000210 // pysical address
#define IOM_FPGA_TEXT_LCD_ADDRESS 0x08000090   // pysical address - 32 Byte (16 * 2)

#define IOM_FPGA_MAJOR 242
#define IOM_FPGA_NAME "dev_driver"

#define SUCCESS 1
#define LCD_LINE_BUF_LEN 16
#define LCD_MAX_LEN 32
#define DOT_LEN 10
#define RIGHT 0
#define LEFT 1

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
static char id_dir = RIGHT;
static char name_dir = RIGHT;

/* Define functions */
ssize_t iom_led_write(unsigned char value);
ssize_t iom_fpga_fnd_write(unsigned char value[4], size_t length);
ssize_t iom_fpga_dot_write(unsigned char value[10], size_t length);
ssize_t iom_fpga_text_lcd_write(unsigned char value[33], size_t length);
ssize_t iom_fpga_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
int iom_fpga_open(struct inode *minode, struct file *mfile);
int iom_fpga_release(struct inode *minode, struct file *mfile);
long iom_fpga_ioctl(struct file *inode, unsigned int ioctl_num, unsigned long param);

/* Timer functions */
void set_timer(void);
static void timer_periodic(unsigned long timeout);

/* update devices functions */
void update_fpga_data(void);
void update_fpga_lcd(void);
void update_fpga_fnd(void);
void update_fpga_led(void);
void update_fpga_dot(void);
void update_fpga_all(void);
/* clear devices functions */
void clear_fpga_lcd(void);
void clear_fpga_fnd(void);
void clear_fpga_led(void);
void clear_fpga_dot(void);
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




#endif