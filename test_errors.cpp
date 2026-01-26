#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "procdata.h"

class DataInits : public ::testing::Environment {
public:
public:
    static ProcData data_source;

    static void SetUpTestSuite() {
        data_source = ProcData();
    }
};

// Static member definition
ProcData DataInits::data_source;

TEST(PROCESS_CHECKS, CheckForFgNameCall) {

    QString fgProcName = DataInits::data_source.getFgProcessName();
    EXPECT_THAT(
        fgProcName.toStdString(),
        ::testing::HasSubstr("Qt")
    );
}

TEST(PROCESS_CHECKS, CheckTotalProcessTime) {

    auto total_proc_time = DataInits::data_source.getTotalProcessTime();
    EXPECT_GT(total_proc_time, 0);
}

TEST(PROCESS_CHECKS, CheckFgProcMemory) {
    auto fg_proc_mem = DataInits::data_source.getFgProcessMemory();
    EXPECT_GT(fg_proc_mem, 0);
}

// Strictly making sure this doesn't error out since it makes several c-api calls
TEST(PROCESS_CHECKS, CheckGpuPaths) {
    double gpu_utilization = DataInits::data_source.getFgProcessGpuUsage();
    ASSERT_TRUE(true);
}