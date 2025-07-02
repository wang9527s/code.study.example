

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