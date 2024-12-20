#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/spinlock.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include <linux/workqueue.h>

char buf[256];
loff_t pos = 0;
struct file *filep = NULL;
spinlock_t spinlock;
struct timespec64 time;

unsigned long long int get_kernel_time(void);
int debug_kernel_write(struct file *file, const void *buf);
struct rtc_time tm;
unsigned long flags;

#define WARNING (1<<1)
#define ERROR (1<<2)
#define cur_level (WARNING | ERROR)

struct work_struct debug_work;

void debug_work_func(struct work_struct *work)
{
    spin_lock_irqsave(&spinlock, flags);
    debug_kernel_write(filep, buf);
    spin_unlock_irqrestore(&spinlock, flags);
}

#define hust_printk(level,fmt,...) do{ if(level&cur_level){ \
spin_lock_irqsave(&spinlock,flags); \
sprintf(buf, "PANDA[%-4d][%-25s]:" fmt "",__LINE__,__FUNCTION__, ##__VA_ARGS__); \
schedule_work(&debug_work); \
flush_work(&debug_work); \
spin_unlock_irqrestore(&spinlock,flags); \
} \
}while(0)

int debug_kernel_write(struct file *file, const void *buf){
if(file){
   return kernel_write(file, buf, strlen(buf), &pos);
}
else{
    return -1;
}
}

static int __init init(void)
{
        char file_name[256] = {0};

        ktime_get_real_ts64(&(time));

        rtc_time_to_tm(time.tv_sec,&tm);

        sprintf(file_name, "/home/localhost/wh/log/log%4d%02d%02d%02d%02d%02d.txt", tm.tm_year+1900,tm.tm_mon+1, tm.tm_mday,tm.tm_hour+8,tm.tm_min,tm.tm_sec);
        printk("path %s\n",file_name);

        if(filep == NULL) {
                filep = filp_open(file_name, O_RDWR | O_APPEND | O_CREAT, 0644);
        }

        if (IS_ERR(filep)) {
                printk("Open file %s error\n", file_name);
                return -1;
        }
        spin_lock_init(&spinlock);
        INIT_WORK(&debug_work, debug_work_func);
        hust_printk(WARNING,"kernel write success %d\n",strlen(file_name));
        hust_printk(WARNING,"kernel write success\n");
        hust_printk(WARNING,"kernel write success %d\n",strlen(file_name));
        hust_printk(WARNING,"kernel write success\n");
        hust_printk(WARNING,"kernel write success %d\n",strlen(file_name));
        hust_printk(WARNING,"kernel write success\n");
        return 0;
}

static void __exit fini(void)
{
printk("Kernel read/write exit\n");
hust_printk(WARNING,"Kernel read/write exit\n");
if(filep != NULL) {
    filp_close(filep, NULL);
}
}

module_init(init);
module_exit(fini);
MODULE_LICENSE("GPL");
