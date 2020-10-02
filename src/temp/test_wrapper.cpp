#include <iostream>
#include "print_func.hpp"
#include "unit_testing.hpp"

extern "C"
{
#include "new.h"
}


int main()
{

UNIT_TESTING_START;

using std::vector;
using std::string;
using out_t = Out::VecStr;
out_t out;
string sp(2, ' '); /* line space */
string es(3, ' '); /* empty shift */

TEST(wrapper, printWrapperNoActions)
{
    out_t check = {};
    trt_printing p = {&out, Out::print_VecStr};
    trt_wrapper wr = trp_init_wrapper_top();
    trp_print_wrapper(wr, p);
    EXPECT_EQ(out, check);
    out.clear();
}

TEST(wrapper, printWrapperSet)
{
    out_t check = {sp, "|"};
    trt_printing p = {&out, Out::print_VecStr};
    trt_wrapper wr = trp_wrapper_set_mark(trp_init_wrapper_top());
    trp_print_wrapper(wr, p);
    EXPECT_EQ(out, check);
    out.clear();
}

TEST(wrapper, printWrapperShiftSet)
{
    out_t check = {sp + es, "|"};
    trt_printing p = {&out, Out::print_VecStr};
    trt_wrapper wr = trp_wrapper_set_mark(trp_wrapper_set_shift(trp_init_wrapper_top()));
    trp_print_wrapper(wr, p);
    EXPECT_EQ(out, check);
    out.clear();
}

TEST(wrapper, printWrapperSetShift)
{
    out_t check = {sp, "|", es};
    trt_printing p = {&out, Out::print_VecStr};
    trt_wrapper wr = trp_wrapper_set_shift(trp_wrapper_set_mark(trp_init_wrapper_top()));
    trp_print_wrapper(wr, p);
    EXPECT_EQ(out, check);
    out.clear();
}

TEST(wrapper, printWrapperSetShiftSet)
{
    out_t check = {sp, "|" , sp, "|"};
    trt_printing p = {&out, Out::print_VecStr};
    trt_wrapper wr = trp_wrapper_set_mark(trp_wrapper_set_shift(trp_wrapper_set_mark(trp_init_wrapper_top())));
    trp_print_wrapper(wr, p);
    EXPECT_EQ(out, check);
    out.clear();
}

TEST(wrapper, printWrapperShiftShiftSet)
{
    out_t check = {sp + es + es, "|"};
    trt_printing p = {&out, Out::print_VecStr};
    trt_wrapper wr = trp_wrapper_set_mark(trp_wrapper_set_shift(trp_wrapper_set_shift(trp_init_wrapper_top())));
    trp_print_wrapper(wr, p);
    EXPECT_EQ(out, check);
    out.clear();
}


PRINT_TESTS_STATS();

UNIT_TESTING_END;

}
