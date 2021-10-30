#include "../unique.h"

#include "deleters.h"
#include "../my_int.h"

#include <catch.hpp>
#include <tuple>

TEST_CASE("Construction with deleters") {
    SECTION("From copyable deleter") {
        const CopyableDeleter<MyInt> cd;
        UniquePtr<MyInt, CopyableDeleter<MyInt>> s(new MyInt, cd);
    }

    SECTION("From move-only deleter") {
        Deleter<MyInt> d;
        UniquePtr<MyInt, Deleter<MyInt>> s(new MyInt, std::move(d));
    }

    SECTION("From temporary") {
        UniquePtr<MyInt, Deleter<MyInt>> s(new MyInt, Deleter<MyInt>{});
    }

    SECTION("Deleter type is non-const reference") {
        Deleter<MyInt> d;
        UniquePtr<MyInt, Deleter<MyInt>&> s(new MyInt, d);
    }

    SECTION("Deleter type is const reference") {
        Deleter<MyInt> d;
        UniquePtr<MyInt, const Deleter<MyInt>&> s1(new MyInt, d);

        const Deleter<MyInt>& cr = d;
        UniquePtr<MyInt, const Deleter<MyInt>&> s2(new MyInt, cr);
    }
}

TEST_CASE("Swap with deleters") {
    SECTION("If storing deleter by value") {
        MyInt* p1 = new MyInt(1);
        UniquePtr<MyInt, Deleter<MyInt>> s1(p1, Deleter<MyInt>(1));
        MyInt* p2 = new MyInt(2);
        UniquePtr<MyInt, Deleter<MyInt>> s2(p2, Deleter<MyInt>(2));

        s1.Swap(s2);

        REQUIRE(s1.Get() == p2);
        REQUIRE(*s1 == 2);
        REQUIRE(s2.Get() == p1);
        REQUIRE(*s2 == 1);
        REQUIRE(s1.GetDeleter().GetTag() == 2);
        REQUIRE(s2.GetDeleter().GetTag() == 1);
        REQUIRE(MyInt::AliveCount() == 2);

        // std::swap(s1, s2);
        s1.Swap(s2);

        REQUIRE(s1.Get() == p1);
        REQUIRE(*s1 == 1);
        REQUIRE(s2.Get() == p2);
        REQUIRE(*s2 == 2);
        REQUIRE(s1.GetDeleter().GetTag() == 1);
        REQUIRE(s2.GetDeleter().GetTag() == 2);
    }

    /*
     * SECTION("If storing reference to deleter") {
     *
     * }
     *
     * well, me think it's enough for you this time...
     */
}

TEST_CASE("Moving deleters") {
    SECTION("Move with custom deleter") {
        UniquePtr<MyInt, Deleter<MyInt>> s1(new MyInt, Deleter<MyInt>(5));
        MyInt* p = s1.Get();
        UniquePtr<MyInt, Deleter<MyInt>> s2(new MyInt);

        REQUIRE(MyInt::AliveCount() == 2);
        REQUIRE(s1.GetDeleter().GetTag() == 5);
        REQUIRE(s2.GetDeleter().GetTag() == 0);

        s2 = std::move(s1);

        REQUIRE(s2.Get() == p);
        REQUIRE(s1.Get() == nullptr);
        REQUIRE(MyInt::AliveCount() == 1);
        REQUIRE(s2.GetDeleter().GetTag() == 5);
        REQUIRE(s1.GetDeleter().GetTag() == 0);
    }

    SECTION("Move with reference deleter type") {
        CopyableDeleter<MyInt> d1(5);
        UniquePtr<MyInt, CopyableDeleter<MyInt>&> s1(new MyInt, d1);
        MyInt* p1 = s1.Get();

        CopyableDeleter<MyInt> d2(6);
        UniquePtr<MyInt, CopyableDeleter<MyInt>&> s2(new MyInt, d2);

        REQUIRE(MyInt::AliveCount() == 2);

        s2 = std::move(s1);

        REQUIRE(s2.Get() == p1);
        REQUIRE(s1.Get() == nullptr);
        REQUIRE(MyInt::AliveCount() == 1);
        REQUIRE(d1.GetTag() == 5);
        REQUIRE(d2.GetTag() == 5);
    }
}

TEST_CASE("GetDeleter") {
    SECTION("Get deleter") {
        UniquePtr<MyInt, Deleter<MyInt>> p;

        REQUIRE(!p.GetDeleter().IsConst());
    }

    SECTION("Get deleter const") {
        const UniquePtr<MyInt, Deleter<MyInt>> p;

        REQUIRE(p.GetDeleter().IsConst());
    }

    SECTION("Get deleter reference") {
        using UDRef = UniquePtr<MyInt, Deleter<MyInt>&>;
        Deleter<MyInt> d;

        UDRef p(nullptr, d);
        const UDRef& cp = p;

        REQUIRE(!p.GetDeleter().IsConst());
        REQUIRE(!cp.GetDeleter().IsConst());
    }

    SECTION("Get deleter const reference") {
        using UDConstRef = UniquePtr<MyInt, const Deleter<MyInt>&>;
        const Deleter<MyInt> d;

        UDConstRef p(nullptr, d);
        const UDConstRef& cp = p;

        REQUIRE(p.GetDeleter().IsConst());
        REQUIRE(cp.GetDeleter().IsConst());
    }
}

struct VoidPtrDeleter {
    void operator()(void* ptr) {
        free(ptr);
    }
};

TEST_CASE("UniquePtr<void, VoidPtrDeleter>") {
    SECTION("It compiles!") {
        UniquePtr<void, VoidPtrDeleter> p(malloc(100));
    }
}

TEST_CASE("Array specialization") {
    SECTION("delete[] is called") {
        UniquePtr<MyInt[]> u(new MyInt[100]);
        REQUIRE(MyInt::AliveCount() == 100);
        u.Reset();
        REQUIRE(MyInt::AliveCount() == 0);
    }

    SECTION("Able to use custom deleters") {
        UniquePtr<MyInt[], Deleter<MyInt[]>> u(new MyInt[100]);
        REQUIRE(MyInt::AliveCount() == 100);
        u.Reset();
        REQUIRE(MyInt::AliveCount() == 0);
    }

    SECTION("Operator []") {
        int* arr = new int[5];
        for (size_t i = 0; i < 5; ++i) {
            arr[i] = i;
        }

        UniquePtr<int[]> u(arr);
        for (int i = 0; i < 5; ++i) {
            REQUIRE(u[i] == i);
            u[i] = -i;
            REQUIRE(u[i] == -i);
        }
    }
}

template <typename T>
void DeleteFunction(T* ptr) {
    delete ptr;
}

template <typename T>
struct StatefulDeleter {
    int some_useless_field = 0;

    void operator()(T* ptr) {
        delete ptr;
        ++some_useless_field;
    }
};

TEST_CASE("Compressed pair usage") {

    SECTION("Stateless struct deleter") {
        static_assert(sizeof(UniquePtr<int>) == sizeof(void*));
        static_assert(sizeof(UniquePtr<int, std::default_delete<int>>) == sizeof(int*));
    }

    SECTION("Stateful struct deleter") {
        static_assert(sizeof(UniquePtr<int, StatefulDeleter<int>>) ==
                      sizeof(std::pair<int*, StatefulDeleter<int>>));
    }

    SECTION("Stateless lambda deleter") {
        auto lambda_deleter = [](int* ptr) { delete ptr; };
        static_assert(sizeof(UniquePtr<int, decltype(lambda_deleter)>) == sizeof(int*));
    }

    SECTION("Stateful lambda deleter") {
        int some_useless_counter = 0;
        auto lambda_deleter = [&some_useless_counter](int* ptr) {
            delete ptr;
            ++some_useless_counter;
        };
        static_assert(sizeof(UniquePtr<int, decltype(lambda_deleter)>) ==
                      sizeof(std::pair<int*, decltype(lambda_deleter)>));
    }

    SECTION("Function pointer deleter") {
        static_assert(sizeof(UniquePtr<int, decltype(&DeleteFunction<int>)>) ==
                      sizeof(std::pair<int*, decltype(&DeleteFunction<int>)>));
    }
}

template <typename T>
class DerivedDeleter : public Deleter<T> {};

TEST_CASE("Upcasts") {
    SECTION("Upcast in move constructor") {
        UniquePtr<MyInt, DerivedDeleter<MyInt>> s(new MyInt);
        UniquePtr<MyInt, Deleter<MyInt>> s2(std::move(s));
    }

    SECTION("Upcast in move assignment") {
        UniquePtr<MyInt, DerivedDeleter<MyInt>> s(new MyInt);
        UniquePtr<MyInt, Deleter<MyInt>> s2(new MyInt);
        s2 = std::move(s);
    }
}

TEST_CASE("Deleter call check") {
    SECTION("Was called") {
        Deleter<int> d;
        { UniquePtr<int, Deleter<int>&> up(new int, d); }
        REQUIRE(d.WasCalled());
    }

    SECTION("Was not called") {
        Deleter<int> d;
        { UniquePtr<int, Deleter<int>&> up(nullptr, d); }
        REQUIRE(!d.WasCalled());
    }
}
