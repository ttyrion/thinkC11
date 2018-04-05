// lambda.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <iostream>
#include <sstream>
#include <algorithm>

/*
    lambda������;��
    1����Ϊ�ص����ߺ���������һ��small function
    2����ĳ�������򣨱���һ�����ĳ����Ա�������ڴ���һ���ֲ����������������ڴ��������ڱ���ε��ã������ⲿ���ɼ�
    lambda�������Ժ����ķ����ⲿ�ľֲ����󣬻������Ա����(ͨ��this)��
    [] : ���ܷ����κ��ⲿ����
    [=, &a] : ͨ�����÷���a�������Ķ�ͨ��ֵ����
*/

int main()
{
    //��;1
    auto Integer2Hex = [](int arg) ->  std::string {
        std::stringstream stream;
        stream << std::hex << arg;
        return stream.str();
    };
    int arg = 1024;
    std::string hex = Integer2Hex(arg);
    std::cout << arg << "=0x" << hex << std::endl;

    //��;2
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

