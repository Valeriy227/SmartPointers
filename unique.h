#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t
#include <memory>
#include <type_traits>
#include <utility>

struct Slug {
    template <typename T>
    void operator()(T* obj) {
        if (obj != nullptr) {
            obj->~T();
            delete obj;
        }
    }
};

// Primary template
template <typename T, typename Deleter = std::default_delete<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    UniquePtr() {
        pair_.GetFirst() = nullptr;
    }
    template <typename R>
    explicit UniquePtr(R* ptr) : pair_(ptr, Deleter()) {
    }
    //    template<typename R, typename Del2>
    //    UniquePtr(R *ptr, Del2& deleter) {
    //        pair_.GetFirst() = ptr;
    //        pair_.GetSecond() = deleter;
    //    }
    //    template<typename R, typename Del2>
    //    UniquePtr(R *ptr, Del2&& deleter) {
    //        pair_.GetFirst() = ptr;
    //        pair_.GetSecond() = std::forward<>(deleter);
    //    }
    template <typename R, typename Del2>
    UniquePtr(R* ptr, Del2&& deleter) : pair_(ptr, std::forward<Del2>(deleter)) {
    }
    template <typename R, typename Del2>
    UniquePtr(R* ptr, const Del2& deleter) : pair_(ptr, deleter) {
    }
    //  template<typename R, typename Del2>
    //  UniquePtr(R *ptr, const Del2& deleter) {
    //      pair_.GetFirst() = ptr;
    //      pair_.GetSecond() = deleter;
    //  }

<<<<<<< Updated upstream
    explicit UniquePtr(T* ptr = nullptr);
    UniquePtr(T* ptr, Deleter deleter);
=======
    template <typename Del2>
    UniquePtr(std::nullptr_t ptr, const Del2& deleter) : pair_(ptr, deleter) {
    }
    template <typename Del2>
    UniquePtr(std::nullptr_t ptr, Del2&& deleter) : pair_(ptr, std::forward<Del2>(deleter)) {
    }
    //     template<typename R>
    //     UniquePtr(R *ptr, const Deleter& deleter) {
    //         pair_.GetFirst() = ptr;
    //         pair_.GetSecond() = std::forward<Deleter>(deleter);
    //     }
>>>>>>> Stashed changes

    template <typename R, typename Del2>
    UniquePtr(UniquePtr<R, Del2>&& other) noexcept
        : pair_(other.pair_.GetFirst(), std::forward<Del2>(other.pair_.GetSecond())) {
        other.pair_.GetFirst() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    template <typename R, typename Del2>
    UniquePtr& operator=(UniquePtr<R, Del2>&& other) noexcept {
        if (other.pair_.GetFirst() == pair_.GetFirst()) {
            return *this;
        }

        if (pair_.GetFirst() != nullptr) {
            pair_.GetSecond()(pair_.GetFirst());
            pair_.GetFirst() = nullptr;
        }

        pair_ = std::forward<decltype(other.pair_)>(other.pair_);
        other.pair_.GetFirst() = nullptr;
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        if (pair_.GetFirst() != nullptr) {
            pair_.GetSecond()(pair_.GetFirst());
        }
        pair_.GetFirst() = nullptr;
        return *this;
    }
    UniquePtr(UniquePtr&) = delete;
    UniquePtr& operator=(UniquePtr&) = delete;
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        if (pair_.GetFirst() != nullptr) {
            pair_.GetSecond()(pair_.GetFirst());
        }
        pair_.GetFirst() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        auto res = pair_.GetFirst();
        pair_.GetFirst() = nullptr;
        return res;
    }
    void Reset(T* ptr = nullptr) {
        if (pair_.GetFirst() == ptr) {
            return;
        }
        auto tmp = pair_.GetFirst();
        pair_.GetFirst() = ptr;
        if (tmp != nullptr) {
            pair_.GetSecond()(tmp);
        }
    }
    template <typename R, typename Del2>
    void Swap(UniquePtr<R, Del2>& other) {
        CompressedPair<T*, Deleter> tmp = std::move(pair_);
        pair_ = std::move(other.pair_);
        other.pair_ = std::move(tmp);

        // auto tmp2 = pair_.GetSecond();
        // pair_.GetSecond() = other.pair_.GetSecond();
        // other.pair_.GetSecond() = tmp2;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return pair_.GetFirst();
    }
    Deleter& GetDeleter() {
        return pair_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return pair_.GetSecond();
    }
    explicit operator bool() const {
        return pair_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    typename std::add_lvalue_reference<T>::type operator*() const {
        return *pair_.GetFirst();
    }
    T* operator->() const {
        return pair_.GetFirst();
    }

public:
    CompressedPair<T*, Deleter> pair_;
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    UniquePtr() {
        pair_.GetFirst() = nullptr;
    }
    template <typename R>
    explicit UniquePtr(R* ptr) : pair_(ptr, Deleter()) {
    }
    template <typename R, typename Del2>
    UniquePtr(R* ptr, Del2&& deleter) : pair_(ptr, std::forward<Del2>(deleter)) {
    }
    template <typename R, typename Del2>
    UniquePtr(R* ptr, const Del2& deleter) : pair_(ptr, deleter) {
    }

    template <typename Del2>
    UniquePtr(std::nullptr_t ptr, const Del2& deleter) : pair_(ptr, deleter) {
    }
    template <typename Del2>
    UniquePtr(std::nullptr_t ptr, Del2&& deleter) : pair_(ptr, std::forward<Del2>(deleter)) {
    }

    template <typename R, typename Del2>
    UniquePtr(UniquePtr<R, Del2>&& other) noexcept
        : pair_(other.pair_.GetFirst(), std::forward<Del2>(other.pair_.GetSecond())) {
        other.pair_.GetFirst() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    template <typename R, typename Del2>
    UniquePtr& operator=(UniquePtr<R, Del2>&& other) noexcept {
        if (other.pair_.GetFirst() == pair_.GetFirst()) {
            return *this;
        }

        if (pair_.GetFirst() != nullptr) {
            pair_.GetSecond()(pair_.GetFirst());
            pair_.GetFirst() = nullptr;
        }

        pair_ = std::forward<decltype(other.pair_)>(other.pair_);
        other.pair_.GetFirst() = nullptr;
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        if (pair_.GetFirst() != nullptr) {
            pair_.GetSecond()(pair_.GetFirst());
        }
        pair_.GetFirst() = nullptr;
        return *this;
    }
    UniquePtr(UniquePtr&) = delete;
    UniquePtr& operator=(UniquePtr&) = delete;
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        if (pair_.GetFirst() != nullptr) {
            pair_.GetSecond()(pair_.GetFirst());
        }
        pair_.GetFirst() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        auto res = pair_.GetFirst();
        pair_.GetFirst() = nullptr;
        return res;
    }
    void Reset(T* ptr = nullptr) {
        if (pair_.GetFirst() == ptr) {
            return;
        }
        auto tmp = pair_.GetFirst();
        pair_.GetFirst() = ptr;
        if (tmp != nullptr) {
            pair_.GetSecond()(tmp);
        }
    }
    template <typename R, typename Del2>
    void Swap(UniquePtr<R, Del2>& other) {
        CompressedPair<T*, Deleter> tmp = std::move(pair_);
        pair_ = std::move(other.pair_);
        other.pair_ = std::move(tmp);

        // auto tmp2 = pair_.GetSecond();
        // pair_.GetSecond() = other.pair_.GetSecond();
        // other.pair_.GetSecond() = tmp2;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return pair_.GetFirst();
    }
    Deleter& GetDeleter() {
        return pair_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return pair_.GetSecond();
    }
    explicit operator bool() const {
        return pair_.GetFirst() != nullptr;
    }

    T& operator[](int ind) const {
        return pair_.GetFirst()[ind];
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    typename std::add_lvalue_reference<T>::type operator*() const {
        return *pair_.GetFirst();
    }
    T* operator->() const {
        return pair_.GetFirst();
    }

public:
    CompressedPair<T*, Deleter> pair_;
};