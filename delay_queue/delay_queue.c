#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/completion.h>
#include <linux/kthread.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>



#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int
#define uint64_t unsigned long long
#define timer_period (1000*1000) // 1ms
#define DELAY_QUEUE_LEN 64//延迟发送队列长度，最大延迟时间是DELAY_QUEUE_LEN*时隙时间


uint32_t g_delay_time = 0;//每个时隙最多发送的包数
module_param(g_delay_time, uint, S_IRUGO|S_IWUSR);


typedef struct   _send_slot_recod
{
	uint16_t send_num;//当前时隙需要发送的包数
	struct sk_buff_head skb_list;//当前时隙需要发送的包链表

} send_slot_recod;

send_slot_recod g_delay_queue[DELAY_QUEUE_LEN];
uint16_t g_first_slot;//延迟队列当前处理的slot record的位置索引
struct hrtimer my_timer;
ktime_t kt;

static struct task_struct *task_1;
//定义一个完成变量
static struct completion my_completion;


//创建一个发送线程，等待完成变量唤醒
static int send_thread(void *data)
{
	struct sk_buff *skb;
	while(!kthread_should_stop())
	{
		wait_for_completion(&my_completion);
			while(!skb_queue_empty(&g_delay_queue[g_first_slot].skb_list))
			{
				skb = skb_dequeue(&g_delay_queue[g_first_slot].skb_list);
	
			}
			printk(KERN_INFO "Thread is sleeping for 1 seconds...\n");
		
	}
	return 0;
}

//初始化一个线程
void send_thread_init(void ){	
	task_1 = kthread_run(send_thread, NULL, "send_thread");
	if (IS_ERR(task_1)) {
		printk(KERN_INFO "create kthread failed!\n");
	}
	else {
		printk(KERN_INFO "create kthread ok!\n");
	}
}
    

enum hrtimer_restart my_timer_callback(struct hrtimer *timer)	//定时器回调函数
{

    // 如果需要重复执行定时任务，可以重新启动定时器
	g_first_slot=(g_first_slot+1)%DELAY_QUEUE_LEN;	
	//唤醒发送线程
	complete(&my_completion);

    hrtimer_forward_now(timer, kt);
    return HRTIMER_RESTART;
}

void time_init(void)
{
	hrtimer_init(&my_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	kt = ktime_set(0, timer_period*1000); // 设置定时器的时间，1s
	my_timer.function = &my_timer_callback; // 设置定时器触发时调用的回调函数
	hrtimer_start(&my_timer, kt, HRTIMER_MODE_REL); // 启动定时器
}

void delay_queu_init(void){
	uint16_t i=0;
	for (i=0;i<DELAY_QUEUE_LEN;i++)
	{
		g_delay_queue[i].send_num=0;
		skb_queue_head_init(&g_delay_queue[i].skb_list);
	}	
}

void delay_queue_add_skb(struct sk_buff *skb,uint16_t delay_time){
	uint16_t slot_index =0;
	slot_index =(g_first_slot+delay_time)%DELAY_QUEUE_LEN;
	skb_queue_tail(&g_delay_queue[slot_index].skb_list,skb);
	g_delay_queue[slot_index].send_num++;
}



static int fn_moudle_init(void)
{
	
	delay_queu_init();
	init_completion(&my_completion);
	send_thread_init();
	time_init();
	return 0;
}

static void fn_moudle_exit(void)
{
	printk("moudle_exit\n");
	hrtimer_cancel(&my_timer); // 取消定时任务
	if (task_1) {
          kthread_stop(task_1);
          task_1=NULL;
          //if (ret > 0)
              printk("<<<<<<<<, ret");
      }
	
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("author");
module_init(fn_moudle_init); // 必须！！
module_exit(fn_moudle_exit); // 必须！！
