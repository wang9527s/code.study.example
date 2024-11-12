#include <pybind11/pybind11.h>
#include <pybind11/functional.h>  // 必须包含这个头文件

namespace py = pybind11;

class CallbackExample {
public:
    void setCallback(std::function<void(int)> cb) {
        callback = cb;
    }

    void triggerCallback(int value) {
        if (callback) {
            callback(value);
        }
    }

private:
    std::function<void(int)> callback;
};

PYBIND11_MODULE(example, m) {
    py::class_<CallbackExample>(m, "CallbackExample")
        .def(py::init<>())
        .def("setCallback", &CallbackExample::setCallback)
        .def("triggerCallback", &CallbackExample::triggerCallback);
}
