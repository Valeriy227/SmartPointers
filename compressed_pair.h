#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

template <typename T, typename B, bool = std::is_empty_v<T> && !std::is_final_v<T>>
class CompressedElement {
public:
    CompressedElement() : val_(T()) {
    }
    template <typename P>
    CompressedElement(P&& val) : val_(std::forward<P>(val)) {
    }
    template <typename P>
    CompressedElement(const P& val) : val_(val) {
    }
    T& Get() {
        return val_;
    }
    const T& Get() const {
        return val_;
    }

private:
    T val_;
};

template <typename T, typename B>
class CompressedElement<T, B, true> : T {
public:
    CompressedElement() : T() {
    }
    template <typename P>
    CompressedElement(P&& val) : T(std::forward<P>(val)) {
    }
    template <typename P>
    CompressedElement(const P& val) : T(val) {
    }
    T& Get() {
        return *this;
    }
    const T& Get() const {
        return *this;
    }
};

template <typename F, typename S>
class CompressedPair : CompressedElement<F, bool>, CompressedElement<S, int> {
public:
    CompressedPair() : CompressedElement<F, bool>(), CompressedElement<S, int>() {
    }
    template <typename P, typename Q>
    CompressedPair(P&& first, Q&& second)
        : CompressedElement<F, bool>(std::forward<P>(first)),
          CompressedElement<S, int>(std::forward<Q>(second)) {
    }
    template <typename P, typename Q>
    CompressedPair(const P& first, Q&& second)
        : CompressedElement<F, bool>(first),
          CompressedElement<S, int>(std::forward<Q>(second)) {
    }
    template <typename P, typename Q>
    CompressedPair(P&& first, const Q& second)
        : CompressedElement<F, bool>(std::forward<P>(first)),
          CompressedElement<S, int>(second) {
    }
    template <typename P, typename Q>
    CompressedPair(const P& first, const Q& second)
        : CompressedElement<F, bool>(first),
          CompressedElement<S, int>(second) {
    }

    template <typename P, typename Q>
    CompressedPair& operator=(CompressedPair<P, Q>&& other) {
        // if constexpr (std::is_empty_v<P>)
        GetFirst() = std::forward<P>(other.GetFirst());
        GetSecond() = std::forward<Q>(other.GetSecond());
        return *this;
    }

    F& GetFirst() {
        return CompressedElement<F, bool>::Get();
    }

    const F& GetFirst() const {
        return CompressedElement<F, bool>::Get();
    };

    S& GetSecond() {
        return CompressedElement<S, int>::Get();
    }

    const S& GetSecond() const {
        return CompressedElement<S, int>::Get();
    };
};