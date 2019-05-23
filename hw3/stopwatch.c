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

#define STATE_RUNNING 0
#define STATE_PAUSED 1
#define STATE_INIT 2
#define FPGA_FND_ADDRESS 0x08000004 // physical address

struct struct_timer_data {
	struct timer_list timer;
 	int count;
} ;

static int inter_major=246, inter_minor=0;
static int result;
static dev_t inter_dev;
static struct cdev inter_cdev;

/* fpga_fnd variables */
static unsigned char *fpga_fnd_addr;
unsigned char fnd_data[4];

/* timer variables */
int total_elapsed_time;
unsigned long last_jiffies;
struct struct_timer_data timer_sec, timer_exit;
int paused;
/* wait_queue_head_t q */
DECLARE_WAIT_QUEUE_HEAD(q);


static int inter_open(struct inode *, struct file *);
static int inter_release(struct inode *, struct file *);
static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

irqreturn_t inter_handler1(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler2(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler3(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler4(int irq, void* dev_id, struct pt_regs* reg);

ssize_t fpga_fnd_write(unsigned char value[4], size_t length);
void set_timer_timer_sec(void);
void set_fnd_data(int total_sec);
void initialize_device(void);
static void timer_sec_periodic(unsigned long timeout);
void resume_timer_timer_sec(void);

static struct file_operations inter_fops =
{
	.owner = THIS_MODULE,
	.open = inter_open,
	.write = inter_write,
	.release = inter_release,
};

/*
 * Calculate fnd_data with total_elapsed_time
 * fnd_data[4] = [HH:MM]
 */
void set_fnd_data(int total_sec){

	int min = total_sec / 60;
	int sec = total_sec % 60;
	printk(KERN_ALERT "min:sec = %d %d\n", min, sec);

	fnd_data[0] = min % 100 / 10; /* min value cannot exceed 100 */
	fnd_data[1] = min % 10;
	fnd_data[2] = sec / 10;
	fnd_data[3] = sec % 10;
}

void initialize_device(void){
	int i;
	/* initialize fpga_fnd */
	for(i=0; i<4; i++) fnd_data[i] = 0;
	fpga_fnd_write(fnd_data, 4);
	
	/* init status variable */
	paused = STATE_INIT;
	total_elapsed_time = 0;

	// delete timer all
	// 
}
/*
 * HOME button : start timer
 * Print elapsed time in fpga_fnd
 * Use timer to measure time  
 */
irqreturn_t inter_HOME_btn(int irq, void* dev_id, struct pt_regs* reg) {
	printk(KERN_ALERT "inter_HOME_btn = %x\n", gpio_get_value(IMX_GPIO_NR(1, 11)));
	
	if(paused == STATE_INIT) {
		paused = STATE_RUNNING;
		set_timer_timer_sec();
	}
	return IRQ_HANDLED;
}
/*
 * BACK button : pause timer
 * Use timer to measure time  
 */
irqreturn_t inter_BACK_btn(int irq, void* dev_id, struct pt_regs* reg) {
        printk(KERN_ALERT "inter_BACK_btn = %x\n", gpio_get_value(IMX_GPIO_NR(1, 12)));
        switch(paused){
		case STATE_RUNNING:
			paused = STATE_PAUSED;
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
/*
 * VOL+ button : reset timer
 * Print initial time(00:00) in fpga_fnd
 */
irqreturn_t inter_VOLUP_btn(int irq, void* dev_id,struct pt_regs* reg) {
        printk(KERN_ALERT "inter_VOLUP_btn!!! = %x\n", gpio_get_value(IMX_GPIO_NR(2, 15)));
        initialize_device();
		return IRQ_HANDLED;
}
/*
 * VOL- button : exit user application
 * press button for 3 secs, exit user application
 * initialize device(fpga_fnd)
 */
irqreturn_t inter_VOLDOWN_btn(int irq, void* dev_id, struct pt_regs* reg) {
        printk(KERN_ALERT "inter_VOLDOWN_btn!!! = %x\n", gpio_get_value(IMX_GPIO_NR(5, 14)));
        initialize_device();
		
		__wake_up(&q, 1, 1, NULL);
		printk(KERN_ALERT "WAKE UP!\n");
		return IRQ_HANDLED;
}
/*
 * When timer_sec expires, call this function.
 * count total_elasped_time, update fpga_fnd device
 */
static void timer_sec_periodic(unsigned long timeout) {
	// struct struct_timer_data *p_data = (struct struct_timer_data*)timeout;
	printk(KERN_ALERT "timer_sec_periodic = %d\n", total_elapsed_time);

	if(paused == STATE_RUNNING){
		total_elapsed_time++;
		set_fnd_data(total_elapsed_time);
		printk(KERN_ALERT "%d %d %d %d\n", fnd_data[0], fnd_data[1], fnd_data[2], fnd_data[3]);

		fpga_fnd_write(fnd_data, 4);
		last_jiffies = get_jiffies_64();
		timer_sec.timer.expires = get_jiffies_64() + (1 * HZ);
		timer_sec.timer.data = (unsigned long)&timer_sec;
		timer_sec.timer.function = timer_sec_periodic;
		add_timer(&timer_sec.timer);
	}   
}
/*
 * Set and add timer_sec.
 */
void set_timer_timer_sec(void){
    del_timer_sync(&timer_sec.timer);
    timer_sec.count = 0;
	/* save current jiffies to resume timer */
	last_jiffies = get_jiffies_64();
    timer_sec.timer.expires = get_jiffies_64() + (1 * HZ);
	timer_sec.timer.data = (unsigned long)&timer_sec;
	timer_sec.timer.function = timer_sec_periodic;
    add_timer(&timer_sec.timer);
}
/*
 * Resume timer_sec.
 */
void resume_timer_timer_sec(void){
    del_timer_sync(&timer_sec.timer);
    timer_sec.count = 0;
    timer_sec.timer.expires = last_jiffies + (1 * HZ);
	timer_sec.timer.data = (unsigned long)&timer_sec;
	timer_sec.timer.function = timer_sec_periodic;
    add_timer(&timer_sec.timer);
}
static int inter_open(struct inode *minode, struct file *mfile){
	int ret;
	int irq;
	printk(KERN_ALERT "Open Module\n");

	/* Initialize some variables and timer */
	initialize_device();

	// inter_HOME_btn
	gpio_direction_input(IMX_GPIO_NR(1,11));
	irq = gpio_to_irq(IMX_GPIO_NR(1,11));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_HOME_btn, IRQF_TRIGGER_FALLING, "qwer", 0);

	// inter_BACK_btn
	gpio_direction_input(IMX_GPIO_NR(1,12));
	irq = gpio_to_irq(IMX_GPIO_NR(1,12));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_BACK_btn, IRQF_TRIGGER_FALLING, "qwer", 0);

	// inter_VOLUP_btn
	gpio_direction_input(IMX_GPIO_NR(2,15));
	irq = gpio_to_irq(IMX_GPIO_NR(2,15));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_VOLUP_btn, IRQF_TRIGGER_FALLING, "qwer", 0);

	// inter_VOLDOWN_btn
	gpio_direction_input(IMX_GPIO_NR(5,14));
	irq = gpio_to_irq(IMX_GPIO_NR(5,14));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_VOLDOWN_btn, IRQF_TRIGGER_FALLING, "qwer", 0);

	return 0;
}

static int inter_release(struct inode *minode, struct file *mfile){
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 12)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL);
	
	printk(KERN_ALERT "Release Module\n");
	return 0;
}
/*
 * when write fnd device, call this function 
 * return non-negative value if success
 */
ssize_t fpga_fnd_write(unsigned char value[4], size_t length) {
    unsigned short int value_short = 0;
    value_short = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
    outw(value_short,(unsigned int)fpga_fnd_addr);	    
	return length;
}
static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos ){
	//sleep current process
	interruptible_sleep_on(&q);
	return 0;
}

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
	fpga_fnd_addr = ioremap(FPGA_FND_ADDRESS, 0x4);

	error = cdev_add(&inter_cdev, inter_dev, 1);
	if(error) {
		printk(KERN_NOTICE "inter Register Error %d\n", error);
	}
	return 0;
}

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

static void __exit inter_exit(void) {
	iounmap(fpga_fnd_addr);
	cdev_del(&inter_cdev);
	unregister_chrdev_region(inter_dev, 1);
	printk(KERN_ALERT "Remove Module Success \n");
}

module_init(inter_init);
module_exit(inter_exit);
MODULE_LICENSE("GPL");
