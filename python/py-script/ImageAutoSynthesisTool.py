"""
名称：图片自动合成工具
背景：root_dir的子目录中包含一组照片，可以使用Imaging Edge Desktop进行照片的合成
功能：打开子目录，拖拽照片到Imaging Edge Desktop中，自动右击进行图片合成；然后将合成的结果移动并重命名到root_dir中。循环操作。
"""

import pyautogui
import time
import os
import sys
import pywinauto
from shutil import copyfile

# 打开son_dir，并拖拽其中的图片到Viewer中
def drap_to_viewer(root_dir, son_dir_name):
    # TASKKILL /F /IM explorer.exe会连桌面一起关闭  
    # 关闭所有文件管理器窗口
    os.system("TASKKILL /IM explorer.exe") 
    time.sleep(1)
    folder_path = root_dir + "\\" + son_dir_name
    os.system("start explorer %s" % folder_path)

    # 用pywinauto获取文件夹和目标窗口的句柄
    folder = pywinauto.Desktop(backend='uia').window(title=son_dir_name)
    target = pywinauto.Application(backend='uia').connect(title='Viewer').window() 

    # 拖拽folder_path中一个图片到目标窗口，Viewer会自动加载同级目录中的所有ARW图片 
    folder_rect = folder.rectangle()
    folder_coords = (folder_rect.left + 220, folder_rect.top + 160) 
    target_rect = target.rectangle()  
    target_coords = (target_rect.left + 340, target_rect.top + 160)

    pyautogui.mouseDown(folder_coords[0], folder_coords[1]) 
    pyautogui.moveTo(target_coords[0], target_coords[1], duration=1)
    pyautogui.mouseUp()
    
def list_folders_and_files(folder_path, depth=1):
    folders = []
    files = []
    
    for f in os.scandir(folder_path):
        name = f.name
        path = f.path
        
        if f.is_dir():
            folders.append((name, path))
            if depth > 1:
                new_folders, new_files = list_folders_and_files(path, depth-1)
                folders.extend(new_folders)
                files.extend(new_files)
        else:
            files.append((name, path))
            
    return folders, files
    
def get_one_image(son_dir_name, son_dir_pathname):
    drap_to_viewer(root_dir, son_dir_name)
    pyautogui.hotkey('ctrl', 'a')
    # 右击菜单 选择像素转换多重拍摄
    pyautogui.click(button='right')
    pyautogui.move(50, 80)
    pyautogui.click()

    pyautogui.press('enter') 
    for count in range(50):
        sys.stdout.write('.')
        sys.stdout.flush()
        time.sleep(0.5)
    print("")
    pyautogui.press('enter') 

    # 移动图片（一些文件名命名规则而已）
    _, son_dir_files = list_folders_and_files(son_dir_pathname)
    result_basename = son_dir_files[0][0].split('.')[0]
    result_name = result_basename + "_PSMS16.JPG"
    from_pathname = os.path.expanduser('~') + "\\Pictures\\" + result_name
    to_pathname = root_dir + "\\" + son_dir_name.split('.')[0] + ".JPG"
    if os.path.exists(from_pathname):
        if os.path.exists(to_pathname):
            os.remove(to_pathname)
        copyfile(from_pathname, to_pathname)  # 可以跨驱动器移动文件
        os.remove(from_pathname)
        print("mv {0} {1}".format(from_pathname, to_pathname))
    else:
        print("build Failed: ", son_dir_pathname)
    print("\n\n")

root_dir = ""
if __name__ == '__main__':
    if (len(sys.argv) < 2):
        print("请输入工作目录，example：")
        print("    ImageAutoSynthesisTool.py C:\\Users\\xbrain\\Pictures\\sony_pix_0907")
        os._exit(-1)
        
    root_dir = sys.argv[1]
    
    # 如果Viewer没有启动，则启动。
    process = 'Viewer.exe' 
    cmd = 'wmic process where "name=\'{}\'" get ProcessId'.format(process)
    if (os.system(cmd) == 0):
        print("start run Viewer")
        os.startfile("C:\\Program Files\\Sony\\Imaging Edge\\Viewer.exe")
        time.sleep(10)
    else:
        print("Viewer is running")
        
    # 获取root_dir文件夹中子目录，循环处理
    folders, _ = list_folders_and_files(root_dir)
    for dir_info in folders:
        get_one_image(dir_info[0], dir_info[1])
