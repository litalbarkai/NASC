/*******************************************************************************
 * Project:     Utilities
 * Version:     1.0 - 19/10/2023
*******************************************************************************/
#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <string>       //std::string
#include <system_error> //std::error_code, generic_category
#include <iostream>

#include <cstring>      //std::strerror

namespace Infrastructure
{
template <typename E>
void ThrowIfBad(bool is_good_, std::error_code err_, std::string e_msg_)
{
    if (!is_good_)
    {
        std::cout << e_msg_ << std::endl;
        throw E(err_, e_msg_);
    }
}

template <typename E>
void ThrowIfBad(bool is_good_, int errno_)
{
    if (!is_good_)
    {
        std::cout << std::strerror(errno_) << std::endl;
        throw E(std::error_code(errno_, std::generic_category()),
                                            std::string(std::strerror(errno_)));
    }
}

template <typename E>
void ThrowIfBad(bool is_good_, const E& e_)
{
    if (!is_good_)
    {
        std::cout << e_.what() << std::endl;
        throw e_;
    }
}
} // Infrastructure

#endif // __UTILS_HPP__