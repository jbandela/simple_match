if(PROJECT_IS_TOP_LEVEL)
  set(CMAKE_INSTALL_INCLUDEDIR include/simple_match CACHE PATH "")
endif()

# Project is configured with no languages, so tell GNUInstallDirs the lib dir
set(CMAKE_INSTALL_LIBDIR lib CACHE PATH "Object code libraries (lib)")

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

install(
    DIRECTORY include/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT simple_match_Development
)

install(
    TARGETS simple_match_simple_match
    EXPORT simple_matchTargets
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    simple_matchConfigVersion.cmake
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)

# Allow package maintainers to freely override the path for the configs
set(
    simple_match_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/simple_match"
    CACHE STRING "CMake package config location relative to the install prefix"
)
mark_as_advanced(simple_match_INSTALL_CMAKEDIR)

install(
    FILES
    cmake/simple_matchConfig.cmake
    "${PROJECT_BINARY_DIR}/simple_matchConfigVersion.cmake"
    DESTINATION "${simple_match_INSTALL_CMAKEDIR}"
    COMPONENT simple_match_Development
)

install(
    EXPORT simple_matchTargets
    NAMESPACE simple_match::
    DESTINATION "${simple_match_INSTALL_CMAKEDIR}"
    COMPONENT simple_match_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
