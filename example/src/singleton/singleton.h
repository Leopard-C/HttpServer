/**
 * @file singleton.h
 * @brief 单例类.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2021-present Jinbao Chen
 */
#ifndef IC_SINGLETON_SINGLETON_H_
#define IC_SINGLETON_SINGLETON_H_
#include <cstdio>
#include <cstdlib>
#include <utility>
#include <string>
#ifdef __GNUC__
#  include <cxxabi.h>
#endif

namespace ic {

/**
 * @brief 单例类：保证每个类仅有一个实例，并为它提供一个全局访问点.
 */
template<typename T>
class Singleton {
public:
    /**
     * @brief 初始化唯一的实例.
     * 
     * @warning 必须首先调用该方法，初始化一个实例
     */
    template<typename ...Args>
    static T* Instance(Args&&... args) {
        if (!s_instance) {
            s_instance = new T(std::forward<Args>(args)...);
        }
        return s_instance;
    }

    /**
     * @brief 获取实例.
     * 
     * @warning 必须已经调用过Instance()后，才能调用该方法
     */
    static T* GetInstance() {
        if (!s_instance) {
            printf(
                "************************ [WARNING] ************************\n"
                "     Not instantiated before GetInstance()\n"
                "     typename T = <%s>\n"
                "***********************************************************\n",
                type_name().c_str()
            );
        }
        return s_instance;
    }

    /**
     * @brief 主动销毁实例.
     */
    static void DesInstance() {
        if (s_instance) {
            delete s_instance;
            s_instance = nullptr;
        }
    }

private:
    /**
     * @brief 获取模板类型的名称
     */
    static std::string type_name() {
        std::string tname = typeid(T).name();
    #ifdef __GNUC__
        int status;
        char* demangled_name = abi::__cxa_demangle(tname.c_str(), NULL, NULL, &status);
        if(status == 0) {
            tname = demangled_name;
            std::free(demangled_name);
        }
    #endif
        return tname;
    }

private:
    explicit Singleton();
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    ~Singleton() { DesInstance(); }

private:
    static T* s_instance;
};

template<typename T>
T* Singleton<T>::s_instance = nullptr;

} // namespace ic

#endif // IC_SINGLETON_SINGLETON_H_
