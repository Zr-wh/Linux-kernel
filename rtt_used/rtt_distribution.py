import csv

# 读取 CSV 文件
filename = 'C:\\Users\\admin\\Desktop\\WLAN 2.csv'  # 请将路径替换为实际文件路径
print(f"文件路径：{filename}")

rtt_data = []

try:
    with open(filename, 'r', encoding='utf-8') as file:
        csv_reader = csv.reader(file)
        next(csv_reader)  # 跳过第一行数据
        for row in csv_reader:
            try:
                rtt_value = 1000 * float(row[1])  # 尝试将值转换为浮点数
                rtt_data.append(rtt_value)
            except ValueError:
                print(f"无法转换为浮点数的值：{row[1]}")
except Exception as e:
    print(f"读取文件时出错：{e}")

# 统计数据区间分布
interval_count = {'0-50': 0, '51-100': 0, '101-150': 0, '151-200': 0, '200以上': 0}

total_data_points = len(rtt_data)
# 打印总数
print(f"总数据点数量: {total_data_points}")
for rtt in rtt_data:
    if rtt <= 50:
        interval_count['0-50'] += 1
    elif rtt <= 100:
        interval_count['51-100'] += 1
    elif rtt <= 150:
        interval_count['101-150'] += 1
    elif rtt <= 200:
        interval_count['151-200'] += 1
    else:
        interval_count['200以上'] += 1

# 计算百分比
percentage_interval_count = {interval: count / total_data_points * 100 for interval, count in interval_count.items()}

# 打印每个区间的范围、数量和百分比
print("RTT 数据区间分布统计：")
for interval, count in interval_count.items():
    percentage = count / total_data_points * 100
    print(f'{interval}: {count} ,{percentage:.2f}%')