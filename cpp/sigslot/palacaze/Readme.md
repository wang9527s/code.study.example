
### palacaze/sigslot

1. 信号的连接/断开（connect/disconnect）是线程安全的；在函数内部已经加锁了

```cpp
namespace sigslot{
class signal_base {
    void add_slot(slot_ptr &&s) {
        const group_id gid = s->group();

        lock_type lock(m_mutex);
    }
}
}

### ex-thread

  跨线程使用sigslot，demo验证结果

+ 槽函数运行在信号所在线程。  

+ 无法实现槽函数运行在指定线程，有方法可以绕过。（还是qt的好用）

  1. 槽函数中使用原子变量、条件变量、无锁队列等在不同或者同一个线程对状态（数据）进行修改。
  2. ctrl线程，轮询判断1中的条件，然后进行任务的分发。