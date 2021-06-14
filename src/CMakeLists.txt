# Hint: run test/scripts/install-clang.sh from the build directory.
find_package(fmt REQUIRED CONFIG)

add_executable(example-program main.cpp)
target_compile_features(example-program PUBLIC cxx_std_20)
target_link_libraries(example-program PRIVATE fmt::fmt)
target_compile_definitions(example-program PRIVATE TRAP_STRATEGY)
