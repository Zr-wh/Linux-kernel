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

struct hrtimer my_timer;
ktime_t kt;

tuple_info g_tuple_info;	//定义一个结构体变量,用于存储白名单信息,全局变量

//写死白名单信息
void fill_tx_tuple_info(void)
{
	g_tuple_info.sip = 0x01234567;
	g_tuple_info.dip = 0x98765432;
	g_tuple_info.sport = 0x3456;
	g_tuple_info.dport = 0x6789;
	g_tuple_info.protocol = 0x06;
	g_tuple_info.mac[0] = 0x01;
	g_tuple_info.mac[1] = 0x23;
	g_tuple_info.mac[2] = 0x45;
	g_tuple_info.mac[3] = 0x67;
	g_tuple_info.mac[4] = 0x89;
	g_tuple_info.mac[5] = 0xab;
}

struct sem_to_1156	//定义一个结构体,包含两个函数指针
{
	uint8_t (*tx_tuple_info)(uint8_t *,uint16_t);		//函数指针,发送白名单信息
};

struct sem_to_1156 *g_1156_ops = NULL;	//定义一个结构体指针

uint8_t register_sem_ops(struct sem_to_1156 *ops)	//注册函数
{
	if (ops == NULL){
		g_1156_ops = NULL;	//取消注册函数
		return 1;
	}
	g_1156_ops = ops;
	return 0;
}

EXPORT_SYMBOL(register_sem_ops); //导出函数,给aftr_insmod.ko调用



enum hrtimer_restart my_timer_callback(struct hrtimer *timer)	//定时器回调函数
{

	if(g_1156_ops!=NULL && g_1156_ops->tx_tuple_info!=NULL)
	{
		
			g_1156_ops->tx_tuple_info((uint8_t *)&g_tuple_info,sizeof(tuple_info));	//发送白名单信息
			printk(KERN_INFO "tx_tuple_info has been sent success.");
			//循环移位
			g_tuple_info.sip = (g_tuple_info.sip<<4) | (g_tuple_info.sip>>28);
			g_tuple_info.dip = (g_tuple_info.dip<<4) | (g_tuple_info.dip>>28);
			g_tuple_info.sport = (g_tuple_info.sport<<4) | (g_tuple_info.sport>>12);
			g_tuple_info.dport = (g_tuple_info.dport<<4) | (g_tuple_info.dport>>12);
			g_tuple_info.protocol = (g_tuple_info.protocol<<4) | (g_tuple_info.protocol>>4);
			g_tuple_info.mac[0] = (g_tuple_info.mac[0]<<4) | (g_tuple_info.mac[0]>>4);
			g_tuple_info.mac[1] = (g_tuple_info.mac[1]<<4) | (g_tuple_info.mac[1]>>4);
			g_tuple_info.mac[2] = (g_tuple_info.mac[2]<<4) | (g_tuple_info.mac[2]>>4);
			g_tuple_info.mac[3] = (g_tuple_info.mac[3]<<4) | (g_tuple_info.mac[3]>>4);
			g_tuple_info.mac[4] = (g_tuple_info.mac[4]<<4) | (g_tuple_info.mac[4]>>4);
			g_tuple_info.mac[5] = (g_tuple_info.mac[5]<<4) | (g_tuple_info.mac[5]>>4);
		

	}
    // 如果需要重复执行定时任务，可以重新启动定时器
    hrtimer_forward_now(timer, kt);
    return HRTIMER_RESTART;
}

static int pre_insmod_init(void)
{
	
	fill_tx_tuple_info();	//写死白名单信息
	hrtimer_init(&my_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	kt = ktime_set(0, timer_period); // 设置定时器的时间，1s
	my_timer.function = &my_timer_callback; // 设置定时器触发时调用的回调函数
	hrtimer_start(&my_timer, kt, HRTIMER_MODE_REL); // 启动定时器
	return 0;
}

static void pre_insmod_exit(void)
{
	printk("pre_insmod_exit\n");
	hrtimer_cancel(&my_timer); // 取消定时任务
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("author");
module_init(pre_insmod_init); // 必须！！
module_exit(pre_insmod_exit); // 必须！！

