# -*- coding: utf-8 -*-

import xlsxwriter
import time

price_all = {}
data_kind = []
data_all  = {}
with open('price.txt', 'r', encoding='UTF-8') as f:
    for line in f:
        date_temp=line.split(":", 1)
        price_all[date_temp[0]]=int(date_temp[1])
print(price_all)

date_now = time.strftime('%Y_%m_%d_%H_%M_%S',time.localtime(time.time()))
file_name = "Demo" + date_now + ".xlsx"
workbook = xlsxwriter.Workbook(file_name)    # 创建一个名为‘Demo1.xlsx’的工作表
worksheet = workbook.add_worksheet()            # 创建一个工作表对象

worksheet.set_column('A:A', 20)                 # 设定第一列（A）的宽度为20px


def write_kinds():
    j = 0
    worksheet.write(0, 0, "时间")
    for kind in price_all:
        j = j +1
        worksheet.write(0,j,kind)

def dump_kinds():
    j = 0
    print("选择种类:")
    for kind in price_all:
        data_kind.append(kind)
        j = j + 1
        print(str(j)+":"+kind)

def write_data():
    i = 1
    while True:
        write_flag_date = 0
        print("还继续录入吗？不继续的话请按 \'q\'，继续请按任意键")
        want = input()
        if want == 'q': break

        if i > 1:
            print("输入时间，如果时间和之前相同，请输入 \'s\'，否则直接输入时间:")
        else:
            print("输入时间:")
        date = input()
        if date == 's' or date in data_all.keys():
            i = i - 1
        else:
            write_flag_date = 1

        dump_kinds()
        chose_num = input()
        print("输入数据:")
        input_num = input()

        print("请确认输入的数据是否准确：Y or N")
        print("---------------------------------------------------------------------")
        print("|时间：" + date + "  | 种类：" + data_kind[int(chose_num) - 1] + " | 数量：" + input_num + "|")
        print("---------------------------------------------------------------------")
        sure_flag = input()

        if sure_flag == 'y' or sure_flag == 'Y':
            if write_flag_date == 1:
                worksheet.write(i, 0, date)
                line_info = {'line_num':i}
                data_all[date] = line_info
                data_info = {chose_num:eval(input_num)}
                data_all[date].update(data_info)
            else:
                if chose_num in data_all[date]:
                    num_temp = data_all[date][chose_num]
                    data_info = {chose_num:eval(input_num)+num_temp}
                    data_all[date].update(data_info)
                else:
                    data_info = {chose_num:eval(input_num)}
                    data_all[date].update(data_info)
            print(data_all)
            worksheet.write_number(data_all[date]['line_num'], int(chose_num), data_all[date][chose_num])
            i = i + 1

def main():
    write_kinds()
    write_data()
    workbook.close()  # 关闭Excel文件
    exit(0)

main()




# bold = workbook.add_format({'blod': True})




# 行列表示法的单元格下标以0作为起始值，如‘3,0’等价于‘A3’
#worksheet.write(2, 1, 123)              # 使用列行表示法写入数字‘123’
#worksheet.write(3, 2, 456)              # 使用列行表示法写入数字‘456’
#worksheet.write(4, 0, '=SUM(A3:A4)')    # 求A3:A4的和，并写入‘4,0’，即‘A5’

#worksheet.insert_image('A5', 'XLSX.png')        # 在A5单元格插入图片
