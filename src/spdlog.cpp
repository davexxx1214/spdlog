// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef SPDLOG_COMPILED_LIB
    #error Please define SPDLOG_COMPILED_LIB to compile this file.
#endif

#include <spdlog/common-inl.h>
#include <spdlog/details/backtracer-inl.h>
#include <spdlog/details/log_msg-inl.h>
#include <spdlog/details/log_msg_buffer-inl.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/os-inl.h>
#include <spdlog/details/registry-inl.h>
#include <spdlog/logger-inl.h>
#include <spdlog/pattern_formatter-inl.h>
#include <spdlog/sinks/base_sink-inl.h>
#include <spdlog/sinks/sink-inl.h>
#include <spdlog/spdlog-inl.h>

#include <mutex>

// 添加未使用的变量 - 会产生未使用变量警告
static int unused_variable = 42;

// 添加隐式类型转换 - 会产生类型转换警告
void generate_warning_function() {
    double pi = 3.14159;
    int rounded_pi = pi;  // 警告：从 double 到 int 的隐式转换可能丢失数据
    
    // 未使用的变量 - 会产生未使用变量警告
    int another_unused = 100;
    
    // 比较有符号和无符号类型 - 会产生符号比较警告
    unsigned int positive = 10;
    int negative = -5;
    if (negative < positive) {  // 警告：比较有符号和无符号整数
        // 空语句 - 可能产生空语句警告
        ;
    }
    
    // 未初始化变量 - 可能产生未初始化警告
    int uninitialized;
    if (rounded_pi > 3) {
        uninitialized = 10;
    }
    // 使用可能未初始化的变量
    int result = uninitialized + 5;
    
    // 防止编译器优化掉上面的代码
    (void)result;
}

// 添加一个被声明但未定义的函数 - 会产生未定义函数警告
void undefined_function();

// 添加一个调用未定义函数的函数
void call_undefined() {
    // 注释掉实际调用，以避免链接错误，但保留代码以产生警告
    // undefined_function();
    
    // 改为条件调用，这样编译器会发出警告但不会导致链接错误
    if (false) {
        undefined_function();
    }
}

// 添加一个弃用的函数 - 会产生使用弃用函数的警告
#ifdef _MSC_VER
    __declspec(deprecated) void deprecated_function() {}
#else
    [[deprecated]] void deprecated_function() {}
#endif

// 添加一个调用弃用函数的函数
void call_deprecated() {
    deprecated_function();
}

// 添加一个有潜在除零风险的函数 - 可能产生除零警告
int divide_by_parameter(int divisor) {
    return 100 / divisor;  // 可能的除零警告
}

// 添加一个有数组越界风险的函数 - 可能产生数组越界警告
void array_bounds_warning() {
    int arr[5] = {1, 2, 3, 4, 5};
    // 下面这行在某些编译器会产生数组越界警告
    arr[5] = 6;  // 越界访问
}

// 添加一个有符号转换警告的函数
void sign_conversion_warning() {
    int negative = -1;
    unsigned int positive = negative;  // 警告：负值转换为无符号类型
}

// 在文件末尾调用这些函数，确保它们被编译
void trigger_warnings() {
    generate_warning_function();
    call_undefined();
    call_deprecated();
    divide_by_parameter(1);  // 不会实际产生除零，但编译器可能会警告
    array_bounds_warning();
    sign_conversion_warning();
}

// template instantiate logger constructor with sinks init list
template SPDLOG_API spdlog::logger::logger(std::string name,
                                           sinks_init_list::iterator begin,
                                           sinks_init_list::iterator end);
template class SPDLOG_API spdlog::sinks::base_sink<std::mutex>;
template class SPDLOG_API spdlog::sinks::base_sink<spdlog::details::null_mutex>;

// 添加一个main函数的声明但不定义它 - 可能产生警告
// 注释掉以避免链接错误
// int main();
