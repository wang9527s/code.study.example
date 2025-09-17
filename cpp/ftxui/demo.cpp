#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <chrono>
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

const size_t MAX_LOG_LINES = 8;
std::deque<std::string> logs; // 使用 deque 方便管理日志

std::mutex logs_mutex;

void log_info(const std::string &message)
{
    std::lock_guard<std::mutex> lock(logs_mutex);
    logs.push_back(message);
    if (logs.size() > MAX_LOG_LINES) {
        logs.pop_front(); // 保证最多只保留最新的 N 条日志
    }

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

AppState state;
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
    auto logs_renderer = Renderer([&] {
        std::deque<std::string> logs_copy;
        {
            std::lock_guard<std::mutex> lock(logs_mutex);
            logs_copy = logs; // 获取最新的日志副本
        }

        Elements logs_view = split_lines(logs_copy);

        return vbox({
                   text("Logs (program logs):"),
                   vbox(logs_view),
               })
               | border;
    });

    // 状态显示区域
    auto status_renderer = Renderer([&] {
        return text("Status: " + state.ToString(radio_options, dropdown_options)) | color(Color::Yellow) | center | border;
    });

    // 总组件
    auto main_component = Container::Vertical({
        Container::Horizontal({
            controls_renderer | size(WIDTH, EQUAL, 120),
            logs_renderer | size(WIDTH, EQUAL, 120),
        }),
        status_renderer | size(WIDTH, EQUAL, 260),
    });

    // 启动日志线程
    std::thread logger([=] {
        for (int i = 1; i <= 50; ++i) {
            log_info("  Log line " + std::to_string(i));
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
        }
        log_info("✅ Logger finished.\n");
    });
    logger.detach();

    auto screen = ScreenInteractive::TerminalOutput();
    update_ui = [&screen]() { screen.PostEvent(Event::Custom); };
    // 启动屏幕循环
    screen.Loop(main_component);

    return 0;
}