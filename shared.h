#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <memory>
#include <type_traits>
#include <utility>

class BlockBase {
public:
    BlockBase() : strong_(1) {
    }
    virtual ~BlockBase() = default;
    virtual void Destruct() {
    }

    int strong_ = 0;
    int weak_ = 0;
};

template <typename T>
class BlockObject : public BlockBase {
public:
    template <class... Args>
    BlockObject(Args&&... args) {
        new (&data_) T(std::forward<Args>(args)...);
    }
    ~BlockObject() override {
        // reinterpret_cast<T*>(&data_)->~T();
    }
    void Destruct() override {
        reinterpret_cast<T*>(&data_)->~T();
    }
    T* GetPtr() {
        return reinterpret_cast<T*>(&data_);
    }

    std::aligned_storage_t<sizeof(T), alignof(T)> data_;
};

template <typename T>
class BlockPtr : public BlockBase {
public:
    template <typename R>
    BlockPtr(R* ptr) : BlockBase(), ptr_(ptr) {
    }
    ~BlockPtr() override {
        // delete ptr_;
    }
    void Destruct() override {
        delete ptr_;
    }

    T* ptr_;
};

template <typename T>
class WeakPtr;

class WhoAmI;

template <class T>
class EnableSharedFromThis;

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() {
    }
    SharedPtr(std::nullptr_t) {
    }
    template <typename R>
    // explicit SharedPtr(R* ptr) : block_(new BlockPtr<R>(ptr)), ptr_(ptr) {
    explicit SharedPtr(R* ptr) : block_(new BlockPtr<R>(ptr)), ptr_(ptr) {
        EnableThis(ptr);
    }

    template <typename R>
    SharedPtr(const SharedPtr<R>& other) : block_(other.block_), ptr_(other.ptr_) {
        if (block_) {
            block_->strong_++;
        }
    }
    SharedPtr(const SharedPtr& other) : block_(other.block_), ptr_(other.ptr_) {
        if (block_) {
            block_->strong_++;
        }
    }
    SharedPtr(SharedPtr&& other) : block_(std::move(other.block_)), ptr_(std::move(other.ptr_)) {
        other.block_ = nullptr;
        other.ptr_ = nullptr;
    }
    template <typename R>
    SharedPtr(SharedPtr<R>&& other) : block_(std::move(other.block_)), ptr_(std::move(other.ptr_)) {
        other.block_ = nullptr;
        other.ptr_ = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
<<<<<<< Updated upstream
    SharedPtr(const SharedPtr<Y>& other, T* ptr);
=======
    SharedPtr(const SharedPtr<Y>& other, T* ptr) : block_(other.block_), ptr_(ptr) {
        if (block_) {
            block_->strong_++;
        }
    }
>>>>>>> Stashed changes

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) : block_(other.block_), ptr_(other.ptr_) {
        if (block_) {
            if (block_->strong_ == 0) {
                throw BadWeakPtr();
            }
            block_->strong_++;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        if (ptr_ == other.ptr_) {
            return *this;
        }

        Reset();

        block_ = other.block_;
        if (block_) {
            block_->strong_++;
        }
        ptr_ = other.ptr_;

        return *this;
    }
    template <class R>
    SharedPtr& operator=(const SharedPtr<R>& other) {
        if (ptr_ == other.ptr_) {
            return *this;
        }

        Reset();

        block_ = other.block_;
        if (block_) {
            block_->strong_++;
        }
        ptr_ = other.ptr_;

        return *this;
    }
    SharedPtr& operator=(SharedPtr&& other) {
        if (ptr_ == other.ptr_) {
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
    SharedPtr& operator=(SharedPtr<R>&& other) {
        if (ptr_ == other.ptr_) {
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

    ~SharedPtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (block_) {
            block_->strong_--;
            if (block_->strong_ == 0) {
                block_->weak_++;
                block_->Destruct();
                block_->weak_--;
                if (block_->weak_ == 0) {
                    delete block_;
                }
            }
        }

        block_ = nullptr;
        ptr_ = nullptr;
    }
    template <class R>
    void Reset(R* ptr) {
        if (ptr_ == ptr) {
            return;
        }

        Reset();

        block_ = new BlockPtr<R>(ptr);
        ptr_ = ptr;
    }
    void Swap(SharedPtr& other) {
        std::swap(block_, other.block_);
        std::swap(ptr_, other.ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_;
    }
    T& operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        if (block_) {
            return block_->strong_;
        }
        return 0;
    }
    explicit operator bool() const {
        return ptr_ != nullptr;
    }

    template <class R>
    void EnableThis(R* ptr) {
        if constexpr (std::is_base_of_v<WhoAmI, R>) {
            if (ptr) {
                OneAnotherEnableThis(ptr);
            }
        }
    }

    template <class R>
    void OneAnotherEnableThis(EnableSharedFromThis<R>* ptr) {
        ptr->wptr_ = *this;
    }

    // Fields
    BlockBase* block_ = nullptr;
    T* ptr_ = nullptr;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.ptr_ == right.ptr_;
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    SharedPtr<T> res;
    auto block = new BlockObject<T>(std::forward<Args>(args)...);
    res.block_ = block;
    res.ptr_ = block->GetPtr();
    res.EnableThis(res.ptr_);
    return res;
}

class WhoAmI {};

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis : WhoAmI {
public:
    SharedPtr<T> SharedFromThis() {
        return wptr_.Lock();
    }
    SharedPtr<const T> SharedFromThis() const {
        return wptr_.Lock();
    }

    WeakPtr<T> WeakFromThis() noexcept {
        return wptr_.Lock();
    }
    WeakPtr<const T> WeakFromThis() const noexcept {
        WeakPtr<const T> res(wptr_);
        return res.Lock();
    }

    WeakPtr<T> wptr_;
};
