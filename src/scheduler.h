#ifndef LIBCOROUTINE_SCHEDULER_H
#define LIBCOROUTINE_SCHEDULER_H
#include <define.h>
#include <use_ptr.h>
#include <vector>
#include <unordered_map>
#include <Task.h>
namespace libcoro {
    class Scheduler : public std::enable_shared_from_this<Scheduler> {
    public:
    private:
        using StateBasePtr = UsePtr<StateBase>;
        using StateVector = std::vector<StateBasePtr>;
        using LockType = SpinLock;
        using TaskDictionaryType = std::unordered_map<StateBase*, std::unique_ptr<Task>>;
        
    };
}




#endif //LIBCOROUTINE_SCHEDULER_H
