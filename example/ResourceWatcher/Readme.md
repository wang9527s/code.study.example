
> 项目功能预览

  主要使用Python的pyutil包和nvidia-smi进行资源监控。    
  cpu、gpu以及内存资源。(很简略、不太准确)  

---

工具调研

  目的：监测每个进程对cpu、内存和gpu资源的使用情况。
	
2023.08.19

  tasklist可以获取内存使用率。
  使用wmic可以获取cpu和内存的使用率，但是cpu的使用率看起来和任务管理器中不一致。
  
```bash
# cpu
wmic path Win32_PerfRawData_PerfProc_Process get Name, PercentProcessorTime

# 内存
wmic path Win32_PerfRawData_PerfProc_Process get Name, WorkingSetPrivate
```

2023.08.22

  无明显进展，内存使用率还算准确，cpu和gpu的使用一言难尽。  
  期间尝试过各种方法
  
2023.08.24

  使用Python的pynvml获取gpu的总体信息，nvidia-smi命令获取使用gpu资源的线程id
  psutil获取cpu和内存总体使用情况 （每个进程的cpu资源也获取的有问题，基本都是0）
  
+ 备注：
  wmic也可以获取整体cpu的使用情况