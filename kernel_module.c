#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/proc_fs.h> 
#include <linux/uaccess.h> 
#include <linux/version.h> 
#include <linux/list.h>
#include <linux/slab.h>


#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0) 
#define HAVE_PROC_OPS 
#endif 

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

#define PROCFS_MAX_SIZE 1024 
#define PROCFS_NAME "quim_module" 
 
MODULE_AUTHOR("Joaquim Ferrer");
MODULE_DESCRIPTION("Simple Kernel Module. Write and read into linked list in kernel memory space");
MODULE_LICENSE("GPL");

/*  /proc file */ 

static struct proc_dir_entry *proc_file; 

 
/* Linked List Node*/
struct k_node {
    struct list_head list;
    char * buffer;
    int len;
};

/* Linked List */
struct list_head list;

/* The size of the buffer */ 
static unsigned long procfs_buffer_size = 0; 

 

/* /proc file read */ 
static ssize_t procfile_read(struct file *filePointer, char __user *buffer, size_t buffer_length, loff_t *offset) 
{ 

    ssize_t ret = 0;
    ssize_t read_len;
    ssize_t bytes_read = 0;
    struct k_node * read_node;

    pr_info("Actual offset %lld\n", *offset);

    while(!list_empty(&list) && bytes_read < buffer_length){
	
	/* Getting the next node in list*/
	pr_info("procfile reading");
	read_node = list_first_entry(&list, struct k_node, list);

	if(*offset < 0 || *offset > read_node->len){
	    ret = -EINVAL;
	    break;
	}

	/* Bytes to read = MIN(remining on entry, bytes left to read) */
	read_len = MIN(read_node->len - *offset, buffer_length-bytes_read);
	
	/* Copy buffer to user buffer */
	if (copy_to_user(buffer+bytes_read, (read_node->buffer)+*offset, read_len)) { 
	    pr_info("copy_to_user failed\n"); 
	    ret = 0; 
	    break;
	} 

	if (read_len == read_node->len - *offset){
	    
	    /* all entry was read */
	    bytes_read += read_node->len - *offset;
	    *offset = 0;

	    /* unallocating used space*/
	    kfree(read_node->buffer);
	    kfree(read_node);
	    list_del(list.next);
	} 
	else {
	    
	    /* There's still entry to read. We stoped in the middle of the entry */
		*offset += buffer_length - bytes_read;
		bytes_read += buffer_length - bytes_read;
	}

	ret = bytes_read;
    }
    return ret; 
} 

 

/* This function is called with the /proc file is written. */ 

static ssize_t procfile_write(struct file *file, const char __user *buff, size_t len, loff_t *off) 
{ 

    char * procfs_buffer;
    struct k_node * newNode;
    procfs_buffer_size = len; 

    if (procfs_buffer_size > PROCFS_MAX_SIZE) 
        procfs_buffer_size = PROCFS_MAX_SIZE; 

    procfs_buffer = kmalloc(procfs_buffer_size,GFP_KERNEL);

    if (copy_from_user(procfs_buffer, buff, procfs_buffer_size)) 
        return -EFAULT; 

 
    newNode = kmalloc(sizeof(struct k_node), GFP_KERNEL);
    newNode->buffer = procfs_buffer;
    newNode->len    = procfs_buffer_size;
    list_add_tail(&(newNode->list), &list);

    procfs_buffer[procfs_buffer_size & (PROCFS_MAX_SIZE - 1)] = '\0'; 

    pr_info("procfile write %s\n", procfs_buffer); 

    return procfs_buffer_size; 
} 

 

#ifdef HAVE_PROC_OPS 

static const struct proc_ops proc_file_fops = { 
    .proc_read = procfile_read, 
    .proc_write = procfile_write, 
}; 

#else 

static const struct file_operations proc_file_fops = { 
    .read = procfile_read, 
    .write = procfile_write, 
}; 

#endif 

 

static int __init procfs_init(void) 
{ 

    proc_file = proc_create(PROCFS_NAME, 0644, NULL, &proc_file_fops); 
    if (NULL == proc_file) { 
        proc_remove(proc_file); 
        pr_alert("Error:Could not initialize /proc/%s\n", PROCFS_NAME); 
        return -ENOMEM; 
    } 
 
    INIT_LIST_HEAD(&list);

    pr_info("/proc/%s created\n", PROCFS_NAME); 
    return 0; 
} 

 

static void __exit procfs_exit(void) 
{ 

    struct k_node *cursor, *temp;
    proc_remove(proc_file); 

    /* Destroy all linked list */
    list_for_each_entry_safe(cursor, temp, &list, list) {
	list_del(&cursor->list);
	if (cursor->buffer != NULL){
	    kfree(cursor->buffer);
	}
        kfree(cursor);
    }

    pr_info("/proc/%s removed. Linked list destroyed\n", PROCFS_NAME); 
} 

module_init(procfs_init); 
module_exit(procfs_exit); 

 

