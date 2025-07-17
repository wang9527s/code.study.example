

## smb server 源码编译使用

[下载](https://download.samba.org/pub/samba/)，mac上使用的是**4.15.9**，更高版本的兼容性更差。

### 1、wsl + Ubuntu 24.04

**build_*.sh**

编译samba

**create_user.sh**

创建samba 用户

**restart.sh**

脚本运行，启动samba服务，输出pid信息，代表程序运行成功。  
然后client则可以正常访问server

### 2、mac (server_mac文件夹)

  4.19.9 可以编译通过，但是功能异常（文件删除等操作异常）  
  4.15.9 可正常编译  
  4.15.13可正常编译

**系统自带的samba服务**

```bash
# stop
if sudo launchctl list | grep -q "$com.apple.smbd"; then
    echo "stop sys smb"
    sudo launchctl bootout system /System/Library/LaunchDaemons/com.apple.smbd.plist
else
    echo "sys smb not start"
fi

# start
sudo launchctl load -w /System/Library/LaunchDaemons/com.apple.smbd.plist  
```

### 3、smbclient的使用

```bash
#!/bin/bash
set -e

HOST="192.168.31.5"

smbclient "//$HOST/download" -U wb%1 -c "put 55555 55"
smbclient "//$HOST/download" -U wb%1 -c "get 55 55.d"
smbclient "//$HOST/download" -U wb%1 -c "mkdir dtest"

smbclient "//$HOST/download" -U wb%1 -c "ls"
smbclient "//$HOST/download" -U wb%1 -c "rmdir dtest"
smbclient "//$HOST/download" -U wb%1 -c "del 55"

# 匿名访问
smbclient "//$HOST/download" -N -c "mkdir hh; cd hhh; ls"
```

参数说明

```bash
-N                    游客（匿名）模式
-U name%password      正常登录

-c "command"          命令
    ls                    获取文件列表
    cd                    进入文件夹
    get                   下载文件
    put                   上传文件
    del                   删除文件
    rmdir                 删除文件夹
    mkdir                 创建文件夹
```

### 4、smb.conf的使用

**[global]** 配置

```bash
[global]
   log file = /Users/mi/samba/tools/samba.log
```

**共享文件夹配置**

```bash
# 配合创建的samba用户一起使用
[download]
   path = /Users/mi/Downloads
   guest ok = yes
   read only = no
   browsable = yes
```
```bash
# 可以匿名访问，但需要用户存在
[download]
   path = /Users/mi/Downloads
   guest ok = yes
   public = yes
   writable = yes    
   create mask = 0777
   directory mask = 0777
   force user = mi 
```
```bash
# 可以匿名访问，不需要sudo创建用户
# 但要配合 chmod 777 /Users/mi/Downloads 使用
[download]
   path = /Users/mi/Downloads
   guest ok = yes
   public = yes
   writable = yes    
   create mask = 0777
   directory mask = 0777
```