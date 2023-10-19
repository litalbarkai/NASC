/*******************************************************************************
 * Project:     Singleton
 * Version:     1.0 - 18/10/2023
*******************************************************************************/
#ifndef __SINGLETON_HPP__
#define __SINGLETON_HPP__

#include <system_error> //std::system_error
#include <atomic>       //syd::atomic, atomic_thread_fence
#include <mutex>        //std::mutex, lock_guard
#include <cstdlib>      //std::atexit

namespace Infrastructure
{
template <typename T>
class Singleton
{
public:
    static T* GetInstance();

private:
    // disable construction by user, make uncopyable
    explicit Singleton() =delete;
    Singleton(const Singleton&) =delete;
    Singleton& operator=(const Singleton&) =delete;

    static T* s_pInstance;
    static std::mutex s_alloc_mutex;

    static void Cleanup();
};

template <typename T>
T* Singleton<T>::s_pInstance = nullptr;

template <typename T>
std::mutex Singleton<T>::s_alloc_mutex;

// static
template <typename T>
T* Singleton<T>::GetInstance()
{
    T* tmp = s_pInstance;
    
    std::atomic_thread_fence(std::memory_order_acquire);
    if (nullptr == tmp)
    {
        const std::lock_guard<std::mutex> lock(s_alloc_m);
        
        tmp = s_pInstance;
        if (nullptr == tmp)
        {
            int status = std::atexit(Cleanup);
            ThrowIfBad<std::system_error>(0 == status,
                            std::make_error_code(std::errc::invalid_argument),
                                            "failed to ensure thread safety");
            
            tmp = new T;

            std::atomic_thread_fence(std::memory_order_release);
            s_pInstance = tmp;
        }
    }
    
    return tmp;
}

// static
template <typename T>
void Singleton<T>::Cleanup()
{
    const std::lock_guard<std::mutex> lock(s_alloc_m);

    delete s_pInstance;
    s_pInstance = nullptr;
}
} // Infrastructure

#endif // __SINGLETON_HPP__