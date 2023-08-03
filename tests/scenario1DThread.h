#include <DeterministicThread.h>
#include <vector>

namespace scenario1DT{

    static std::vector<int> in{1,1,2,2,3,3,4,4,5,5,6,6};
    static int index{};
    static std::vector<int> out;
    static std::vector<int> exp = {0, 1, 1, 1, 2, 2, 3, 2, 4, 3, 5, 3, 6, 4, 7, 4, 8, 5, 9, 5, 10, 6, 11, 6};

    void threadFunction(DeterministicConcurrency::thread_context* t) {
        t->wait_for_tick();
        while (index < in.size()){
            out.push_back(index);
            out.push_back(in[index]);
            index++;
            t->tock();
            t->wait_for_tick();
        }
    }
}

// {0, 1, 1, 1, 2, 2, 3, 2, 4, 3, 5, 3, 6, 4, 7, 4, 8, 5, 9, 5, 10, 6, 11, 6}