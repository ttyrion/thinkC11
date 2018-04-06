// lambda.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <map>
#include <functional>

/*
    lambda函数用途：
    1、作为回调或者函数参数的一个small function
        后面把lambda和bind对象作为shared_ptr的自定义deleter的例子。

    2、在某个作用域（比如一个类的某个成员函数）内创建一个局部的匿名函数，可在此作用域内被多次调用，但对外部不可见
    lambda函数可以很灵活的访问外部的局部对象，或者类成员对象(通过this)：
    [] : 不能访问任何外部对象
    [=, &a] : 通过引用访问a，其他的都通过值访问
*/

class Sample {
public:
    Sample() {
    }
    ~Sample() {
        std::cout << "sample" << " destructed." << std::endl;
    }
};

template<typename T>
class SamplePool {
public:
    ~SamplePool() {
        for (auto s : samples_) {
            delete s.first;
        }

        samples_.clear();
    }

    T* Aquire() {
        for (auto s : samples_) {
            if (s.second) {
                std::cout << "Pool has reusable Sample: " << std::hex << s.first << std::endl;
                return s.first;
            }
        }

        T* t = new T();
        samples_[t] = false;
        return t;
    }

    void PushBack(T* t) {
        std::cout << "Sample " << std::hex << t << " pushed back." << std::endl;
        samples_[t] = true;
    }

    void Print() {
        /*
            因为map的key是不可修改的，所以下面前两个写法都是错误的，特别注意第二个，也是错的
        */
        //wrong
        //std::for_each(samples_.begin(), samples_.end(), [](std::pair<T*, bool>& p) {
        //    std::cout << "Sample " << std::hex << p.first << " available=" << p.second << "." << std::endl;
        //});

        //wrong : 
        //std::for_each(samples_.begin(), samples_.end(), [](std::pair<const T*, bool>& p) {
        //    std::cout << "Sample " << std::hex << p.first << " available=" << p.second << "." << std::endl;
        //});

        //right!
        std::for_each(samples_.begin(), samples_.end(), [](const std::pair<T const*, bool>& p) {
            //std::cout << "Sample " << std::hex << p.first << " available=" << p.second << "." << std::endl;
        });

        std::for_each(samples_.begin(), samples_.end(), [](auto& p) {
            std::cout << "Sample " << std::hex << p.first << " available = " << p.second << "." << std::endl;
        });
    }

private:
    std::map<T*, bool> samples_;
};

int main()
{
    //用途2
    auto Integer2Hex = [](int arg) ->  std::string {
        std::stringstream stream;
        stream << std::hex << arg;
        return stream.str();
    };
    int arg = 1024;
    std::string hex = Integer2Hex(arg);
    std::cout << arg << "=0x" << hex << std::endl;
    std::cout << "***************************" << std::endl;

    //用途1
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
    std::cout << "***************************" << std::endl;

    //给shared_ptr指定一个deleter
    /*
        问题：shared_ptr对象出了作用域后，会把内部引用计数减一，接着判断： 
        if new value of reference count is 0 then it deletes the associated raw pointer calling delete operator.
        But sometimes we want delete[] operator ,but not the delete.
        这种情况下，我们就要给shared_ptr提供一个自定义的deleter: it could be a funciton, functor, or lambda。
    */
    std::shared_ptr<Sample> ps(new Sample[10], [](Sample *x) {
        delete[] x;
    });

    //另外一种情况是，可能不希望shared_ptr释放内存
    std::shared_ptr<SamplePool<Sample>> pool = std::make_shared<SamplePool<Sample>>();
    std::cout << "当前内存池情况：" << std::endl;
    pool->Print();
    std::shared_ptr<Sample> shared_ps(pool->Aquire(), std::bind(&SamplePool<Sample>::PushBack, pool, std::placeholders::_1));
    std::cout << "当前内存池情况：" << std::endl;
    Sample* s1 = pool->Aquire();

    //
    Sample* s2 = pool->Aquire();
    pool->PushBack(s1);
    s1 = nullptr;
    //s3可以重用pool内的对象了(之前被推回pool内的s1)
    Sample* s3 = pool->Aquire();

    std::cout << "当前内存池情况：" << std::endl;
    pool->Print();

    return 0;
}

