// lambda.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <sstream>
#include <algorithm>

/*
    lambda函数用途：
    1、作为回调或者函数参数的一个small function
    2、在某个作用域（比如一个类的某个成员函数）内创建一个局部的匿名函数，可在此作用域内被多次调用，但对外部不可见
    lambda函数可以很灵活的访问外部的局部对象，或者类成员对象(通过this)：
    [] : 不能访问任何外部对象
    [=, &a] : 通过引用访问a，其他的都通过值访问
*/

int main()
{
    //用途1
    auto Integer2Hex = [](int arg) ->  std::string {
        std::stringstream stream;
        stream << std::hex << arg;
        return stream.str();
    };
    int arg = 1024;
    std::string hex = Integer2Hex(arg);
    std::cout << arg << "=0x" << hex << std::endl;

    //用途2
    int nums[] = { 1,2,3,4,5 };
    int flag = 0;
    std::for_each(nums, nums + sizeof(nums) / sizeof(int), [](const int& n) {
        std::cout << n << " ";

        //error: flag is not visible here, it's not in the capture list of this lambda
        //flag = 1;
    });
    std::cout << std::endl << "flag=" << flag << std::endl;

    std::for_each(nums, nums + sizeof(nums) / sizeof(int), [flag](int& n) {
        ++n;
        
        //error: flag is read-only here, it's const and copied by value
        //flag = 2;
    });
    std::cout << std::endl << "flag=" << flag << std::endl;

    std::for_each(nums, nums + sizeof(nums) / sizeof(int), [flag](const int& n) mutable {
        std::cout << n << " ";

        /*
            right: now we can write to flag , because the mutable keyword is added.
            But their modification will not affect value of outer scope variables, because they are captured by value.
            That is, flag is no longer defined const.
        */
        flag = 3;
    });
    std::cout << std::endl << "flag=" << flag << std::endl;

    std::for_each(nums, nums + sizeof(nums) / sizeof(int), [&flag](const int& n) mutable {
        std::cout << n << " ";

        /*
            right: we can both read from and write to flag.
            And its modification will affect the value of outer scope flag.
        */
        flag = 3;
    });
    std::cout << std::endl << "flag=" << flag << std::endl;

    return 0;
}

