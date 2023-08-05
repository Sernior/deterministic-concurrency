#include <gtest/gtest.h>
//#include <UserControlledScheduler.h>
#include <DeterministicConcurrency>
#include "scenario1DScheduler.h"
#include "scenario2DScheduler.h"


TEST(UserCtrlSchedulerSimpleTest, Scenario1) {
    EXPECT_EQ(scenario1DS::ret, scenario1DS::expected);
}

TEST(UserCtrlScheduler2ParallelismTest, Scenario1) {
    EXPECT_EQ(scenario2DS::ret1_before, scenario2DS::expected1_before);
    EXPECT_EQ(scenario2DS::ret1_after, scenario2DS::expected1_after);
    EXPECT_EQ(scenario2DS::ret2_before, scenario2DS::expected2_before);
    EXPECT_EQ(scenario2DS::ret2_after, scenario2DS::expected2_after);
}


int main(int argc, char* argv[]) {

    //first Test Act (UserCtrlSchedulerSimpleTest)

    for (int i = 9; i >= 0; i--){
        scenario1DS::sch.switchContextTo(i);
    }

    scenario1DS::sch.joinAll();// end first Test Act

    //second Test Act (UserCtrlScheduler2ParallelismTest)

    scenario2DS::sch.switchContextTo(1,2);// 12 03 13 02 
    scenario2DS::sch.switchContextTo(0,3);
    scenario2DS::sch.switchContextTo(1,3);
    scenario2DS::sch.switchContextTo(0,2);

    scenario2DS::sch.joinAll();// end second Test Act

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
