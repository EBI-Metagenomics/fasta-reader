cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(
  fasta_reader
  VERSION 0.0.3
  LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(external/cmake-boilerplate/cbp.cmake)
cbp_set_rpath()

add_library(far src/aux.c src/error.c src/far.c src/fsm.c src/tgt.c src/tok.c)
add_library(FAR::far ALIAS far)

cbp_hide_functions(far)
cbp_set_warning_flags()
cbp_ci_build_option()
cbp_generate_export_header(far EXPORT_FILE)

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

cbp_install(far MERGED)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/far/export.h DESTINATION include/far)
install(FILES ${EXPORT_FILE} DESTINATION include/far)

enable_testing()
add_subdirectory(test)
