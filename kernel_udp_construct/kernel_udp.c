#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/inet.h>

#define IF_NAME "eth0"
#define DIP "192.168.1.1"
#define SIP "192.168.1.2"
#define DPORT 319
#define SPORT 319
#define ETH_ALEN 6
#define ETH_P_IP 0x0800
#define IPPROTO_UDP 17
#define IPPROTO_TCP 6

u8 dst_mac[ETH_ALEN] = {0x10, 0x7b, 0x44, 0x4c, 0x4d, 0x4e}; /* dst MAC */
u8 src_mac[ETH_ALEN] = {0x10, 0x7b, 0x44, 0x4c, 0x4d, 0x4f}; /* src MAC */


//ip头部校验和计算
// ip_send_check(iph);
// {   iph->check = 0；
//     iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
// }

//icmp头部校验和计算，icmp头部校验和是icmp头部加icmp数据部分
unsigned short icmp_calc_checksum(char * icmp_packet, int size)
{
 unsigned short * sum = (unsigned short *)icmp_packet;
 unsigned int checksum = 0;
 while (size > 1) {
  checksum += ntohs(*sum++);
  size -= sizeof(unsigned short);
 }
 if (size) {
  *sum = *((unsigned char*)sum);
  checksum += ((*sum << 8) & 0xFF00);
 }
 
 checksum = (checksum >> 16) + (checksum & 0xffff);
 checksum += checksum >> 16;
 
 return (unsigned short)(~checksum);
}

struct socket *sock;

static int k_sendmsg(struct socket *sock, void *buff, size_t len,
                     unsigned flags, struct sockaddr *addr, int addr_len)
{
    struct kvec vec;
    struct msghdr msg;

    vec.iov_base = buff;
    vec.iov_len = len;

    memset(&msg, 0x00, sizeof(msg));
    /* msg.msg_name = addr;  */
    /* msg.msg_namelen = addr_len;  */
    /* msg.msg_flags = flags | MSG_DONTWAIT;  */

    return kernel_sendmsg(sock, &msg, &vec, 1, len);
}

static void send_by_sock()
{
    struct sockaddr_in addr;
    u8 buf[15] = {"hello world"};
    int ret;

    sock_create_kern(PF_INET, SOCK_DGRAM, 0, &sock);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(319);
    addr.sin_addr.s_addr = in_aton("192.168.1.36");

    kernel_bind(sock, (struct sockaddr *)&addr, sizeof(addr));

    ret = k_sendmsg(sock, buf, sizeof(buf), 0, (struct sockaddr *)&addr, sizeof(addr));
    if (ret)
    {
        printk("kernel_msg: %d\n", ret);
    }
}

static void sock_init()
{
    struct ifreq ifr;

    sock_create_kern(PF_INET, SOCK_DGRAM, 0, &sock);
    strcpy(ifr.ifr_ifrn.ifrn_name, IF_NAME);
    kernel_sock_ioctl(sock, SIOCSIFNAME, (unsigned long)&ifr);
}

static void send_by_skb()
{
    struct net_device *netdev;
    struct net *net;
    struct sk_buff *skb;
    struct ethhdr *eth_header;
    struct iphdr *ip_header;
    struct udphdr *udp_header;
    __be32 dip = in_aton(DIP);
    __be32 sip = in_aton(SIP);
    u8 buf[16] = {"hello world"};
    u16 data_len = sizeof(buf);
    u16 expand_len = 16; /* for skb align */
    u8 *pdata = NULL;
    u32 skb_len;
    // u8 dst_mac[ETH_ALEN] = {DST_MAC_00, DST_MAC_01, DST_MAC_02, DST_MAC_03, DST_MAC_04, DST_MAC_05}; /* dst MAC */
    // u8 src_mac[ETH_ALEN] = {SRC_MAC_00, SRC_MAC_01, SRC_MAC_02, SRC_MAC_03, SRC_MAC_04, SRC_MAC_05}; /* src MAC */

    /* construct skb */
    sock_init();
    net = sock_net((const struct sock *)sock->sk);
    netdev = dev_get_by_name(net, IF_NAME);

    skb_len = LL_RESERVED_SPACE(netdev) + sizeof(struct iphdr) + sizeof(struct udphdr) + data_len + expand_len;
    printk("iphdr: %d\n", sizeof(struct iphdr));
    printk("udphdr: %d\n", sizeof(struct udphdr));
    printk("data_len: %d\n", data_len);
    printk("skb_len: %d\n", skb_len);

    skb = dev_alloc_skb(skb_len);
    if (!skb)
    {
        return;
    }

    skb_reserve(skb, LL_RESERVED_SPACE(netdev));
    skb->dev = netdev;
    skb->pkt_type = PACKET_OTHERHOST;
    skb->protocol = htons(ETH_P_IP);
    skb->ip_summed = CHECKSUM_NONE;
    skb->priority = 0;

    /* construct ethernet header in skb */
    eth_header = (struct ethhdr *)skb_put(skb, sizeof(struct ethhdr));
    memcpy(eth_header->h_dest, dst_mac, ETH_ALEN);
    memcpy(eth_header->h_source, src_mac, ETH_ALEN);
    eth_header->h_proto = htons(ETH_P_IP);

    /* construct ip header in skb */
    skb_set_network_header(skb, sizeof(struct ethhdr));
    skb_put(skb, sizeof(struct iphdr));
    ip_header = ip_hdr(skb);
    ip_header->version = 4;
    ip_header->ihl = sizeof(struct iphdr) >> 2;
    ip_header->frag_off = 0;
    ip_header->protocol = IPPROTO_UDP;
    ip_header->tos = 0;
    ip_header->daddr = dip;
    ip_header->saddr = sip;
    ip_header->ttl = 0x40;
    ip_header->tot_len = htons(skb->len);
    ip_header->check = 0;

    /* construct udp header in skb */
    skb_set_transport_header(skb, sizeof(struct ethhdr) + sizeof(struct iphdr));
    skb_put(skb, sizeof(struct udphdr));
    udp_header = udp_hdr(skb);
    udp_header->source = htons(SPORT);
    udp_header->dest = htons(DPORT);

    /* insert data in skb */
    pdata = skb_put(skb, data_len);
    if (pdata)
    {
        memcpy(pdata, buf, data_len);
    }

    /* caculate checksum */
    udp_header->check = csum_tcpudp_magic(sip, dip, skb->len - ip_header->ihl * 4, IPPROTO_UDP, skb->csum);
    skb->csum = skb_checksum(skb, ip_header->ihl * 4, skb->len - ip_header->ihl * 4, 0);

    /* send packet */
    if (dev_queue_xmit(skb) < 0)
    {
        dev_put(netdev);
        kfree_skb(skb);
        printk("send packet by skb failed.\n");
        return;
    }
    printk("send packet by skb success.\n");
}

static int __init testmod_init(void)
{

    /* send_by_sock(); */
    send_by_skb();

    printk("testmod kernel module load!\n");

    return 0;
}

static void __exit testmod_exit(void)
{
    sock_release(sock);

    printk("testmod kernel module removed!\n");
}

module_init(testmod_init);
module_exit(testmod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lion3875 <>");
MODULE_DESCRIPTION("A packet generation & send kernel module");