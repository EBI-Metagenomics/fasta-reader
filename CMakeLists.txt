cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(
  fasta_reader
  VERSION 0.0.1
  LANGUAGES C)

# Generate compile_commands.json file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
# Set RPATH only if it's not a system directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
     "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif()

add_library(
  far
  src/aux.c
  src/bug.c
  src/error.c
  src/far.c
  src/fsm.c
  src/tgt.c
  src/tok.c)
add_library(FAR::far ALIAS far)

include(GenerateExportHeader)
generate_export_header(
  far
  BASE_NAME
  FAR
  INCLUDE_GUARD_NAME
  FAR_EXPORT_H
  EXPORT_MACRO_NAME
  FAR_API
  EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/far/export.h)

if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
  # /wd5105:
  # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/c5105?view=msvc-160
  set(WARNING_FLAGS /W3 /wd5105)
else()
  set(WARNING_FLAGS
      -Wall
      -Wextra
      -Wstrict-prototypes
      -Wshadow
      -Wconversion
      -Wmissing-prototypes
      -Wno-unused-parameter
      -Wsign-conversion
      -Wno-unused-function)
endif()

if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
  list(APPEND WARNING_FLAGS -Wno-gnu-designator -Wno-empty-translation-unit
       -Wno-gnu-statement-expression -Wno-nullability-extension)
endif()

option(CI_BUILD "CI, extra flags will be set" OFF)
if(CI_BUILD)
  message(STATUS "CI build enabled")
  add_compile_options(-Werror)
endif()

target_include_directories(
  far
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(far PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(far PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_compile_features(far PRIVATE c_std_11)
target_compile_definitions(
  far PRIVATE $<$<BOOL:${WIN32}>:_CRT_NONSTDC_NO_DEPRECATE>)
target_compile_definitions(far PRIVATE $<$<BOOL:${WIN32}>:__extension__>)
target_compile_options(far PRIVATE ${WARNING_FLAGS})
target_compile_options(far PRIVATE "-fverbose-asm")
target_link_libraries(far INTERFACE $<$<BOOL:${UNIX}>:m>)

install(
  TARGETS far
  EXPORT far-targets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include)
install(DIRECTORY src/far/ DESTINATION include/far)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/far/export.h DESTINATION include/far)
install(
  EXPORT far-targets
  FILE far-targets.cmake
  NAMESPACE FAR::
  DESTINATION lib/cmake/far)
include(CMakePackageConfigHelpers)
configure_package_config_file(
  far-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/far-config.cmake
  INSTALL_DESTINATION lib/cmake/far
  NO_SET_AND_CHECK_MACRO)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/far-config.cmake
        DESTINATION lib/cmake/far)

enable_testing()
add_subdirectory(test)
