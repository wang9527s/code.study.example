

## smb server 源码编译使用

[下载](https://download.samba.org/pub/samba/)

### wsl + Ubuntu 24.04

**build.sh**

编译samba

**update_and_run.sh**

运行samba服务

1. 脚本运行，输出pid信息，代表程序运行成功
2. 在1的基础上，创建samba用户 (新启一终端中运行，即可)；然后重新运行update_and_run。

3. client则可以正常访问server

### mac (server_mac文件夹)

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

### smbclient访问server

```bash
smbclient //192.168.31.46/home -U wb%1 -c "ls"
smbclient //192.168.31.46/home -U wb%1 -c "get .viminfo"

smbclient //192.168.31.46/download -U wb%1 -c "put 55555 55"
smbclient //192.168.31.46/download -U wb%1 -c "get 55 55.d"

smbclient //192.168.31.46/download -U wb%1 -c "put 55555 www/66"
smbclient //192.168.31.46/download -U wb%1 -c "cd www;rm 66"
smbclient //192.168.31.46/download -U wb%1 -c "cd www;ls"
smbclient //192.168.31.46/download -U wb%1 -c "cd www;rm 66"
smbclient //192.168.31.46/download -U wb%1 -c "cd www;ls"
```