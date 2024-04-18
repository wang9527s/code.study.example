import pynvml
import psutil
import json
import subprocess
import re
import time
import csv

def convert_bytes_to_megabytes(bytes):
    megabytes = bytes / 1024 / 1024
    return f"{megabytes:.2f}M"

def update_tasklist():
    result = subprocess.run(['tasklist', '/FO', 'CSV', '/NH'], capture_output=True, text=True)
    output = result.stdout

    for line in output.splitlines():
        fields = line.strip('"').split('","')
        process_name = fields[0]
        process_id = str(fields[1])
        memory_usage = fields[4]

        update_process_data(process_id, "name", process_name)
        update_process_data(process_id, "memory_usage", memory_usage)

def update_process_data(pid, key, value):
    if pid not in processes_info.keys():
        processes_info[pid] = {
            "name": "",
            "memory_usage": "",
            "memory_util": "",
            "workingSet": ""
        }
        
    processes_info[pid][key] = value

def update_process_info():
    command = 'powershell Get-Process | Select-Object Name, Id, CPU, WorkingSet | ConvertTo-Json'
    result = subprocess.run(command, capture_output=True, text=True)
    output = result.stdout.strip()

    data = json.loads(output)
    for process in data:
        process_id = str(process["Id"])
        work_set = process["WorkingSet"]
        memory_workingSet = convert_bytes_to_megabytes(work_set)  
        update_process_data(process_id, "workingSet", memory_workingSet)

def update_process_from_util():
    for proc in psutil.process_iter():
        process = psutil.Process(proc.pid)
        memory_info = process.memory_info()
        memory_usage = convert_bytes_to_megabytes(memory_info.rss)
        update_process_data(str(proc.pid), "name", process.name())
        update_process_data(str(proc.pid), "memory_util", memory_usage)


def get_gpu_processes():
    command = 'nvidia-smi --query-compute-apps=pid,process_name,used_gpu_memory --format=csv'
    result = subprocess.run(command, capture_output=True, text=True)
    output = result.stdout
    
    lines = output.strip().split("\n")
    reader = csv.DictReader(lines)

    parsed_info = []
    for row in reader:
        parsed_info.append(row)
        
    return parsed_info

def get_gpu_info():
    pynvml.nvmlInit()

    # 获取GPU数量
    device_count = pynvml.nvmlDeviceGetCount()

    gpu_info = []

    for i in range(device_count):
        handle = pynvml.nvmlDeviceGetHandleByIndex(i)

        # 获取GPU相关信息
        gpu_name = pynvml.nvmlDeviceGetName(handle)
        gpu_temp = pynvml.nvmlDeviceGetTemperature(handle, pynvml.NVML_TEMPERATURE_GPU)

        # 获取GPU资源使用情况
        mem_info = pynvml.nvmlDeviceGetMemoryInfo(handle)
        gpu_memory_total = mem_info.total
        gpu_memory_used_all = mem_info.used
        gpu_memory_percent = (gpu_memory_used_all / gpu_memory_total) * 100

        # 获取GPU利用率信息
        utilization = pynvml.nvmlDeviceGetUtilizationRates(handle)
        gpu_utilization = utilization.gpu

        # 获取正在使用GPU的进程信息
        process_use_gpu = []
        for process in pynvml.nvmlDeviceGetComputeRunningProcesses(handle):
            process_info = {
                'pid': process.pid,
                'memory_usage': process.usedGpuMemory if process.usedGpuMemory is not None else 0,
                'gpu_utilization': process.gpuUtilization if hasattr(process, 'gpuUtilization') else 'N/A'
            }
            process_use_gpu.append(process_info)
        process_use_gpu_nvidia = get_gpu_processes()
            
        gpu_memory = ' '.join([
            f"{gpu_memory_percent:.2f}%  ",
            convert_bytes_to_megabytes(gpu_memory_used_all),
            '/',
            convert_bytes_to_megabytes(gpu_memory_total)
        ])
        gpu = {
            'gpu_index': i,
            'gpu_name': gpu_name,
            'gpu_temp': f"{gpu_temp}°C",
            'gpu_memory': gpu_memory,
            'gpu_utilization': f"{gpu_utilization}%",
            'process': process_use_gpu,
            'process_use_gpu_nvidia': process_use_gpu_nvidia 
        }

        gpu_info.append(gpu)
        
    pynvml.nvmlShutdown()

    return gpu_info

def update_utilization():
    cpu_percent_used = psutil.cpu_percent(interval=0.1)
    memory_usage_used = psutil.virtual_memory().percent
    
    utilization_rate["psutil"]["cpu"] = f"{cpu_percent_used}%"
    utilization_rate["psutil"]["memory"] = f"{memory_usage_used}%"


utilization_rate= {
    "psutil": {
        "cpu": 0,
        "memory": 0,
    },
    "getCounter": {
        "cpu": 0,
        "memory": 0,
    }
}

processes_info = {}

def update_utilization_getCounter():
    # 构建 PowerShell 命令获取系统信息
    command = r"Get-Counter -Counter '\Processor(_Total)\% Processor Time', '\Memory\% Committed Bytes In Use' | Select-Object -ExpandProperty CounterSamples | Select-Object -Property Path, CookedValue"
    
    # 执行 PowerShell 命令并获取输出
    result = subprocess.run(["powershell", "-Command", command], capture_output=True, text=True)
    output = result.stdout.strip()
    
    # 解析输出
    pattern = r"\\(.+)\s+(\d+\.\d+)"
    matches = re.findall(pattern, output)
    
    # 构建系统信息字典
    data = {
        "memory": 0,
        "cpu": 0
    }
    for match in matches:
        path = match[0]
        if "memory" in path:
            utilization_rate["getCounter"]["memory"] = float(match[1])
        else:
            utilization_rate["getCounter"]["cpu"] = float(match[1])
    
    return data

def generate_system_info_json():
    update_utilization()
    # update_utilization_getCounter没有update_utilization 准确
    # update_utilization_getCounter()
    gpu_info = get_gpu_info()
    
    # tasklist  Get-Process pyutil三者获取的进程内存使用基本是一致的
    # update_tasklist()
    # update_process_info()
    update_process_from_util()
    
    # 看起来cpu使用了都不太靠谱的样子
    
    json_output = {
        'process': processes_info,
        'gpu_info': gpu_info,
        'utilization_rate': utilization_rate
    }
    
    json_string = json.dumps(json_output, indent=4)
    return json_string

# 调用函数生成系统信息的JSON并输出
json_string = generate_system_info_json()
print(json_string)
