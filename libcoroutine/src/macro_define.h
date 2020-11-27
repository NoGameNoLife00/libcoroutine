#ifndef LIBCOROUTINE_MACRO_DEFINE_H
#define LIBCOROUTINE_MACRO_DEFINE_H

#ifndef _offset_of
#define _offset_of(c, m) reinterpret_cast<size_t>(&static_cast<c *>(0)->m)
#endif

#define go (*::libcoro::ThisScheduler()) +
#define GO  (*::libcoro::ThisScheduler()) + [=]() mutable->libcoro::Future<>

#define CurrentScheduler() (co_await ::libcoro::GetCurrentScheduler())
#define RootState() (co_await ::libcoro::GetRootState())
#define CurrentTask() (co_await ::libcoro::GetCurrentTask())

#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif // likely
#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif // unlikely

#endif //LIBCOROUTINE_MACRO_DEFINE_H
