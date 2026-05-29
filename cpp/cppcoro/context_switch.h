#ifndef CONTEXT_SWITCH_H
#define CONTEXT_SWITCH_H

#include <cstdint>

#if defined(_WIN32) || defined(_WIN64)
#error "Windows is not supported, use Linux or macOS"
#endif

#if !defined(__LP64__) && !defined(__x86_64__) && !defined(__aarch64__)
#error "32-bit systems are not supported, only 64-bit"
#endif

// macOS 导出符号需要 _ 前缀, Linux 不需要
#if defined(__APPLE__)
#define ASM_SYMBOL "_context_switch"
#else
#define ASM_SYMBOL "context_switch"
#endif

// ==================== ARM64 (macOS / Linux) ====================
#if defined(__aarch64__)

// 只需保存 callee-saved 寄存器（调用约定保证 caller-saved 由调用方自行处理）:
//   x19-x28: 通用 callee-saved（编译器可能将频繁使用的局部变量放在这些寄存器中）
//   fp(x29): 帧指针 —— 指向当前函数栈帧的底部，形成链表用于调试回溯(backtrace)
//   lr(x30): 返回地址 —— 切换回来后从哪行代码继续执行
//   sp:      栈指针 —— 指向栈顶，切换 sp 就切换了整个协程的栈
//
// 栈与栈帧的关系:
//   栈(stack) = 整块连续内存，每个协程独立分配一块
//   栈帧(frame) = 栈中属于某个函数的那一段（局部变量、保存的寄存器、返回地址）
//   每调用一个函数多一帧，返回则弹掉一帧。sp 指向栈顶，fp 指向当前帧底部。
struct Context {
    uint64_t x19, x20, x21, x22, x23;  // +0,  +8,  +16, +24, +32
    uint64_t x24, x25, x26, x27, x28;  // +40, +48, +56, +64, +72
    uint64_t fp;                         // +80
    uint64_t lr;                         // +88
    uint64_t sp;                         // +96
};

// void context_switch(Context* from [x0], Context* to [x1])
//
// 工作原理: 把当前 CPU 寄存器写入 from 指向的内存（存档），
//          再从 to 指向的内存读回寄存器（读档），然后 ret 跳到 to->lr。
__asm__(
    ".text\n"                           // 放在代码段
    ".globl " ASM_SYMBOL "\n"           // 导出符号，让链接器能找到
    ".p2align 2\n"                      // 4字节对齐（2^2），ARM64 每条指令固定4字节
    ASM_SYMBOL ":\n"

    // --- 存档: 把当前寄存器的值写入 Context* from [x0] ---
    //     stp = Store Pair, 一条指令存两个寄存器(16字节)到内存
    "    stp x19, x20, [x0, #0]\n"      // 移动 x19、x20 到 x0的#0偏移处
    "    stp x21, x22, [x0, #16]\n"
    "    stp x23, x24, [x0, #32]\n"
    "    stp x25, x26, [x0, #48]\n"
    "    stp x27, x28, [x0, #64]\n"
    "    stp fp, lr, [x0, #80]\n"       // fp(帧指针) + lr(返回地址)
    "    mov x2, sp\n"                  // sp 不能直接 str，先搬到 x2
    "    str x2, [x0, #96]\n"          

    // --- 读档: 从 to (x1) 指向的 Context 结构体读回寄存器 ---
    //     ldp = Load Pair, 一条指令从内存读两个寄存器
    "    ldp x19, x20, [x1, #0]\n"
    "    ldp x21, x22, [x1, #16]\n"
    "    ldp x23, x24, [x1, #32]\n"
    "    ldp x25, x26, [x1, #48]\n"
    "    ldp x27, x28, [x1, #64]\n"
    "    ldp fp, lr, [x1, #80]\n"        // 恢复 fp + lr
    "    ldr x2, [x1, #96]\n"           
    "    mov sp, x2\n"                  // 读取之前保存的sp，即切换到运行栈帧执行 （栈还是在内存上的）

    "    ret\n"                          // 跳转到 lr，即目标协程上次暂停的位置继续执行   TODO 这里的ret还是有点不明白
);

// ==================== x86-64 (Linux / macOS) ====================
#elif defined(__x86_64__)

// x86-64 callee-saved 寄存器:
//   rbx, rbp, r12-r15: 通用
//   rsp: 栈指针
//   rip: 返回地址（由 call 指令压入栈顶）
struct Context {
    uint64_t rbx;                        // +0
    uint64_t rbp;                        // +8
    uint64_t r12, r13, r14, r15;         // +16, +24, +32, +40
    uint64_t rsp;                        // +48
    uint64_t rip;                        // +56
};

// void context_switch(Context* from [rdi], Context* to [rsi])
//
// x86-64 调用约定: 第一个参数在 rdi, 第二个在 rsi
// x86-64 没有独立的 lr 寄存器, 返回地址是 call 指令压在栈顶的
__asm__(
    ".text\n"
    ".globl " ASM_SYMBOL "\n"
    ".p2align 4\n"                      // 16字节对齐（2^4），x86 惯例，利于指令预取
    ASM_SYMBOL ":\n"

    // --- 存档: 寄存器 → from (rdi) 指向的内存 ---
    //     movq = Move Quadword (8字节), x86 没有 stp，只能逐个存
    //     偏移(%rdi) = 地址 rdi+偏移
    "    movq %rbx,  0(%rdi)\n"
    "    movq %rbp,  8(%rdi)\n"
    "    movq %r12, 16(%rdi)\n"
    "    movq %r13, 24(%rdi)\n"
    "    movq %r14, 32(%rdi)\n"
    "    movq %r15, 40(%rdi)\n"
    "    movq %rsp, 48(%rdi)\n"
    "    movq (%rsp), %rax\n"           // 栈顶 = call 压入的返回地址，取到 rax
    "    movq %rax,  56(%rdi)\n"        // 存为 rip

    // --- 读档: to (rsi) 指向的内存 → 寄存器 ---
    "    movq  0(%rsi), %rbx\n"
    "    movq  8(%rsi), %rbp\n"
    "    movq 16(%rsi), %r12\n"
    "    movq 24(%rsi), %r13\n"
    "    movq 32(%rsi), %r14\n"
    "    movq 40(%rsi), %r15\n"
    "    movq 48(%rsi), %rsp\n"         // 切换到目标栈
    "    movq 56(%rsi), %rax\n"         // rax = 目标返回地址
    "    jmpq *%rax\n"                  // 跳转到目标协程上次暂停处
);

#else
#error "Unsupported architecture: only ARM64 and x86-64 are supported"
#endif

extern "C" void context_switch(Context* from, Context* to);

#endif // CONTEXT_SWITCH_H
