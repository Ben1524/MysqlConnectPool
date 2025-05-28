//
// Created by cxk_zjq on 25-5-27.
//

#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

/**
 *
 *  @file NonCopyable.h
 */

#pragma once

namespace cxk
{
    /**
     * @brief This class represents a non-copyable object.
     *
     */
    class  NonCopyable
    {
    protected:
        NonCopyable()
        {
        }
        ~NonCopyable()
        {
        }
        NonCopyable(const NonCopyable &) = delete;
        NonCopyable &operator=(const NonCopyable &) = delete;
        // some uncopyable classes maybe support move constructor....
        NonCopyable(NonCopyable &&) noexcept(true) = default;
        NonCopyable &operator=(NonCopyable &&) noexcept(true) = default;
    };

}  // namespace trantor


#endif //NONCOPYABLE_H
