### 项目说明

| 项目                         | 说明                  |
|------------------------------|-----------------------|
| [pdfwriter](#jump_pdfwriter) | 导出pdf(支持2G以上)   |
| qdbus                        | dbus demo             |
| qtcp                         | tcp客户端和服务端demo |
| [sqlmodel](#jump_sqlmodel)   | 可视化数据库 |
| SystemSemaphone              | 信号量的使用          |
| [ScreenShot](#jump_ScreenShot)| 截屏          |

### <span id="jump_pdfwriter">pdfwriter</span>

&emsp;&emsp;Qt5.8的导出pdf，如果文件过大，导出失败。  
&emsp;&emsp;现在分段导出，使用[qpdf](https://github.com/qpdf/qpdf/releases)进行合并。

### <span id="jump_sqlmodel">sqlmodel</span>

&emsp;&emsp;  
&emsp;&emsp;使用sqlmodel，实时显示数据库表中的数据。

### <span id="jump_ScreenShot">截屏</span>

<center><img src=./Readme_img/screen_shot.png width=80% /></center>

&emsp;&emsp;一个功能简单的截屏软件，提供如下功能：

- 支持多屏、高分屏幕；
  
- 支持绘制矩形、绘制文字以及绘制箭头的功能；
  
- 支持撤销操作；
  
- 随时调整截屏区域的位置以及大小；
  
  拖拽选择区域后，可以整体拖拽移动，也可以拖动边框调整大小；

- 提供保存到本地以及剪切板的功能；