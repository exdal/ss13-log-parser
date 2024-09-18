#include "core/stack.hh"

#include "core/os.hh"

ThreadStack::ThreadStack() {
    constexpr static usize stack_size = ls::mib_to_bytes(32);
    ptr = static_cast<u8 *>(os::mem_reserve(stack_size));
    os::mem_commit(ptr, stack_size);
}

ThreadStack::~ThreadStack() {
    os::mem_release(ptr);
}

ScopedStack::ScopedStack() {
    auto &stack = get_thread_stack();
    ptr = stack.ptr;
}

ScopedStack::~ScopedStack() {
    auto &stack = get_thread_stack();
    stack.ptr = ptr;
}
