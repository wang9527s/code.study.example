#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::string s_clear_enter = "\033[0m\n";
    std::string s_clear_space = "\033[0m ";

    std::cout << "\n\033[1m>>> 常用控制示例：\033[0m\n";
    std::cout << "    \033[0m" << "[0] 重置（Reset all styles）" << s_clear_enter;
    std::cout << "    \033[1m" << "[1] 加粗（Bold / Increased Intensity）" << s_clear_enter;
    std::cout << "    \033[2m" << "[2] 变淡（Dim / Decreased Intensity）" << s_clear_enter;
    std::cout << "    \033[3m" << "[3] 斜体（Italic）" << s_clear_enter;
    std::cout << "    \033[4m" << "[4] 下划线（Underline）" << s_clear_enter;
    std::cout << "    \033[7m" << "[7] 反色（Reverse Video / Invert）   PS: 不同于背景色" << s_clear_enter;
    std::cout << "    \033[8m" << "[8] 隐藏（Conceal / Hidden text）" << "    \033[0m ←你可能看不见" << std::endl;
    std::cout << "    \033[9m" << "[9] 删除线（Strikethrough）" << s_clear_enter;

    std::cout << "\n\033[1m>>> 常用前景颜色示例：\033[0m\n";
    std::cout << "    \033[30m" << "黑色" << s_clear_space;
    std::cout << "    \033[31m" << "红色" << s_clear_space;
    std::cout << "    \033[32m" << "绿色" << s_clear_space;
    std::cout << "    \033[33m" << "黄色" << s_clear_space;
    std::cout << "    \033[34m" << "蓝色" << s_clear_space;
    std::cout << "    \033[35m" << "洋红" << s_clear_space;
    std::cout << "    \033[36m" << "青色" << s_clear_space;
    std::cout << "    \033[37m" << "白色" << s_clear_enter;

    std::cout << "\n\033[1m>>> 背景颜色示例：\033[0m\n";
    std::cout << "    \033[40m" << " 黑底 " << s_clear_space;
    std::cout << "    \033[41m" << " 红底 " << s_clear_space;
    std::cout << "    \033[42m" << " 绿底 " << s_clear_space;
    std::cout << "    \033[43m" << " 黄底 " << s_clear_space;
    std::cout << "    \033[44m" << " 蓝底 " << s_clear_space;
    std::cout << "    \033[45m" << " 洋红底 " << s_clear_space;
    std::cout << "    \033[46m" << " 青底 " << s_clear_space;
    std::cout << "    \033[47m" << " 白底 " << s_clear_enter;

    std::cout << "\n\033[1m>>> 组合颜色示例：\033[0m\n";
    std::cout << "    \033[1;37;41m" << " 白字红底加粗 " << s_clear_space;
    std::cout << "    \033[4;32;47m" << " 绿字白底下划线 " << s_clear_space;
    std::cout << "    \033[3;36m" << " 青字斜体 " << s_clear_space;
    std::cout << "    \033[9;31m" << " 红字删除线 " << s_clear_enter;

    // 模拟闪烁
    std::cout << "\n\033[1m>>> 模拟闪烁文本：\033[0m\n";
    bool show = true;
    for (int i = 0; i < 10; ++i) {
        if (show) {
            std::cout << "\r\033[K" << "    \033[1;31m" 
                << "[闪烁文本] " << "\033[0m" << std::flush;
        } else {
            std::cout << "\r\033[K" << std::flush;
        }
        show = !show;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << std::endl;

    return 0;
}
