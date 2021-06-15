cmake_minimum_required(VERSION 3.14)

include(cmake/in-source-guard.cmake)

project(
    simple_match
    VERSION 0.1.0
    DESCRIPTION "Simple header only pattern matching for c++14"
    HOMEPAGE_URL "https://github.com/jbandela/simple_match"
    LANGUAGES NONE
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(simple_match_simple_match INTERFACE)
add_library(simple_match::simple_match ALIAS simple_match_simple_match)

set_property(
    TARGET simple_match_simple_match PROPERTY
    EXPORT_NAME simple_match
)

target_include_directories(
    simple_match_simple_match ${simple_match_warning_guard}
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_compile_features(simple_match_simple_match INTERFACE cxx_std_14)

# ---- Install rules ----

include(cmake/install-rules.cmake)

# ---- Developer mode ----

if(NOT simple_match_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of simple_match"
  )
endif()

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()

option(BUILD_MCSS_DOCS "Build documentation using Doxygen and m.css" OFF)
if(BUILD_MCSS_DOCS)
  include(cmake/docs.cmake)
endif()

option(ENABLE_COVERAGE "Enable coverage support separate from CTest's" OFF)
if(ENABLE_COVERAGE)
  include(cmake/coverage.cmake)
endif()

include(cmake/lint-targets.cmake)
include(cmake/spell-targets.cmake)
