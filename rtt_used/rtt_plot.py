import pyshark
import matplotlib.pyplot as plt

cap = pyshark.LiveCapture(interface='WLAN 2', tshark_path='D:\\Wireshark\\Wireshark.exe')
RTT_values = []

plt.ion()  # 开启交互模式

fig, ax = plt.subplots()
ax.set_xlabel('Packet Number')
ax.set_ylabel('RTT')
ax.set_title('Real-time RTT Plot')

line, = ax.plot([], [], label='RTT')
ax.legend()

packet_number = 0

for packet in cap.sniff_continuously():
    if 'TCP' in packet:
        if 'tcp.analysis.ack_rtt' in packet:
            RTT = float(packet.tcp.analysis_ack_rtt)
            RTT_values.append(RTT)

            line.set_data(range(len(RTT_values)), RTT_values)
            ax.relim()
            ax.autoscale_view()

            fig.canvas.draw()
            fig.canvas.flush_events()

            print(f'Packet Number: {packet_number} \t RTT: {RTT}')
            packet_number += 1


# import pyshark
# cap = pyshark.LiveCapture(interface='WLAN 2', tshark_path='D:/Wireshark/tshark.exe',bfp_filter=f'tcp port {8554}')
# i=0
# j=0
# rtts=0
# rttd=0
# a=0.125
# b=0.25
# ack_cur=0
# ack_pre=0
# for pkt in cap:
#     i+=1
#     if 'TCP' in pkt:
#         try:
#             if'analysis_ack_rtt' in dir(pkt.tcp):
#                 rtt=float(pkt.tcp.analysis_ack_rtt)
#                 if i==1:
#                     rtts=rtt
#                     rttd=rtt
#                     rto=rtts+4*rttd
#                     print(f'Packet Number: {i} \t RTT: {rtt}')
#                 else:
#                     rtts=rttd
#                     rttd=(1-a)*rtts+a*rtt
#                     rttd=(1-b)*rtts+b*abs(rttd-rtts)
#                 ack_cur=int(pkt.tcp.ack)
#                 rate=round((ack_cur-ack_pre)/rtt/(1024*1024),2)
#                 ack_pre=ack_cur
#                 print(f'source ip: {pkt.ip.src} \t destination ip: {pkt.ip.dst} \t source port: {pkt.tcp.srcport} \t destination port: {pkt.tcp.dstport}')
#                 print(f'Packet Number: {i} \t RTT: {rtt} \t Rate: {rate} Mbps')
#             elif str(pkt.tcp.flags_reset)=="1":
#                 print("Connection Reset")
#                 break
#         except AttributeError as e:
#             print('ack序号:%d'%i,e)