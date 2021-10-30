# ------------------------------------------------------------------------------
# UniquePtr

add_catch(test_unique_basic unique_basic/test.cpp)
add_catch(test_unique_advanced unique_advanced/test.cpp unique_basic/test.cpp)

# ------------------------------------------------------------------------------
# SharedPtr + WeakPtr

add_catch(test_shared_basic shared_basic/test.cpp)
add_catch(test_shared_weak shared_basic/test.cpp shared_weak/test.cpp)
add_catch(test_shared_from_this shared_basic/test.cpp shared_weak/test.cpp shared_from_this/test.cpp)
