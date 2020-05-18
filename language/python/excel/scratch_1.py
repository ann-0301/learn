# -*- coding: utf-8 -*-

import xlsxwriter
import time

price_all = {}
data_kind = []
data_all = {}
line_total = 0
price_filename = 'price.txt'
before_date_filename = 'before_date.txt'
with open(price_filename, 'r', encoding='UTF-8') as f:
    for line in f:
        date_temp = line.split(":", 1)
        price_all[date_temp[0]] = int(date_temp[1])
print(price_all)

def dump_kinds():
    j = 0
    print("选择种类:")
    for kind in price_all:
        data_kind.append(kind)
        j = j + 1
        print(str(j) + ":" + kind)

date_before = ''
def write_data():
    global data_all, date_before
    while True:
        print("还继续录入吗？不继续的话请按 \'q\'，继续请按任意键")
        want = input()
        if want == 'q': break

        print("-----------------------------------------------------")
        if date_before == '':
            print("输入时间:")
        else:
            print("输入时间，如果时间和之前相同，请输入 \'s\'，否则直接输入时间:")

        date = input()
        if date == 's':
            date = date_before
        date_before = date
        print("-----------------------------------------------------")
        dump_kinds()
        chose_num = input()
        print("-----------------------------------------------------")
        print("输入数据:")
        input_num = input()
        print("-----------------------------------------------------")
        print("请确认输入的数据是否准确：Y or N")
        print ("\n\n\n\n")
        print("******************************************************")
        print("|时间：" + date + "  | 种类：" + data_kind[int(chose_num) - 1] + " | 数量：" + input_num + "|")
        print("******************************************************")
        print ("\n\n\n\n")
        sure_flag = input()

        data_num_temp = 0
        if sure_flag == 'y' or sure_flag == 'Y':
            if date in data_all:
                if chose_num in data_all[date]:
                    data_num_temp = data_all[date][chose_num]
                data_info = {chose_num: eval(input_num) + data_num_temp}
                data_all[date].update(data_info)
            else:
                data_all[date] = {chose_num: eval(input_num)}
        else:
            print ("\n")
            print('######################################################')
            print('！！！！！WARNING:刚才的数据没有保存，请注意！！！！！')
            print('######################################################')
            print ("\n")

        print(data_all)
        with open(before_date_filename, 'w', encoding='UTF-8') as f:
            f.write(str(data_all))
            f.close()

def continue_check():
    try:
        f = open(before_date_filename)
        f.close()
    except IOError:
        return
    print("是否继续之前的数据进行操作？Y or N")
    continue_flag = input()
    if continue_flag == 'y' or continue_flag == 'Y':
        global data_all, line_total
        with open(before_date_filename, 'r', encoding='UTF-8') as f:
            lines = f.readlines()
            data_all = eval(lines[0])
            f.close()
    elif continue_flag == 'n' or continue_flag == 'N':
        with open(before_date_filename, 'w', encoding='UTF-8') as f:
            f.truncate()
            f.close()


def save_excle():
    date_now = time.strftime('%Y_%m_%d_%H_%M_%S', time.localtime(time.time()))
    file_name = "Demo" + date_now + ".xlsx"
    workbook = xlsxwriter.Workbook(file_name)
    worksheet = workbook.add_worksheet()

    worksheet.set_column('A:A', 20)
    data_sum = {}

    j = 0
    worksheet.write(0, 0, "时间")
    for kind in price_all:
        j = j + 1
        worksheet.write(0, j, kind)

    line_num = 0
    for date in sorted(data_all):
        line_num = line_num + 1
        worksheet.write(line_num, 0, date)
        for colnum in data_all[date]:
            if colnum in data_sum:
                data_sum[colnum] = data_sum[colnum] + float(data_all[date][colnum])
            else:
                data_sum[colnum] = float(data_all[date][colnum])
            worksheet.write_number(line_num, int(colnum), float(data_all[date][colnum]))
    if line_num > 0:
        line_num = line_num + 1
        worksheet.write(line_num, 0, "总和")
        for colnum in data_sum:
            worksheet.write_number(line_num, int(colnum), float(data_sum[colnum]))
    workbook.close()  # 关闭Excel文件

def main():
    continue_check()
    write_data()
    save_excle()
    exit(0)

main()


# bold = workbook.add_format({'blod': True})

# 行列表示法的单元格下标以0作为起始值，如‘3,0’等价于‘A3’
# worksheet.write(2, 1, 123)              # 使用列行表示法写入数字‘123’
# worksheet.write(3, 2, 456)              # 使用列行表示法写入数字‘456’
# worksheet.write(4, 0, '=SUM(A3:A4)')    # 求A3:A4的和，并写入‘4,0’，即‘A5’

# worksheet.insert_image('A5', 'XLSX.png')        # 在A5单元格插入图片

