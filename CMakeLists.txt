cmake_minimum_required(VERSION 3.13)
project(radlib C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-fstack-protector-all -Wall -Wpedantic -g)

add_executable(unit-test-1
  tests/unit-test-1.cpp 
  tests/TestFSKModulator.cpp 
  rtty/BaudotDecoder.cpp 
  rtty/BaudotEncoder.cpp 
)

add_executable(unit-test-2
  tests/unit-test-2.cpp
  util/WindowAverage.cpp 
)
