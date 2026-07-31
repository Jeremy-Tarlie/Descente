cmake_minimum_required(VERSION 3.16)

include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.4
)
FetchContent_MakeAvailable(Catch2)

add_executable(descente_tests
  test_bsp.cpp
  test_astar.cpp
)
target_link_libraries(descente_tests PRIVATE descente_core Catch2::Catch2WithMain)
if(NOT MSVC)
  target_compile_options(descente_tests PRIVATE -Wno-c2y-extensions)
endif()

include(CTest)
include(Catch)
catch_discover_tests(descente_tests)
