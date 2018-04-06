// lambda.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <map>
#include <functional>

/*
    lambda������;��
    1����Ϊ�ص����ߺ���������һ��small function
        �����lambda��bind������Ϊshared_ptr���Զ���deleter�����ӡ�

    2����ĳ�������򣨱���һ�����ĳ����Ա�������ڴ���һ���ֲ����������������ڴ��������ڱ���ε��ã������ⲿ���ɼ�
    lambda�������Ժ����ķ����ⲿ�ľֲ����󣬻������Ա����(ͨ��this)��
    [] : ���ܷ����κ��ⲿ����
    [=, &a] : ͨ�����÷���a�������Ķ�ͨ��ֵ����
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
            ��Ϊmap��key�ǲ����޸ĵģ���������ǰ����д�����Ǵ���ģ��ر�ע��ڶ�����Ҳ�Ǵ��
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
    //��;2
    auto Integer2Hex = [](int arg) ->  std::string {
        std::stringstream stream;
        stream << std::hex << arg;
        return stream.str();
    };
    int arg = 1024;
    std::string hex = Integer2Hex(arg);
    std::cout << arg << "=0x" << hex << std::endl;
    std::cout << "***************************" << std::endl;

    //��;1
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

    //��shared_ptrָ��һ��deleter
    /*
        ���⣺shared_ptr�������������󣬻���ڲ����ü�����һ�������жϣ� 
        if new value of reference count is 0 then it deletes the associated raw pointer calling delete operator.
        But sometimes we want delete[] operator ,but not the delete.
        ��������£����Ǿ�Ҫ��shared_ptr�ṩһ���Զ����deleter: it could be a funciton, functor, or lambda��
    */
    std::shared_ptr<Sample> ps(new Sample[10], [](Sample *x) {
        delete[] x;
    });

    //����һ������ǣ����ܲ�ϣ��shared_ptr�ͷ��ڴ�
    std::shared_ptr<SamplePool<Sample>> pool = std::make_shared<SamplePool<Sample>>();
    std::cout << "��ǰ�ڴ�������" << std::endl;
    pool->Print();
    std::shared_ptr<Sample> shared_ps(pool->Aquire(), std::bind(&SamplePool<Sample>::PushBack, pool, std::placeholders::_1));
    std::cout << "��ǰ�ڴ�������" << std::endl;
    Sample* s1 = pool->Aquire();

    //
    Sample* s2 = pool->Aquire();
    pool->PushBack(s1);
    s1 = nullptr;
    //s3��������pool�ڵĶ�����(֮ǰ���ƻ�pool�ڵ�s1)
    Sample* s3 = pool->Aquire();

    std::cout << "��ǰ�ڴ�������" << std::endl;
    pool->Print();

    return 0;
}

