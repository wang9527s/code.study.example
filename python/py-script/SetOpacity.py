'''
Python 3.7.9
'''

import win32gui
import win32con
import win32api

import argparse

# -c 1 -g
# -c 0 -s hd 0.1
parser = argparse.ArgumentParser()
parser.add_argument('-c', '--control',help="a optional argument",type=int, choices=[0, 1])
parser.add_argument('-g', '--get', action="store_true")
parser.add_argument('-s', '--set', type=str, nargs='+')

args = parser.parse_args()

def windowEnumerationHandler(hwnd, windowlist):
    windowlist.append((hwnd, win32gui.GetWindowText(hwnd)))

#通过枚举获取所有窗口的句柄和标题
windowlist = []
win32gui.EnumWindows(windowEnumerationHandler, windowlist)

if args.control == 1:
    for i in windowlist:
        if i[1] == "":
            continue
        if "Realtek Jack Windows" in i[1]:
            continue

        if win32gui.GetParent(i[0])==0:
            print(i) 
if args.control == 0: 
    param = args.set
    print (param) 
    nExStyle = win32api.GetWindowLong(param[0],win32con.GWL_EXSTYLE)
    nExStyle |= win32con.WS_EX_LAYERED
    win32api.SetWindowLong(param[0],win32con.GWL_EXSTYLE, nExStyle)
    win32gui.SetLayeredWindowAttributes(param[0],0,int(param[1]),win32con.LWA_ALPHA)