#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>

#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int
#define uint64_t unsigned long long
#define timer_period (1000*1000*1000) // 1s


//定义一个白名单需要保障的信息
typedef struct _tuple_info{	
	uint32_t sip;			//源ip
	uint32_t dip;			//目的ip
	uint16_t sport;			//源端口	
	uint16_t dport;			//目的端口
	uint8_t protocol;		//协议
	uint8_t mac[6];			//mac地址
}tuple_info;

tuple_info g_1156_tuple_info;	//1156里的白名单信息，全局变量，用于存储白名单信息用于截获过滤

uint8_t app_flow_sem_to_1156(uint8_t *buf,uint16_t len);	//函数声明
struct hrtimer my_timer;	//定义一个定时器
ktime_t kt;

struct sem_to_1156	//定义一个结构体,包含两个函数指针
{
	uint8_t (*tx_tuple_info)(uint8_t *,uint16_t);		//函数指针,白名单信息
};

extern uint8_t register_sem_ops(struct sem_to_1156 *ops); //声明导出函数

static struct sem_to_1156 sem_to_1156_ops = {	//为函数指针赋值
	.tx_tuple_info = app_flow_sem_to_1156,
};

uint8_t app_flow_sem_to_1156(uint8_t *buf,uint16_t len)
{
	memcpy(&g_1156_tuple_info,buf,len);	//此时g_tuple_info中存储的就是白名单信息，从pre_insmod传递过来
	return 0;
}


enum hrtimer_restart my_timer_callback(struct hrtimer *timer)	//定时器回调函数
{
    printk(KERN_INFO "tx_tuple_info has been received by 1156\n");

	if(register_sem_ops(&sem_to_1156_ops)==0)	//注册函数，将函数指针传递给1156
	{
		//打印白名单信息
		printk("sip:0x%x\n",g_1156_tuple_info.sip);
		printk("dip:0x%x\n",g_1156_tuple_info.dip);
		printk("sport:0x%x\n",g_1156_tuple_info.sport);
		printk("dport:0x%x\n",g_1156_tuple_info.dport);
		printk("protocol:0x%x\n",g_1156_tuple_info.protocol);
		printk("mac:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x\n",g_1156_tuple_info.mac[0],g_1156_tuple_info.mac[1],g_1156_tuple_info.mac[2],\
		g_1156_tuple_info.mac[3],g_1156_tuple_info.mac[4],g_1156_tuple_info.mac[5]);
	}
    // 如果需要重复执行定时任务，可以重新启动定时器
    hrtimer_forward_now(timer, kt);
    return HRTIMER_RESTART;
}

static int __init after_insmod_init(void)
{
	printk("after_insmod_init\n");
	hrtimer_init(&my_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	kt = ktime_set(0, timer_period); // 设置定时器的时间，1s
	my_timer.function = &my_timer_callback; // 设置定时器触发时调用的回调函数
	hrtimer_start(&my_timer, kt, HRTIMER_MODE_REL); // 启动定时器
	return 0;
}
static void __exit after_insmod_exit(void)
{
	printk("after_insmod_exit\n");
	hrtimer_cancel(&my_timer); // 取消定时任务
	register_sem_ops(NULL);	//取消注册函数
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("author");
module_init(after_insmod_init); // 必须！！
module_exit(after_insmod_exit); // 必须！！

