#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <iostream>

namespace py = pybind11;

class CallbackExample {
public:
    // setCallback 方法接受一个 Python 类实例
    void setCallback(py::object callback) {
        this->callback = callback;
    }

    // triggerCallback 方法，调用 Python 类的成员函数
    void triggerCallback(int value) {
        if (callback) {
            // 调用 Python 类实例的方法
            std::cout << "cpp:  call py" << std::endl;
            callback.attr("onCallback")(value);
        }
    }

private:
    py::object callback;  // 保存 Python 类实例
};

PYBIND11_MODULE(example, m) {
    // 将 CallbackExample 类暴露给 Python
    py::class_<CallbackExample>(m, "CallbackExample")
        .def(py::init<>())  // 默认构造函数
        .def("setCallback", &CallbackExample::setCallback)  // 设置回调函数
        .def("triggerCallback", &CallbackExample::triggerCallback);  // 触发回调
}
