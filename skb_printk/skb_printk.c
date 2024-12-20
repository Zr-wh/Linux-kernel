#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/skbuff.h>

typedef unsigned char uint8;
extern void print_skb_info(struct sk_buff *skb);
int generate_skb(void) {
	uint8   data_hdr[6]={0x00,0x01,0x02,0x03,0x04,0x05};
	uint8		*data_body;
	uint8		size;
	size=20;
	
	struct sk_buff *skb = NULL;
	data_body = kmalloc(20, GFP_KERNEL);
	memset(data_body,0x11,size); 

	skb = alloc_skb(200, GFP_KERNEL); 
    //skb_reserve(skb,0);
    // 放入MAC帧头部信息 
	memcpy(skb_tail_pointer(skb),data_hdr, sizeof(data_hdr));
    	skb_put(skb, 6);  
	memcpy(skb_tail_pointer(skb),data_body, size);
	skb_put(skb, size); 
	print_skb_info(skb);

	kfree(data_body);
	kfree_skb(skb);
    printk("generatre skbuffer end.\n");
    return 0;
}


  static int  mod_init(void)
  {

    printk("=====mod set up===");
    generate_skb();
    printk("======end");
    return 0;
    

  }

  static void  mod_exit(void)
  {
    printk("mod_exit ()");
    
 }
MODULE_LICENSE("GPL");
module_init(mod_init);
module_exit(mod_exit);

 void print_skb_info(struct sk_buff *skb)
{
	printk("----------------------------------Start print skb  info---------------------\n");
	// 空指针保护
	if (skb == NULL)
	{
		printk("ERROR: skb memory allocated failed, skb poniter is NULL, print skb info failed. \n");
		return;
	}

	// 判断线性或非线性, 当前仅能够打印线性skb (因为在probe函数当中仅使用线性区)
	if (skb->data_len > 0)
	{
		printk("ERROR: skb  data_len paramater is non-zero, means non-linear skb, print skb info failed. \n");
		return;
	}

	// 判断结束，开始打印skb mpdu
	uint8 msdu_len = skb->len; // mpdu长度
	printk("--------------------INFO: length of MSDU is: %d --------------------", msdu_len);
	printk("skb->head=%pK",skb->head);
	printk("skb->data=%pK",skb->data);
	printk("skb->tail=%pK",skb_tail_pointer(skb));
	printk("skb->tail=%d",skb->tail);
	printk("skb->end=%pK",skb_end_pointer(skb));
	printk("skb->end=%d",skb->end);
	printk("skb_headroom=%d",skb_headroom(skb));
	printk("skb_tailroom=%d",skb_tailroom(skb));
	printk("alloc_len=%d",skb_end_pointer(skb)-skb->head);
	// 进行skb->data的打印
	printk("-----------------INFO: skb data info print begin------------------ \n");
	uint8 i = 0;

	for (i = 0; i < skb->len; i++){
 
		printk(KERN_CONT "%02x ", *(skb->data + i));
 
		if(3 == i % 4){
			printk("\n");
		}
	}
	printk("\n");
	printk("--------------INFO: skb data info print end-------------------\n");
	return;
}

