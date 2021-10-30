#include "../unique.h"
#include "../my_int.h"

#include <catch.hpp>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////

struct Person {
    virtual int GetFavoriteNumber() const = 0;
    virtual ~Person() = default;
};

struct Alice : Person {
    int GetFavoriteNumber() const override {
        return 37;
    }
};

struct Bob : Person {
    int GetFavoriteNumber() const override {
        return 43;
    }
};

TEST_CASE("Basic") {
    SECTION("Lifetime") {
        {
            UniquePtr<MyInt> s(new MyInt);

            REQUIRE(MyInt::AliveCount() == 1);
        }

        REQUIRE(MyInt::AliveCount() == 0);
    }

    SECTION("Cannot copy") {
        static_assert(!std::is_copy_constructible_v<UniquePtr<int>> &&
                      !std::is_copy_assignable_v<UniquePtr<int>>);
    }

    SECTION("Noexcept") {
        static_assert(std::is_nothrow_move_constructible_v<UniquePtr<int>>);
        static_assert(std::is_nothrow_move_assignable_v<UniquePtr<int>>);
    }

    SECTION("Default value") {
        UniquePtr<int> s;

        REQUIRE(s.Get() == nullptr);
    }

    SECTION("Move") {
        UniquePtr<int> s1(new int);
        UniquePtr<int> s2(new int);

        int* p = s1.Get();
        s2 = std::move(s1);

        REQUIRE(s2.Get() == p);
        REQUIRE(s1.Get() == nullptr);
    }

    SECTION("Self move") {
        UniquePtr<MyInt> s(new MyInt(42));
        MyInt* p = s.Get();
        s = std::move(s);

        REQUIRE(MyInt::AliveCount() == 1);
        REQUIRE(s.Get() == p);
        REQUIRE(*p == 42);
    }

    SECTION("NULL") {
        UniquePtr<MyInt> s(new MyInt);

        REQUIRE(MyInt::AliveCount() == 1);

        s = NULL;  // NOLINT

        REQUIRE(MyInt::AliveCount() == 0);
        REQUIRE(s.Get() == nullptr);
    }

    SECTION("Nullptr") {
        UniquePtr<MyInt> s(new MyInt);

        REQUIRE(MyInt::AliveCount() == 1);

        s = nullptr;

        REQUIRE(MyInt::AliveCount() == 0);
        REQUIRE(s.Get() == nullptr);
    }

    SECTION("Upcasts") {
        std::vector<UniquePtr<Person>> v;
        UniquePtr<Alice> alice(new Alice);
        v.push_back(std::move(alice));
        v.emplace_back(new Bob);
        std::vector<int> res;
        for (const auto& ptr : v) {
            res.push_back(ptr->GetFavoriteNumber());
        }
        REQUIRE(res == std::vector<int>{37, 43});
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Modifiers") {
    SECTION("Release") {
        UniquePtr<MyInt> s(new MyInt(42));
        MyInt* ps = s.Get();
        MyInt* p = s.Release();

        REQUIRE(MyInt::AliveCount() == 1);
        REQUIRE(s.Get() == nullptr);
        REQUIRE(ps == p);
        REQUIRE(*p == 42);

        delete p;

        REQUIRE(MyInt::AliveCount() == 0);
    }

    SECTION("Swap") {
        MyInt* p1 = new MyInt(1);
        UniquePtr<MyInt> s1(p1);
        MyInt* p2 = new MyInt(2);
        UniquePtr<MyInt> s2(p2);

        REQUIRE(s1.Get() == p1);
        REQUIRE(*s1 == 1);
        REQUIRE(s2.Get() == p2);
        REQUIRE(*s2 == 2);

        s1.Swap(s2);

        REQUIRE(s1.Get() == p2);
        REQUIRE(*s1 == 2);
        REQUIRE(s2.Get() == p1);
        REQUIRE(*s2 == 1);
        REQUIRE(MyInt::AliveCount() == 2);

        std::swap(s1, s2);

        REQUIRE(s1.Get() == p1);
        REQUIRE(*s1 == 1);
        REQUIRE(s2.Get() == p2);
        REQUIRE(*s2 == 2);
    }

    SECTION("Reset") {
        UniquePtr<MyInt> s(new MyInt);

        REQUIRE(MyInt::AliveCount() == 1);

        MyInt* p = s.Get();

        REQUIRE(p != nullptr);

        MyInt* new_value = new MyInt;

        REQUIRE(MyInt::AliveCount() == 2);

        s.Reset(new_value);

        REQUIRE(MyInt::AliveCount() == 1);
        REQUIRE(s.Get() == new_value);
    }

    SECTION("Reset const") {
        UniquePtr<const MyInt> s(new MyInt);

        REQUIRE(MyInt::AliveCount() == 1);

        const MyInt* p = s.Get();

        REQUIRE(p != nullptr);

        MyInt* new_value = new MyInt;

        REQUIRE(MyInt::AliveCount() == 2);

        s.Reset(new_value);

        REQUIRE(MyInt::AliveCount() == 1);
        REQUIRE(s.Get() == new_value);
    }

    SECTION("Reset nullptr") {
        UniquePtr<MyInt> s(new MyInt);

        REQUIRE(MyInt::AliveCount() == 1);

        MyInt* p = s.Get();

        REQUIRE(p != nullptr);

        s.Reset(nullptr);

        REQUIRE(MyInt::AliveCount() == 0);
        REQUIRE(s.Get() == nullptr);
    }

    SECTION("Reset no arg") {
        UniquePtr<MyInt> s(new MyInt);

        REQUIRE(MyInt::AliveCount() == 1);

        MyInt* p = s.Get();

        REQUIRE(p != nullptr);

        s.Reset();

        REQUIRE(s.Get() == nullptr);
    }

    SECTION("Reset self pass") {
        struct Sui {
            UniquePtr<Sui> ptr_;

            Sui() : ptr_(this) {
            }

            void Reset() {
                ptr_.Reset();
            }
        };

        (new Sui)->Reset();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Observers") {
    SECTION("Dereference") {
        UniquePtr<int> p(new int(3));

        REQUIRE(*p == 3);
    }

    SECTION("operator bool") {
        UniquePtr<int> p(new int(1));
        UniquePtr<int> const& cp = p;

        REQUIRE(p);
        REQUIRE(cp);

        UniquePtr<int> p0;
        UniquePtr<int> const& cp0 = p0;

        REQUIRE(!p0);
        REQUIRE(!cp0);
    }

    SECTION("Get") {
        int* p = new int(1);

        UniquePtr<int> s(p);
        UniquePtr<int> const& sc = s;

        REQUIRE(s.Get() == p);
        REQUIRE(sc.Get() == s.Get());
    }

    SECTION("Get const") {
        const int* p = new int(1);

        UniquePtr<const int> s(p);
        UniquePtr<const int> const& sc = s;

        REQUIRE(s.Get() == p);
        REQUIRE(sc.Get() == s.Get());
    }

    SECTION("operator->") {
        struct A {
            int i_{7};
        };

        UniquePtr<A> p(new A);
        REQUIRE(p->i_ == 7);
    }
}
