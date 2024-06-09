cmake_minimum_required(VERSION 3.5)

project (LTL)

set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(TESTCASES_DIR ${PROJECT_ROOT_DIR}/testcases)

file(GLOB SOURCES "${PROJECT_ROOT_DIR}/*.cpp")

add_executable(LTL
  ${SOURCES}
)

target_compile_options(LTL
  PRIVATE
    -g
    -Wall
    -Wextra
    -DPROJECT_ROOT_DIR="${PROJECT_ROOT_DIR}"
)

add_custom_target(run
   COMMAND ${PROJECT_ROOT_DIR}/build/LTL 
   DEPENDS LTL
   WORKING_DIRECTORY ${PROJECT_ROOT_DIR}
   VERBATIM 
)