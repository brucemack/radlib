cmake_minimum_required(VERSION 3.13)
project(radlib C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-fstack-protector-all -Wall -Wpedantic -g)

add_executable(rtty-test-1
  tests/rtty/rtty-test-1.cpp 
  tests/scamp/TestModem2.cpp 
  tests/scamp/TestDemodulatorListener.cpp 
  tests/TestFSKModulator.cpp 
  tests/TestFSKModulator2.cpp 
  rtty/BaudotDecoder.cpp 
  rtty/BaudotEncoder.cpp 
  rtty/RTTYDemodulator.cpp 
  util/Demodulator.cpp 
  util/fixed_math.cpp 
  util/fixed_fft.cpp 
  util/dsp_util.cpp 
  util/WindowAverage.cpp 
)

add_executable(util-test-1
  tests/util/util-test-1.cpp
  util/fixed_math.cpp 
  util/fixed_fft.cpp 
  util/dsp_util.cpp 
  util/f32_fft.cpp 
  util/WindowAverage.cpp 
)

add_executable(unit-test-2
  tests/unit-test-2.cpp
  util/WindowAverage.cpp 
)

add_executable(unit-test-3
  tests/unit-test-3.cpp
  tests/TestFSKModulator.cpp 
  morse/MorseEncoder.cpp 
)

# Desktop build
add_executable(unit-test-4
  tests/scamp/unit-test-4.cpp 
  tests/scamp/TestModem.cpp 
  tests/scamp/TestModem2.cpp 
  tests/scamp/TestDemodulatorListener.cpp
  scamp/Symbol6.cpp 
  scamp/CodeWord12.cpp 
  scamp/CodeWord24.cpp 
  scamp/Frame30.cpp 
  scamp/Util.cpp 
  scamp/ClockRecoveryPLL.cpp
  scamp/ClockRecoveryDLL.cpp
  util/Demodulator.cpp
  util/FileModulator.cpp 
  util/fixed_math.cpp 
  util/fixed_fft.cpp 
  util/dsp_util.cpp 
)

add_executable(unit-test-7a
  tests/scamp/unit-test-7a.cpp 
  tests/scamp/TestModem.cpp 
  tests/scamp/TestModem2.cpp 
  tests/scamp/TestDemodulatorListener.cpp
  scamp/Symbol6.cpp 
  scamp/CodeWord12.cpp 
  scamp/CodeWord24.cpp 
  scamp/Frame30.cpp 
  scamp/Util.cpp 
  scamp/ClockRecoveryPLL.cpp
  scamp/ClockRecoveryDLL.cpp
  util/Demodulator.cpp
  scamp/SCAMPDemodulator.cpp
  util/FileModulator.cpp 
  util/fixed_math.cpp 
  util/fixed_fft.cpp 
  util/dsp_util.cpp 
)

