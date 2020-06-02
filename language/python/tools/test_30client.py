# -*- coding: utf-8 -*-

import os
import paramiko
import time
import xlsxwriter
from threading import Thread, Lock

iplist_filename = "ip.txt"
data_temp_filename = "data_save_temp.txt"
hostname = '192.168.20.201'
#username = 'shawn'
#password = '1'
data_all = {}
lock = Lock()
before_output_file_name = ''
totle_ip_num = 0
totle_output_num = 0


def output_file():
    global totle_output_num, before_output_file_name
    totle_output_num = totle_output_num + 1
    if totle_output_num < totle_ip_num:
        return

    totle_output_num = 0
    date_now = time.strftime('%Y_%m_%d_%H_%M_%S', time.localtime(time.time()))
    output_file_name = "output" + date_now + ".xlsx"
    workbook = xlsxwriter.Workbook(output_file_name)
    worksheet = workbook.add_worksheet()

    my_format_error = workbook.add_format({
        'bold': 1,
        'fg_color': 'red',
        'align': 'center',
    })
    my_format_common = workbook.add_format({
        'align': 'center',
    })

    worksheet.set_column('A:B', 20)
    worksheet.set_column('C:D', 25)
    worksheet.set_column('E:E', 50)
    worksheet.write(0, 0, "IP Address", my_format_common)
    worksheet.write(0, 1, "Date", my_format_common)
    worksheet.write(0, 2, "Download Speed(Mbit/s)", my_format_common)
    worksheet.write(0, 3, "Upload Speed(Mbit/s)", my_format_common)
    worksheet.write(0, 4, "Other", my_format_common)

    line_num = 1

    for ip in sorted(data_all):
        for date in data_all[ip]:
            worksheet.write(line_num, 0, ip)
            worksheet.write(line_num, 1, date)
            if data_all[ip][date]["DownloadSpeed"] <= 0:
                worksheet.write_number(line_num, 2, data_all[ip][date]["DownloadSpeed"], my_format_error)
            else:
                worksheet.write_number(line_num, 2, data_all[ip][date]["DownloadSpeed"], my_format_common)

            if data_all[ip][date]["UploadSpeed"] <= 0:
                worksheet.write_number(line_num, 3, data_all[ip][date]["UploadSpeed"], my_format_error)
            else:
                worksheet.write_number(line_num, 3, data_all[ip][date]["UploadSpeed"], my_format_common)
            worksheet.write(line_num, 4, data_all[ip][date]["Error"])
            line_num = line_num + 1

    workbook.close()  # 关闭Excel文件
    if before_output_file_name != '':
        try:
            os.remove(before_output_file_name)
        except IOError:
            print(before_output_file_name + " maybe in used")
    before_output_file_name = output_file_name



def connect_clients(hostname, username, password):
    try:
        global data_all
        data_info = {}
        date_now = time.strftime('%Y/%m/%d %H:%M:%S', time.localtime(time.time()))
        download_speed = 0
        upload_speed = 0
        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        client.connect(hostname=hostname, username=username, password=password)
        shell = client.invoke_shell()
        shell.sendall("python speedtest.py\n")
        shell.sendall("exit\n")
        while True:
            data = shell.recv(2048).decode()
            if not data:
                break

            date_line = data.split("\n")
            for data_temp in date_line:
                if 'Download:' in data_temp:
                    download_speed = float(data_temp.split(":", 1)[1].split(" ", 2)[1])
                if 'Upload:' in data_temp:
                    upload_speed = float(data_temp.split(":", 1)[1].split(" ", 2)[1])

    except Exception as e:
        print("[%s] %s target failed, the reason is %s" % (date_now, hostname, str(e)))
        lock.acquire()
        if hostname in data_all:
            data_all[hostname].update(
                {date_now: {"DownloadSpeed": download_speed, "UploadSpeed": upload_speed, "Error": str(e)}})
        else:
            data_all[hostname] = {
                date_now: {"DownloadSpeed": download_speed, "UploadSpeed": upload_speed, "Error": str(e)}}
        lock.release()
    else:
        lock.acquire()
        if hostname in data_all:
            data_all[hostname].update(
                {date_now: {"DownloadSpeed": download_speed, "UploadSpeed": upload_speed, "Error": ""}})
        else:
            data_all[hostname] = {date_now: {"DownloadSpeed": download_speed, "UploadSpeed": upload_speed, "Error": ""}}
        lock.release()
        print("[%s] %s DownloadSpeed:%.2f, UploadSpeed:%.2f" % (date_now, hostname,download_speed,upload_speed))
    finally:
        client.close()
        lock.acquire()
        output_file()
        lock.release()



def main():
    global totle_ip_num
    interval = 180
    username = ""
    password = ""
    while True:
        with open(iplist_filename, 'r', encoding='UTF-8') as f:
            totle_ip_num = 0
            for line in f:
                totle_ip_num = totle_ip_num+1
                line_temp = line.split(":", 1)
                ipaddress = line_temp[1].rstrip("\n")
                if ipaddress == "192.168.101.21":
                    username = "shawn"
                    password = "1"
                else:
                    username = "pi"
                    password = "onuware"
                connect_client = Thread(target=connect_clients, args=(ipaddress, username, password))
                connect_client.start()
        time.sleep(interval)


def show_data():
    print("IP Address     |  Date                | Download Speed(Mbit/s)    | Upload Speed(Mbit/s) | Other ")
    for ip in sorted(data_all):
        for date in data_all[ip]:
            print(ip + "   |  " + date + "   |  " \
                  + str(data_all[ip][date]["DownloadSpeed"]) + "           |          " \
                  + str(data_all[ip][date]["UploadSpeed"]) + "        |        " \
                  + data_all[ip][date]["Error"])


if __name__ == '__main__':
    main()

