# -*- coding: utf-8 -*-

import os
import paramiko
import time
from threading import Thread, Lock
from scp import SCPClient
import ConfigParser

iplist_filename = "ip.txt"
hostname = '192.168.20.121'
#username = 'shawn'
#password = '1'
upload_flag = 0
download_flag = 0
remote_path = "/home/shawn"

cf = ConfigParser.ConfigParser()
cf.read("pi_py.conf")
db_host = cf.get("db", "db_hostip")
print(db_host)

def connect_clients(hostname, username, password,cmd):
    try:
        date_now = time.strftime('%Y_%m_%d_%H_%M_%S', time.localtime(time.time()))
        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        client.connect(hostname=hostname, username=username, password=password)

        if upload_flag != 0 or download_flag != 0:
            scpclient = SCPClient(client.get_transport(),socket_timeout=15.0)
            print(cmd)
            if upload_flag == 1:
                scpclient.put(cmd, remote_path)
            elif download_flag == 1:
                scpclient.get(cmd, os.path.abspath("."))
        else:
            shell = client.invoke_shell()
            shell.sendall(cmd+"\n")
            shell.sendall("exit\n")

    except Exception as e:
        print("[%s] %s target failed, the reason is %s" % (date_now, hostname, str(e)))
    else:
        print("[%s] %s target success" % (date_now, hostname))
    finally:
        client.close()

def main():
    global upload_flag,download_flag
    return
    while True:
        select = input("what do you want to do?\n1:upload file\n2.download file\n3.other command\n4.exit\n")
        upload_flag = 0
        download_flag = 0
        cmd = ""
        if select == "1":
            upload_flag = 1
            file_name = input("file name:\n")
            cmd = os.path.abspath(file_name).replace("\\", "\\\\")
            print(cmd)
        elif select == "2":
            download_flag = 1
            file_name = input("file name:\n")
            cmd = remote_path+"/"+file_name
        elif select == "3":
            cmd = input("input the command:\n")
        elif select == "4":
            exit(0)
        else:
            continue

        with open(iplist_filename, 'r', encoding='UTF-8') as f:
            for line in f:
                line_temp = line.split(":", 1)
                ipaddress = line_temp[1].rstrip("\n")
                if ipaddress == "192.168.101.26":
                    username = "pi"
                    password = "onuware"
                else:
                    username = "shawn"
                    password = "1"
                connect_client = Thread(target=connect_clients, args=(ipaddress, username, password,cmd))
                connect_client.start()

if __name__ == '__main__':
    main()

