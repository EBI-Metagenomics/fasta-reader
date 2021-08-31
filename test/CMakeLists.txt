add_custom_target(all_tests)
find_package(hope REQUIRED)

file(COPY assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
set(ASSETS ${CMAKE_CURRENT_BINARY_DIR}/assets)

function(far_add_test name srcs)
  add_executable(${name} ${srcs})
  add_dependencies(all_tests ${name})
  target_link_libraries(${name} PRIVATE FAR::far)
  target_link_libraries(${name} PRIVATE HOPE::hope)
  target_compile_options(${name} PRIVATE ${WARNING_FLAGS})
  target_compile_features(${name} PRIVATE c_std_11)
  add_test(NAME ${name} COMMAND ${name})
  target_compile_definitions(${name} PUBLIC "ASSETS=\"${ASSETS}\"")
endfunction()

far_add_test(read_fasta read_fasta.c)
