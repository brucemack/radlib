cmake_minimum_required(VERSION 3.13)
project(radlib C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-fstack-protector-all -Wall -Wpedantic -g)

# Desktop build
add_executable(unit-test-1
  tests/unit-test-1.cpp 
  tests/TestFSKModulator.cpp 
  rtty/BaudotDecoder.cpp 
  rtty/BaudotEncoder.cpp 
)
