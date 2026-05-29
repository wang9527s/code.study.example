## 简单协程库

### 01.coroutine_ucontext - 基于 ucontext

基于 POSIX `ucontext` API 实现的协作式协程库，用于学习协程原理。（linux和mac平台适用）

<span style="color:red">核心依赖 ucontext 系列函数，实现运行栈的切换</span>

- `getcontext(&ctx)` — 获取当前 CPU 状态快照
- `makecontext(&ctx, func, argc)` — 在指定栈上虚构一个"即将执行 func"的栈帧
- `swapcontext(&from, &to)` — <span style="color:red">保存当前状态到 from，切换到 to 执行</span>，即切换协程

#### API

| 函数 | 说明 |
|------|------|
| `co::create(func)` | 创建协程，返回 id |
| `co::resume(id)` | 调度器调用，切换到指定协程执行 |
| `co::yield()` | 协程内部调用，让出 CPU 回到调度器 |

#### 注意点

- **非对称协程**: 协程只能和调度器交互，不能互相切换
- **协作式调度**: 协程必须主动 yield，不会被强制打断
  - 因此：`co::resume` 只能调度器调用，`co::yield` 只能协程内部调用，搞反会导致 main_ctx 被覆盖


### 02.coroutine_asm - 汇编

和ucontext的逻辑基本相同，主要是自己实现了：

1) 在指定栈上虚构一个"即将执行 func"的栈帧```fake_stack```。  
2) 切换到指定的栈帧执行```context_switch```。

