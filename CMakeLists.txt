cmake_minimum_required(VERSION 3.16)

project(IssueFlow VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(issueflow
    src/domain.cpp
    src/algorithms.cpp
)

target_include_directories(issueflow PUBLIC include)

if(MSVC)
    target_compile_options(issueflow PRIVATE /W4 /permissive-)
else()
    target_compile_options(issueflow PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(issueflow_cli src/main.cpp)
target_link_libraries(issueflow_cli PRIVATE issueflow)

enable_testing()
add_executable(issueflow_tests tests/test_issueflow.cpp)
target_link_libraries(issueflow_tests PRIVATE issueflow)
add_test(NAME issueflow_tests COMMAND issueflow_tests)
set_tests_properties(issueflow_tests PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
