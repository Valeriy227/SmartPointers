#pragma once

#include <memory>
#include "sw_fwd.h"  // Forward declaration
#include "shared.h"

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() {
    }

    template <typename R>
    WeakPtr(const WeakPtr<R>& other) : block_(other.block_), ptr_(other.ptr_) {
        if (block_) {
            block_->weak_++;
        }
    }
    WeakPtr(const WeakPtr& other) : block_(other.block_), ptr_(other.ptr_) {
        if (block_) {
            block_->weak_++;
        }
    }
    WeakPtr(WeakPtr&& other) : block_(std::move(other.block_)), ptr_(std::move(other.ptr_)) {
        other.block_ = nullptr;
        other.ptr_ = nullptr;
    }
    template <typename R>
    WeakPtr(WeakPtr<R>&& other) : block_(std::move(other.block_)), ptr_(std::move(other.ptr_)) {
        other.block_ = nullptr;
        other.ptr_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) : block_(other.block_), ptr_(other.ptr_) {
        if (block_) {
            block_->weak_++;
        }
    }
    template<class P>
    WeakPtr(const SharedPtr<P>& other) : block_(other.block_), ptr_(other.ptr_) {
        if (block_) {
            block_->weak_++;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (block_ == other.block_) {
            return *this;
        }

        Reset();

        block_ = other.block_;
        if (block_) {
            block_->weak_++;
        }
        ptr_ = other.ptr_;

        return *this;
    }
    template <class R>
    WeakPtr& operator=(const WeakPtr<R>& other) {
        if (block_ == other.block_) {
            return *this;
        }

        Reset();

        block_ = other.block_;
        if (block_) {
            block_->weak_++;
        }
        ptr_ = other.ptr_;

        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) {
        if (block_ == other.block_) {
            return *this;
        }

        Reset();

        block_ = std::move(other.block_);
        ptr_ = std::move(other.ptr_);

        other.block_ = nullptr;
        other.ptr_ = nullptr;

        return *this;
    }
    template <class R>
    WeakPtr& operator=(WeakPtr<R>&& other) {
        if (block_ == other.block_) {
            return *this;
        }

        Reset();

        block_ = std::move(other.block_);
        ptr_ = std::move(other.ptr_);

        other.block_ = nullptr;
        other.ptr_ = nullptr;

        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (block_) {
            block_->weak_--;
            if (block_->weak_ + block_->strong_ == 0) {
                delete block_;
            }
        }

        block_ = nullptr;
        ptr_ = nullptr;
    }
    void Swap(WeakPtr& other) {
        std::swap(block_, other.block_);
        std::swap(ptr_, other.ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (block_) {
            return block_->strong_;
        }
        return 0;
    }
    bool Expired() const {
        return !(block_ && block_->strong_ > 0);
    }
    SharedPtr<T> Lock() const {
        if (Expired()) {
            return SharedPtr<T>();
        }
        return SharedPtr<T>(*this);
    }

    // Fields
    BlockBase* block_ = nullptr;
    T* ptr_ = nullptr;
};

// template <class T>
// SharedPtr<T>::SharedPtr(const WeakPtr<T>& other) : block_(other.block_), ptr_(other.ptr_) {
//     if (block_) {
//         block_->strong_++;
//     }
// }
