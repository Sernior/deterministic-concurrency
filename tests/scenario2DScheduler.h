#include <DeterministicConcurrency>
#include <vector>

namespace scenario2DS{

    static std::vector<int> ret1_before;

    static std::vector<int> ret2_before;

    static std::vector<int> ret1_after;

    static std::vector<int> ret2_after;

    void threadFunc1(DeterministicConcurrency::thread_context* t, int arg1, int arg2) {
        ret1_before.push_back(arg1);

        t->switchContext();

        ret1_after.push_back(arg2);
    }

    void threadFunc2(DeterministicConcurrency::thread_context* t, int arg1, int arg2) {
        ret2_before.push_back(arg1);

        t->switchContext();

        ret2_after.push_back(arg2);
    }

    static DeterministicConcurrency::UserControlledScheduler<4> sch{
        std::tuple{&threadFunc1, 0, 1}, //0
        std::tuple{&threadFunc1, 2, 3}, //1
        std::tuple{&threadFunc2, 4, 5}, //2
        std::tuple{&threadFunc2, 6, 7}  //3
    };

    static std::vector<int> expected1_before{2,0};

    static std::vector<int> expected1_after{3,1};

    static std::vector<int> expected2_before{4,6};

    static std::vector<int> expected2_after{7,5};
    // 12 03 13 02 
}
