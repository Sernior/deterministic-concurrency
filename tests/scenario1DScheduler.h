#include <UserControlledScheduler.h>
#include <vector>

namespace scenario1DS{

    static std::vector<int> ret;

    void threadFunc(DeterministicConcurrency::thread_context* t, int arg) {
        t->start();

        ret.push_back(arg);

        t->finish();
    }

    static DeterministicConcurrency::UserControlledScheduler<10> sch{
        std::tuple{&threadFunc, 0},
        std::tuple{&threadFunc, 1},
        std::tuple{&threadFunc, 2},
        std::tuple{&threadFunc, 3},
        std::tuple{&threadFunc, 4},
        std::tuple{&threadFunc, 5},
        std::tuple{&threadFunc, 6},
        std::tuple{&threadFunc, 7},
        std::tuple{&threadFunc, 8},
        std::tuple{&threadFunc, 9}
    };

    static std::vector<int> expected{9,8,7,6,5,4,3,2,1,0};

}
