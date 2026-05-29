
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <functional>

#include "context_switch.h"

// 简单协程库 - 基于汇编上下文切换

namespace co {

static const uint32_t SCHEDULER_ID = 0; // 调度器ID，也用作"当前无协程运行"的标识

static const uint32_t STACK_SIZE = 64 * 1024;

enum State {
    ready,    // 就绪
    running,  // 运行中
    suspend,  // 挂起
    dead      // 已结束
};

struct Coroutine {
    uint32_t id;
    Context ctx;
    std::function<void()> func;
    State state;
    char* stack;

    Coroutine(uint32_t id, std::function<void()> f)
        : id(id), func(f), state(ready), stack(nullptr) {
        memset(&ctx, 0, sizeof(ctx));
        stack = new char[STACK_SIZE];
    }
    ~Coroutine() { delete[] stack; }
};

struct Scheduler {
    Context main_ctx;
    std::unordered_map<uint32_t, Coroutine*> coroutines;
    uint32_t running_id;    // 当前运行的协程 ID, SCHEDULER_ID 表示无
    uint32_t alloc_id;     // 下一个分配的协程 ID

    Scheduler() : running_id(SCHEDULER_ID), alloc_id(1) {
        memset(&main_ctx, 0, sizeof(main_ctx));
    }
    ~Scheduler() {
        for (auto& [id, c] : coroutines) delete c;
    }
};

static Scheduler g_sched;

// 协程入口跳板
static void coroutine_entry() {
    Coroutine* c = g_sched.coroutines[g_sched.running_id];
    c->func();
    c->state = dead;
    g_sched.running_id = SCHEDULER_ID;
    // 约等于 c->ctx.uc_link = &g_sched.main_ctx; 
    // 在协程结束时切回调度器     
    Context dummy;
    context_switch(&dummy, &g_sched.main_ctx);
    __builtin_unreachable();
}

// 在协程的独立栈上虚构一个"正准备执行 coroutine_entry"的栈帧
static void fake_stack(Coroutine* c) {
    char* stack_top = c->stack + STACK_SIZE;
    stack_top = (char*)((uintptr_t)stack_top & ~0xFULL);

#if defined(__aarch64__)
    c->ctx.sp = (uint64_t)stack_top;
    c->ctx.lr = (uint64_t)coroutine_entry;
    c->ctx.fp = 0;
#elif defined(__x86_64__)
    stack_top -= 8;  // 模拟 call 压栈, 保持 16 字节对齐
    c->ctx.rsp = (uint64_t)stack_top;
    c->ctx.rip = (uint64_t)coroutine_entry;
    c->ctx.rbp = 0;
#endif
}

uint32_t create(std::function<void()> func) {
    uint32_t id = g_sched.alloc_id++;
    Coroutine* c = new Coroutine(id, func);
    g_sched.coroutines[id] = c;
    return id;
}

void resume(uint32_t id) {
    Coroutine* c = g_sched.coroutines[id];
    if (c->state == dead) return;

    switch (c->state) {
    case ready: {
        fake_stack(c);
        c->state = running;
        g_sched.running_id = id;
        context_switch(&g_sched.main_ctx, &c->ctx);
        break;
    }
    case suspend: {
        c->state = running;
        g_sched.running_id = id;
        context_switch(&g_sched.main_ctx, &c->ctx);
        break;
    }
    default:
        break;
    }
}

void yield() {
    uint32_t id = g_sched.running_id;
    if (id == SCHEDULER_ID) return;

    Coroutine* c = g_sched.coroutines[id];
    c->state = suspend;
    g_sched.running_id = SCHEDULER_ID;
    context_switch(&c->ctx, &g_sched.main_ctx);
}

uint32_t current() {
    return g_sched.running_id;
}

void schedule() {
    while (true) {
        bool active = false;
        for (auto& [id, c] : g_sched.coroutines) {
            if (c->state != dead) {
                active = true;
                resume(id);
            }
        }
        if (!active) break;
    }
}

} // namespace co

// ========== 使用示例 ==========

void task_a() {
    for (int i = 0; i < 10; i++) {
        printf("[协程%u] A1 i=%d\n", co::current(), i);
        co::yield();
        printf("[协程%u] A2 i=%d\n", co::current(), i);
        co::yield();
    }
    printf("[协程%u] A (结束)\n", co::current());
}

void task_b() {
    uint32_t num = 0;
    for (int i = 0; i < 10; i++) {
        num += i;
        printf("[协程%u] B i=%d, num = %d\n", co::current(), i, num);
        co::yield();
    }
    printf("[协程%u] B (结束)\n", co::current());
}

int main() {
#if defined(__aarch64__)
    printf("=== 简单协程演示 (asm, ARM64) ===\n\n");
#elif defined(__x86_64__)
    printf("=== 简单协程演示 (asm, x86-64) ===\n\n");
#endif

    uint32_t co_a = co::create(task_a);
    uint32_t co_b = co::create(task_b);

    printf("[主线程 co=%u] 开始调度\n\n", co::current());

    co::schedule();

    printf("\n[主线程 co=%u] 所有协程已结束\n", co::current());
    return 0;
}
