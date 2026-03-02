#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "procdata.h"
#include "historybuffer.h"
#include "sample_point.h"

class DataInits : public ::testing::Environment {
public:
public:
    static ProcData data_source;
};

ProcData DataInits::data_source;

std::string separate_numbers(int64_t number) {

    constexpr int grouping = 1000;
    constexpr int double_digit_threshold = 100;
    constexpr int single_digit_threshold = 10;

    // Number fillers
    constexpr char delim = ',';
    constexpr char double_digit_filler[] = {delim, '0', '0', '\n'};
    constexpr char single_digit_filler[] = {delim, '0', '\n'};

    if (number < 1)
        return "0";

    int current_digits = number % grouping;
    std::string num_string = std::to_string(current_digits);
    number /= grouping;

    if (number == 0) {
        return num_string;
    } else {

        if (number < single_digit_threshold) {
            return separate_numbers(number) + single_digit_filler + num_string;
        }
        else if (number < double_digit_threshold) {
            return separate_numbers(number) + double_digit_filler + num_string;
        }
        else {
            return separate_numbers(number) + delim + num_string;
        }
    }
}

TEST(PROCESS_CHECKS, CheckForFgNameCall) {

    std::wstring fgProcName = DataInits::data_source.getFgProcessName();
    EXPECT_GT(fgProcName.size(), 0);
}

TEST(PROCESS_CHECKS, CheckTotalProcessTime) {
    auto total_proc_time = DataInits::data_source.getTotalProcessTime();
    EXPECT_GT(total_proc_time, 0);
}

TEST(PROCESS_CHECKS, CheckFgProcTime) {
    auto fg_proc_time = DataInits::data_source.getTotalProcessTime();
    EXPECT_GT(fg_proc_time, 0);
}

TEST(PROCESS_CHECKS, CheckFgProcMemory) {
    auto fg_proc_mem = DataInits::data_source.getFgProcessMemory();
    EXPECT_GT(fg_proc_mem, 0);
}

TEST(PROCESS_CHECKS, CheckBothCpuCalls){

    auto total_time = DataInits::data_source.getTotalCpuTime();
    auto fg_time = DataInits::data_source.getTotalProcessTime();

    EXPECT_GE(total_time, fg_time) <<
        "Total CPU time = " <<
        separate_numbers(total_time) <<
        "\nForeG CPU time = " <<
        separate_numbers(fg_time);
}

TEST(STATIC_FUNC_CHECKS, GetLastPathVerify) {

    auto qtc_path_example = L"C:\\Qt\\bin\\qtcreator.exe";
    auto short_path = L"C:\\a.exe";

    std::wstring qtc_name = DataInits::data_source.getLastPathItem(qtc_path_example, std::wcslen(qtc_path_example));
    std::wstring short_name = DataInits::data_source.getLastPathItem(short_path, std::wcslen(short_path));

    EXPECT_STREQ(L"qtcreator.exe", qtc_name.data());
}

TEST(CIRC_BUFFER_TEST, BasicInit) {

    constexpr int buff_limit = 20;
    constexpr int loop_limit = (buff_limit * 3) / 2;

    HWOverlay::HistoryBuffer<buff_limit> hbuff;
    
    for (auto a = 0; a < loop_limit; a++) {
        hbuff.add(a);
    }
    
    std::vector<HWOverlay::SamplePoint> sample = hbuff.get_linear_buffer();
    HWOverlay::SamplePoint latest = *(sample.rbegin());

    ASSERT_DOUBLE_EQ(30.0, latest.measurement);
}
