cmake_minimum_required(VERSION 3.14)

project(simple_matchTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/windows-set-path.cmake)

if(PROJECT_IS_TOP_LEVEL)
  find_package(simple_match REQUIRED)
  enable_testing()
endif()

find_package(Boost REQUIRED)

function(test name)
  set(target "simple_match_${name}")
  add_executable("${target}" "${name}.cpp")
  target_link_libraries(
      "${target}" PUBLIC
      simple_match::simple_match Boost::headers
  )
  target_compile_features("${target}" PRIVATE cxx_std_14)
  add_test(NAME "${target}" COMMAND "${target}")
endfunction()

test(test)
test(cppcon_feedback)
