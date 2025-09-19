#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace ftxui;

struct AppState {
    bool featureA = false;
    bool featureB = true;
    int radio_selected = 0;
    int dropdown_selected = 0;
    std::string input_value;
    std::string last_action;
    bool log_show_real = true;

    std::string ToString(const std::vector<std::string> &radio_options, const std::vector<std::string> &dropdown_options) const
    {
        std::stringstream ss;
        ss << "FeatureA: " << (featureA ? "ON" : "OFF") << " | ";
        ss << "FeatureB: " << (featureB ? "ON" : "OFF") << " | ";
        ss << "Radio: " << radio_options[radio_selected] << " | ";
        ss << "Dropdown: " << dropdown_options[dropdown_selected] << " | ";
        ss << "Input: " << input_value << " | ";
        ss << "Action: " << last_action;
        return ss.str();
    }
};

std::function<void()> update_ui;

std::deque<std::string> logs; // 使用 deque 方便管理日志

std::mutex logs_mutex;

void log_info(const std::string &message)
{
    std::lock_guard<std::mutex> lock(logs_mutex);
    logs.push_back(message);

    if (update_ui) {
        update_ui();
    }
}

Elements split_lines(const std::deque<std::string> &log_lines)
{
    Elements lines;
    for (const auto &line : log_lines) {
        lines.push_back(text(line));
    }
    return lines;
}

class log_file_stream
{
public:
    log_file_stream() = default;

    // 重载 << 操作符
    template <typename T>
    log_file_stream &operator<<(const T &value)
    {
        std::lock_guard<std::mutex> lock(file_mutex_);
        oss_ << value;   // 将值添加到内存缓冲区
        write_to_file(); // 每次插入内容后立即写入文件
        return *this;
    }

private:
    // 将缓冲区内容写入日志文件
    void write_to_file()
    {
        std::ofstream log_f("logs.txt", std::ios::app); // 以追加模式打开文件
        if (log_f.is_open()) {
            log_f << oss_.str(); // 写入当前缓冲区的内容
            oss_.str("");        // 清空缓冲区
            oss_.clear();        // 清空状态
        }
        else {
            std::cerr << "Unable to open log file!" << std::endl;
        }
    }

    std::ostringstream oss_;       // 用于构建日志消息
    static std::mutex file_mutex_; // 用于确保多线程写入安全
};

// 初始化静态成员
std::mutex log_file_stream::file_mutex_;

// 创建一个 global 的流实例
log_file_stream log_file;

AppState state;

constexpr int LOG_HEIGHT = 18;
constexpr int MAX_LOG_LINES = LOG_HEIGHT - 3;
// 根据滑块值计算显示的日志行范围
void updateShowIndex(int log_size, int slider_value, int &start, int &end)
{
    if (MAX_LOG_LINES >= log_size) {
        start = 0;
        end = log_size - 1;
    }
    else if (state.log_show_real) {
        end = log_size - 1;
        start = std::max(0, end - MAX_LOG_LINES + 1);
    }
    else {
        end = (slider_value / 100.0) * (log_size - 1);
        end = std::min(end, static_cast<int>(log_size) - 1);

        start = end - MAX_LOG_LINES + 1;
        start = std::max(0, start);
    }
}

int main()
{
    // 控件选项
    std::vector<std::string> radio_options = {"Option 1", "Option 2", "Option 3"};
    std::vector<std::string> dropdown_options = {"Item 1", "Item 2", "Item 3"};

    // 控件绑定
    auto checkboxA = Checkbox("Feature A", &state.featureA);
    auto checkboxB = Checkbox("Feature B", &state.featureB);
    auto checkboxes = Container::Horizontal({checkboxA, checkboxB});

    auto radiobox = Radiobox(&radio_options, &state.radio_selected);
    auto dropdown_menu = Menu(&dropdown_options, &state.dropdown_selected);

    auto input = Input(&state.input_value, "Type here...");
    auto button_ok = Button("OK", [&] {
        state.last_action = "Pressed OK";
        log_info("[User] Pressed OK");
    });
    auto button_cancel = Button("Cancel", [&] {
        state.last_action = "Cancelled";
        log_info("[User] Pressed Cancel\n");
    });
    auto buttons = Container::Horizontal({button_ok, button_cancel});
    auto input_buttons = Container::Horizontal({input, buttons});

    // 上部控件容器
    auto controls = Container::Vertical({
        checkboxes,
        radiobox,
        input_buttons,
        dropdown_menu,
    });

    auto controls_renderer = Renderer(controls, [&] {
        return vbox({
                   hbox({checkboxA->Render(), checkboxB->Render()}),
                   radiobox->Render(),
                   hbox({input->Render(), button_ok->Render(), button_cancel->Render()}),
                   dropdown_menu->Render(),
               })
               | border;
    });

    // 日志区
    int value = 0;

    SliderOption<int> option;
    option.value = &value;
    option.min = 1;
    option.max = 100;
    option.increment = 1;
    option.color_active = Color::Blue;
    option.color_inactive = Color::Yellow;
    option.direction = GaugeDirection::Down;
    auto slider_comp = Slider<int>(option); // ← 交互组件

    auto log_show_real = Checkbox("", &state.log_show_real);
    auto logs_view = Renderer([&] {
        std::deque<std::string> logs_copy;
        {
            std::lock_guard<std::mutex> lock(logs_mutex);
            logs_copy = logs;
        }

        int slider_value = value;
        int start = 0;
        int end = 0;
        updateShowIndex(logs_copy.size(), slider_value, start, end);

        log_file << "start=" << start << ", end=" << end << ", slider_value=" << slider_value
                 << ", logs_copy.size()=" << logs_copy.size() << "\n";

        // 提取日志区域
        std::deque<std::string> visible_logs(logs_copy.begin() + start, logs_copy.begin() + end + 1);

        return vbox(split_lines(visible_logs)) | xflex;
    });

    auto logs_and_slider = Container::Horizontal({
        // ← 事件分发容器
        logs_view,
        Container::Vertical({
            log_show_real,
            slider_comp | border | flex,
        }),
    });

    /*
        Container::Horizontal  容器，负责界面的布局
        Renderer                渲染器，负责界面的重绘（相当于 paintEvent），必须存在，否则界面不刷新
    */
    // 注意 Container::Horizontal 和 Renderer的使用，否则会出现 Slider无法相应键鼠事件
    auto logs_renderer = Renderer(logs_and_slider, [&] {
        return vbox({
                   text("[ Logs ]: | 点击Slider后，支持 ↑ ↓  | 不支持换行显示"),
                   logs_and_slider->Render() | flex,
               })
               | border;
    });

    // 状态显示区域
    auto status_renderer = Renderer([&] {
        return text("Status: " + state.ToString(radio_options, dropdown_options)) | color(Color::Yellow) | center | border;
    });
    auto title_renderer = Renderer([&] { return text("================= FTXUI ================="); });

    // 总组件
    auto main_component = Container::Vertical({
        title_renderer,
        // 日志框固定大小
        Container::Horizontal({
            controls_renderer | size(WIDTH, EQUAL, 90) | size(HEIGHT, EQUAL, LOG_HEIGHT),
            logs_renderer | size(WIDTH, EQUAL, 60) | size(HEIGHT, EQUAL, LOG_HEIGHT),
        }),
        status_renderer | size(WIDTH, EQUAL, 150 - 2),
    });

    // 启动日志线程
    std::thread logger([=] {
        for (int i = 1; i <= 90; ++i) {
            if (i % 3 == 0) {
                log_info("🔴 Log line " + std::to_string(i) + " -> with some extra details to make it longer.");
            }
            else {
                log_info("  Log line " + std::to_string(i));
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
        }
        log_info("✅ Logger finished.\n");
    });
    logger.detach();

    auto screen = ScreenInteractive::TerminalOutput();
    update_ui = [&screen]() { screen.PostEvent(Event::Custom); };
    screen.Loop(main_component);

    return 0;
}
