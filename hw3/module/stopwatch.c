#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <mach/gpio.h>
#include <linux/platform_device.h>
#include <asm/gpio.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/cdev.h>

/* Define stopwatch states */
#define STATE_RUNNING 0
#define STATE_PAUSED 1
#define STATE_INIT 2
#define FPGA_FND_ADDRESS 0x08000004 // physical address

/* struct for kernel timer */
struct struct_timer_data {
	struct timer_list timer;
 	int count;
} ;

/* driver variables */
static int inter_major=246, inter_minor=0;
static int result;
static dev_t inter_dev;
static struct cdev inter_cdev;
static int stopwatch_usage = 0;
static int quit;

/* fpga_fnd variables */
static unsigned char *fpga_fnd_addr;
unsigned char fnd_data[4];

/* timer variables */
int total_elapsed_time;
unsigned long last_jiffies;
struct struct_timer_data timer_sec, timer_exit;
int paused;

/* Decare and initialize wait_queue_head_t q */
DECLARE_WAIT_QUEUE_HEAD(q);


static int inter_open(struct inode *, struct file *);
static int inter_release(struct inode *, struct file *);
static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

irqreturn_t inter_HOME_btn(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_BACK_btn(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_VOLUP_btn(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_VOLDOWN_btn(int irq, void* dev_id, struct pt_regs* reg);

void set_fnd_data(int total_sec);
void initialize_device(void);
ssize_t fpga_fnd_write(unsigned char value[4], size_t length);
void set_timer_timer_sec(void);
void set_timer_timer_exit(void);
static void timer_sec_periodic(unsigned long timeout);
static void timer_exit_function(unsigned long timeout);
void resume_timer_timer_sec(void);

static struct file_operations inter_fops =
{
	.owner = THIS_MODULE,
	.open = inter_open,
	.write = inter_write,
	.release = inter_release,
};

/* ***************************************
 * Calculate fnd_data with total_elapsed_time
 * fnd_data[4] = [HH:MM]
 * ***************************************/
void set_fnd_data(int total_sec){

	int min = total_sec / 60;
	int sec = total_sec % 60;
	// printk(KERN_ALERT "min:sec = %d %d\n", min, sec);

	fnd_data[0] = min % 100 / 10; /* min value cannot exceed 100 */
	fnd_data[1] = min % 10;
	fnd_data[2] = sec / 10;
	fnd_data[3] = sec % 10;
}
/* ***************************************
 * Initialize device driver variables, fpga_fnd device and timers
 * Set fpga_fnd device [0000]
 * Initialize device status
 * Delete all timers
 * ***************************************/
void initialize_device(void){
	int i;
	/* initialize fpga_fnd */
	for(i=0; i<4; i++) fnd_data[i] = 0;
	fpga_fnd_write(fnd_data, 4);
	
	/* init status variable */
	paused = STATE_INIT;
	total_elapsed_time = 0;

	/* Delete all timers */
    del_timer_sync(&timer_sec.timer);
	/* Do not delete timer_exit because the only way to 
	 * wakeup process(exit program) is calling timer_exit_function(). */
}
/* ***************************************
 * HOME button : start timer
 * Print elapsed time in fpga_fnd
 * Use timer to measure time  
 * ***************************************/
irqreturn_t inter_HOME_btn(int irq, void* dev_id, struct pt_regs* reg) {

	if(paused == STATE_INIT) {
		paused = STATE_RUNNING;
		set_timer_timer_sec();
	}
	return IRQ_HANDLED;
}
/* ***************************************
 * BACK button : pause timer
 * Use timer to measure time  
 * ***************************************/
irqreturn_t inter_BACK_btn(int irq, void* dev_id, struct pt_regs* reg) {

        switch(paused){
		case STATE_RUNNING:
			paused = STATE_PAUSED;
			del_timer_sync(&timer_sec);
			break;
		case STATE_PAUSED:
			paused = STATE_RUNNING;
			resume_timer_timer_sec();
			break;
		case STATE_INIT:
			break;
		}
		return IRQ_HANDLED;
}
/* ***************************************
 * VOL+ button : reset timer
 * Print initial time(00:00) in fpga_fnd
 * ***************************************/
irqreturn_t inter_VOLUP_btn(int irq, void* dev_id,struct pt_regs* reg) {

        initialize_device();
		return IRQ_HANDLED;
}
/* ***************************************
 * VOL- button : exit user application
 * press button for 3 secs, exit user application
 * initialize device(fpga_fnd)
 * ***************************************/
irqreturn_t inter_VOLDOWN_btn(int irq, void* dev_id, struct pt_regs* reg) {
        
		if(gpio_get_value(IMX_GPIO_NR(5, 14)) == 0){
			/* VOL- button pressed */
			/* add timer_exit and 
			 * If pressing time is greater than 3 seconds, wakeup process */
			set_timer_timer_exit();
		} 
		else if(gpio_get_value(IMX_GPIO_NR(5, 14)) == 1){
			/* VOL- button released */
			/* If pressing time is less than 3 seconds, delete timer_exit */
			del_timer_sync(&timer_exit);
		}
		
		return IRQ_HANDLED;
}
/* ***************************************
 * When timer_sec expires, call this function.
 * count total_elasped_time, update fpga_fnd device
 * ***************************************/
static void timer_sec_periodic(unsigned long timeout) {

	if(paused == STATE_RUNNING){
		total_elapsed_time++;
		set_fnd_data(total_elapsed_time);
		fpga_fnd_write(fnd_data, 4);
		last_jiffies = get_jiffies_64();

		timer_sec.timer.expires = last_jiffies + (1 * HZ);
		timer_sec.timer.data = (unsigned long)&timer_sec;
		timer_sec.timer.function = timer_sec_periodic;
		add_timer(&timer_sec.timer);
	}   
}
/* ***************************************
 * When timer_exit expires, call this function.
 * Wakeup user process.
 * Set quit flag as 1 to wakeup process.
 * ***************************************/
static void timer_exit_function(unsigned long timeout) {
	quit = 1;	
	__wake_up(&q, 1, 1, NULL);
	initialize_device();
	printk(KERN_ALERT "WAKE UP!\n");
	return ;
}
/* ***************************************
 * Set and add timer_sec.
 * timer_sec increases fpga_fnd number +1 per 1 seconds.
 * ***************************************/
void set_timer_timer_sec(void){
    del_timer_sync(&timer_sec.timer);
    timer_sec.count = 0;
	/* save current jiffies to resume timer */
	last_jiffies = get_jiffies_64();
    timer_sec.timer.expires = last_jiffies + (1 * HZ);
	timer_sec.timer.data = (unsigned long)&timer_sec;
	timer_sec.timer.function = timer_sec_periodic;
    add_timer(&timer_sec.timer);
}
/* ***************************************
 * Set and add timer_exit.
 * timer_exit measure pressing VOL- button time. 
 * If button pressed for 3 seconds, set flag to wakeup process.
 * ***************************************/
void set_timer_timer_exit(void){
    del_timer_sync(&timer_exit.timer);
    timer_exit.count = 0;
	/* After 3 seconds, set flag to waketup process */
    timer_exit.timer.expires = get_jiffies_64() + (3 * HZ);
	timer_exit.timer.data = (unsigned long)&timer_exit;
	timer_exit.timer.function = timer_exit_function;
    add_timer(&timer_exit.timer);
}
/* ***************************************
 * Resume timer_sec.
 * When timer_sec was paused and resume, this function is called.
 * timer expiration time is calculated taking into account the micro seconds.
 * ***************************************/
void resume_timer_timer_sec(void){
	// printk(KERN_ALERT "resume::Expire time = %ld\n", last_jiffies + (1 * HZ));

    del_timer_sync(&timer_sec.timer);
    timer_sec.timer.expires = last_jiffies + (1 * HZ);
	timer_sec.timer.data = (unsigned long)&timer_sec;
	timer_sec.timer.function = timer_sec_periodic;
    add_timer(&timer_sec.timer);
}
/* ***************************************
 * When device is opened, this function is called.
 * set stopwatch usage flag 1 and initialize device.
 * Register irqs on a target device buttons.
 * ***************************************/
static int inter_open(struct inode *minode, struct file *mfile){
	int ret;
	int irq;
	printk(KERN_ALERT "Open Module\n");

	/* Check device usage */
	if (stopwatch_usage != 0) {
		return -EBUSY;
	}
	/* Assign default value to variables  */
	stopwatch_usage = 1;
	quit = 0;

	/* Initialize some variables and timer */
	initialize_device();
	/* Initialize waitqueue head */
	init_waitqueue_head(&q);

	/* Register irqs on a target device buttons */
	// inter_HOME_btn
	gpio_direction_input(IMX_GPIO_NR(1,11));
	irq = gpio_to_irq(IMX_GPIO_NR(1,11));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_HOME_btn, IRQF_TRIGGER_FALLING, "HOME", 0);

	// inter_BACK_btn
	gpio_direction_input(IMX_GPIO_NR(1,12));
	irq = gpio_to_irq(IMX_GPIO_NR(1,12));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_BACK_btn, IRQF_TRIGGER_FALLING, "BACK", 0);

	// inter_VOLUP_btn
	gpio_direction_input(IMX_GPIO_NR(2,15));
	irq = gpio_to_irq(IMX_GPIO_NR(2,15));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_VOLUP_btn, IRQF_TRIGGER_FALLING, "VOL+", 0);

	// inter_VOLDOWN_btn
	gpio_direction_input(IMX_GPIO_NR(5,14));
	irq = gpio_to_irq(IMX_GPIO_NR(5,14));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_VOLDOWN_btn, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "VOL-", 0);

	return 0;
}
/* ***************************************
 * when stopwatch device is closed, call this function 
 * Free interrupt handler for target board's buttons
 * and set usage flag 0.
 * Return : 0 if success
 * ***************************************/
static int inter_release(struct inode *minode, struct file *mfile){
	stopwatch_usage = 0;
	/* Free an interrupt allocated with request_irq.  
	 * ************************************************************************
	 * free_irq() : Remove an interrupt handler. The handler is removed and 
	 * if the interrupt line is no longer in use by any driver it is disabled. 
	 * On a shared IRQ the caller must ensure the interrupt is disabled on the 
	 * card it drives before calling this function. The function does not return 
	 * until any executing interrupts for this IRQ have completed.
	 * This function must not be called from interrupt context.  
	 * ************************************************************************/
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 12)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL);
	
	printk(KERN_ALERT "Release Module\n");
	return 0;
}
/* ***************************************
 * when write fpga_fnd device, call this function 
 * Return : non-negative value if success
 * ***************************************/
ssize_t fpga_fnd_write(unsigned char value[4], size_t length) {
    unsigned short int value_short = 0;
    value_short = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
    outw(value_short,(unsigned int)fpga_fnd_addr);	    
	return length;
}
/* ***************************************
 * when write stopwatch device, call this function 
 * Sleep user program until press VOL- button for 3 seconds.
 * Return : 0 
 * ***************************************/
static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos ){
	/* sleep current user process */
	interruptible_sleep_on(&q);
	return 0;
}
/* ***************************************
 * This function is called in inter_init() function.
 * Register device and mmap fpga_fnd device address.
 * Return : 0 if success, otherwise error
 * ***************************************/
static int inter_register_cdev(void)
{
	int error;
	if(inter_major) {
		inter_dev = MKDEV(inter_major, inter_minor);
		error = register_chrdev_region(inter_dev,1,"inter");
	}else{
		error = alloc_chrdev_region(&inter_dev,inter_minor,1,"inter");
		inter_major = MAJOR(inter_dev);
	}
	if(error<0) {
		printk(KERN_WARNING "inter: can't get major %d\n", inter_major);
		return result;
	}
	printk(KERN_ALERT "major number = %d\n", inter_major);
	cdev_init(&inter_cdev, &inter_fops);
	inter_cdev.owner = THIS_MODULE;
	inter_cdev.ops = &inter_fops;
	/* mmap fpga_fnd device address */
	fpga_fnd_addr = ioremap(FPGA_FND_ADDRESS, 0x4);

	error = cdev_add(&inter_cdev, inter_dev, 1);
	if(error) {
		printk(KERN_NOTICE "inter Register Error %d\n", error);
	}
	return 0;
}
/* ***************************************
 * This function is called When stopwatch module is inserted into kernel.
 * Call inter_register_cdev() function to init and init timers.
 * Return : 0 if success
 * ***************************************/
static int __init inter_init(void) {
	int result;
	if((result = inter_register_cdev()) < 0 )
		return result;
	printk(KERN_ALERT "Init Module Success \n");
	printk(KERN_ALERT "Device : /dev/stopwatch, Major Num : 246 \n");
	init_timer(&timer_sec.timer);    
	init_timer(&timer_exit.timer);  
	return 0;
}
/* ***************************************
 * This function is called When stopwatch module is removed from kernel.
 * unmap fpga_fnd device, unregister stopwatch device and delete timers.
 * ***************************************/
static void __exit inter_exit(void) {
	/* unmap fpga_fnd address */
	iounmap(fpga_fnd_addr);
	/* release stopwatch device */
	cdev_del(&inter_cdev);
	unregister_chrdev_region(inter_dev, 1);
    del_timer_sync(&timer_sec.timer);
    del_timer_sync(&timer_exit.timer);

	printk(KERN_ALERT "Remove Module Success \n");
}

module_init(inter_init);
module_exit(inter_exit);
MODULE_LICENSE("GPL");
