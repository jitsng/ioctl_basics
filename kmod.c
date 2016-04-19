#include <asm/highmem.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <asm/ioctl.h>		// echo @%: to find out the name of a file in the vim 
#include <linux/slab.h>

#define IOCTL_TYPE 	15	// from 0-255

MODULE_LICENSE("Dual BSD/GPL");

/* Constants */
#define FIRST_MINOR 	10
#define NR_DEVS 	1	//number of device numbers

// Function declaration for syscall definitions

int myOpen (struct inode *inode, struct file *filep);
int myRelease (struct inode *in, struct file *fp);
int my_ioctl (struct file *, int, void *);

// Initialization routines

static int myInit (void);
static void myExit (void);

struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = myOpen,
	.release = myRelease,
	.unlocked_ioctl = my_ioctl
};

/* Global Variable */

char *devname = "xyz";	//contains device name
//int majno;		// no use here
static dev_t mydev;	// encodes major no and minor no
struct cdev *my_cdev;	// holds char device driver descriptor

/* to accept input from command line */
// module_param (devname, charp, 0000);

// class and device structures
static struct class *mychar_class;
static struct device *mychar_device;

/*
	my_ioctl: to perform a task on a process in this case
*/

int my_ioctl (struct file *fd, int cmd, void *ptr)
{
	struct task_struct *task;	
	int dir;
	pmd_t *vir_pmd;
	pte_t *vir_pte;
	void *vir_pfram;
	unsigned int addr;		// Only to be used for PAGE_TABLE_LEVELS = 3
	unsigned int ofset_pgd; 	// 2
	unsigned int ofset_pmt; 	// 9
	unsigned int ofset_pt;  	// 9 
	unsigned int ofset_pfram; 	// 12

	if (IOCTL_TYPE != _IOC_TYPE(cmd)) {
		printk ("IOCTL Type failed\n");
		return -1;
	}
	dir = _IOC_DIR (cmd);
	switch (dir) {
		case _IOC_NONE:
				
				addr = ((unsigned int *) ptr);
				printk ("addr: %x\n", addr);
				ofset_pgd = addr >> 30;
				printk ("pgd: %x\n", ofset_pgd);
				ofset_pmt = (addr & ((~((~0) << 9)) << 21)) >> 21;
				printk ("pmt: %x\n", ofset_pmt);
				ofset_pt = (addr & ((~((~0) << 9 )) << 12)) >> 12;	
				printk ("pt: %x\n", ofset_pt);
				ofset_pfram = addr & (~((~0) << 12));
				printk ("pfram: %x\n", ofset_pfram);	
					

				task = pid_task(find_get_pid(*((int *)ptr)), 0);
				printk ("pgd_t: %x pgd: %x\n",task->mm->pgd + ofset_pgd, task->mm->pgd); // virtual addr of pgd + offset
					
				vir_pmd = (pmd_t *) kmap(&mem_map[((task->mm->pgd + ofset_pgd)->pgd) >> 12]); // pgd to pmd (1)
				printk("phy: %x\n", kmap(&mem_map[((task->mm->pgd + ofset_pgd)->pgd) >> 12]));

				vir_pte = (pte_t *) kmap(&mem_map[((vir_pmd + ofset_pmt)->pmd) >> 12]); // pmd to pte (2)
				printk("phy: %x\n", vir_pte);
				
				
				vir_pfram = kmap(&mem_map[((vir_pte + ofset_pt)->pte) >> 12]);	// pte to pgfram (3)
				printk("phy pfram: %x\n", (vir_pfram + ofset_pfram));
				 *((unsigned int *) (vir_pfram + ofset_pfram)) = 10;

				addr = (unsigned int *) (vir_pfram + ofset_pfram);
				printk ("addr: %x\n", addr);
				ofset_pgd = addr >> 30;
				printk ("pgd: %x\n", ofset_pgd);
				ofset_pmt = (addr & ((~((~0) << 9)) << 21)) >> 21;
				printk ("pmt: %x\n", ofset_pmt);
				ofset_pt = (addr & ((~((~0) << 9 )) << 12)) >> 12;	
				printk ("pt: %x\n", ofset_pt);
				ofset_pfram = addr & (~((~0) << 12));
				printk ("pfram: %x\n", ofset_pfram);	
					

				task = pid_task(find_get_pid(*((int *)ptr)), 0);
				printk ("pgd_t: %x pgd: %x\n",task->mm->pgd + ofset_pgd, task->mm->pgd); // virtual addr of pgd + offset
					
				vir_pmd = (pmd_t *) kmap(&mem_map[((task->mm->pgd + ofset_pgd)->pgd) >> 12]); // pgd to pmd (1)
				printk("phy: %x\n", kmap(&mem_map[((task->mm->pgd + ofset_pgd)->pgd) >> 12]));

				vir_pte = (pte_t *) kmap(&mem_map[((vir_pmd + ofset_pmt)->pmd) >> 12]);	// pmd to pte (2)
				printk("phy: %x\n", vir_pte);

				vir_pfram = kmap(&mem_map[((vir_pte + ofset_pt)->pte) >> 12]);	// pte to pgfram (3)
				printk("phy pfram: %x\n", (vir_pfram + ofset_pfram));
				printk("value: %x\n", *((unsigned int *) (vir_pfram + ofset_pfram)) = 35);
				
				break;

		case _IOC_READ:
				printk ("2\n");
				break;
		case _IOC_WRITE:
				printk ("3\n");
				break;

		case _IOC_READ | _IOC_WRITE:
						printk ("4\n");
						break;

	}

	return 0;
}


/*
	myOpen: open function of the psuedo driver
*/

int myOpen (struct inode *inode, struct file *filep)
{
	printk ("levels: %d\n", PAGETABLE_LEVELS);
	printk(KERN_INFO "Module/File Opened\n");
	return 0;

}

/*
	myRelease: close function of the psuedo driver
*/

int myRelease (struct inode *in, struct file *fp)
{
	printk(KERN_INFO "File Release\n");
	return 0;
}

// myinit: Init fun of the kernel module

static int __init myInit (void)
{
	int ret = -ENODEV;
	int status;

	printk (KERN_INFO "Initializing Character Device\n");
	
	// Allocating device numbes
	status = alloc_chrdev_region (&mydev, FIRST_MINOR, NR_DEVS, devname);
	if (status < 0) {
		printk (KERN_NOTICE "Device number allocation failed: %d\n", status);
		goto err;
	}	

	printk (KERN_INFO "Major number allocated = %d\n", MAJOR(mydev));
	my_cdev = cdev_alloc();	// allocating memory for my_cdev // my_cdev = kmalloc()
	if (my_cdev == NULL) {
		printk (KERN_ERR "cdev_alloc failed\n");
		goto err_cdev_alloc;	
	}
	
	cdev_init (my_cdev, &fops);	//initialize my_cdev with fops
	my_cdev->owner = THIS_MODULE;	

	status = cdev_add (my_cdev, mydev, NR_DEVS);	// add my_cdev to the list
	if (status) {
		printk (KERN_ERR "cdev_add failed\n");
		goto err_cdev_add;
	}

	// Create a class and an entry in sysfs
	mychar_class = class_create (THIS_MODULE, devname);
	if (IS_ERR(mychar_class)) {
		printk (KERN_ERR "class_create() failed\n");
		goto err_class_create;
	}

	// Create mychar_device in sysfs and device entry will in /dev directory

	mychar_device = device_create (mychar_class, NULL, mydev, NULL, devname);
	if (IS_ERR(mychar_device)) {
		printk (KERN_ERR "device_create() failed\n");
		goto err_device_create;		
	}

	return 0;

	err_device_create:
		class_destroy (mychar_class);

	err_class_create:
		cdev_del (my_cdev);

	err_cdev_add:
		kfree (my_cdev);
	
	err_cdev_alloc:
		unregister_chrdev_region (mydev, NR_DEVS);

	err:
		return ret;

}

static void myExit (void)
{
	printk (KERN_INFO "Existing the Character Driver\n");
	device_destroy (mychar_class, mydev);
	class_destroy (mychar_class);
	cdev_del (my_cdev);
	unregister_chrdev_region (mydev, NR_DEVS);
	return;
}
	
module_init (myInit);
module_exit (myExit);

