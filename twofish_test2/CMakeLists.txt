cmake_minimum_required(VERSION 3.27.8)
project(twofish_test2)

set(CMAKE_CXX_STANDARD 17)

add_executable(twofish_test2 main.cpp
        twofish.cpp
        twofish.h
        keys.h
        galois.cpp
        galois.h
        twofishTests.cpp
        twofishTests.h
        converting.cpp
        converting.h
)
