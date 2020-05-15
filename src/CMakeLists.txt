# Hint: run test/scripts/install-clang.sh from the build directory.
find_package(fmt REQUIRED CONFIG)

add_executable(animals main.cpp acme/socket.cpp)
target_compile_features(animals PUBLIC cxx_std_20)
target_link_libraries(animals PRIVATE fmt::fmt)
