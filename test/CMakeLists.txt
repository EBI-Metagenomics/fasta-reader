add_custom_target(all_tests)
find_package(hope REQUIRED)

set(ASSETS ${CMAKE_CURRENT_BINARY_DIR}/assets)
file(MAKE_DIRECTORY ${ASSETS})
set(URL "https://hmmer-reader.s3.eu-west-2.amazonaws.com")

# file(DOWNLOAD ${URL}/three-profs.hmm.7z ${ASSETS}/three-profs.hmm.7z
# EXPECTED_HASH SHA1=099404b43b5c0416061d8e911af785d6fd2ee6f3)
# file(ARCHIVE_EXTRACT INPUT ${ASSETS}/three-profs.hmm.7z DESTINATION ${ASSETS})

# file(DOWNLOAD ${URL}/empty.hmm ${ASSETS}/empty.hmm EXPECTED_HASH
# SHA1=da39a3ee5e6b4b0d3255bfef95601890afd80709)

# file(DOWNLOAD ${URL}/corrupted1.hmm.7z ${ASSETS}/corrupted1.hmm.7z
# EXPECTED_HASH SHA1=a4518e30c4fed4fd882534a84f7e603998282346)
# file(ARCHIVE_EXTRACT INPUT ${ASSETS}/corrupted1.hmm.7z DESTINATION ${ASSETS})

# file(DOWNLOAD ${URL}/corrupted2.hmm.7z ${ASSETS}/corrupted2.hmm.7z
# EXPECTED_HASH SHA1=23f27383633d61ca640fce532488281ccee9b7d9)
# file(ARCHIVE_EXTRACT INPUT ${ASSETS}/corrupted2.hmm.7z DESTINATION ${ASSETS})

# file(DOWNLOAD ${URL}/corrupted3.hmm.7z ${ASSETS}/corrupted3.hmm.7z
# EXPECTED_HASH SHA1=4a8dcc07b5b070acd463343e9814412e998cb0ed)
# file(ARCHIVE_EXTRACT INPUT ${ASSETS}/corrupted3.hmm.7z DESTINATION ${ASSETS})

# file(DOWNLOAD ${URL}/corrupted4.hmm.7z ${ASSETS}/corrupted4.hmm.7z
# EXPECTED_HASH SHA1=5694044e4dcd00e9d7f0635d25c53ffda687896e)
# file(ARCHIVE_EXTRACT INPUT ${ASSETS}/corrupted4.hmm.7z DESTINATION ${ASSETS})

# file(DOWNLOAD ${URL}/corrupted5.hmm.7z ${ASSETS}/corrupted5.hmm.7z
# EXPECTED_HASH SHA1=ea038ee3b4ecb6e73689c92bd7a340b56747abc4)
# file(ARCHIVE_EXTRACT INPUT ${ASSETS}/corrupted5.hmm.7z DESTINATION ${ASSETS})

# file(DOWNLOAD ${URL}/corrupted6.hmm.7z ${ASSETS}/corrupted6.hmm.7z
# EXPECTED_HASH SHA1=13d280bd74ba1a4216a7c1910a2e3666c6b11923)
# file(ARCHIVE_EXTRACT INPUT ${ASSETS}/corrupted6.hmm.7z DESTINATION ${ASSETS})

# file(DOWNLOAD ${URL}/corrupted7.hmm.7z ${ASSETS}/corrupted7.hmm.7z
# EXPECTED_HASH SHA1=3793798611c6561c9cced3b5ec469ded39a12bed)
# file(ARCHIVE_EXTRACT INPUT ${ASSETS}/corrupted7.hmm.7z DESTINATION ${ASSETS})

# file(DOWNLOAD ${URL}/corrupted8.hmm.7z ${ASSETS}/corrupted8.hmm.7z
# EXPECTED_HASH SHA1=177e57ef67ce4a49f4fcf77901ddec5b5fb5c744)
# file(ARCHIVE_EXTRACT INPUT ${ASSETS}/corrupted8.hmm.7z DESTINATION ${ASSETS})

function(far_add_test name srcs)
  add_executable(${name} ${srcs})
  add_dependencies(all_tests ${name})
  target_link_libraries(${name} PRIVATE FAR::far)
  target_link_libraries(${name} PUBLIC HOPE::hope)
  target_compile_options(${name} PRIVATE ${WARNING_FLAGS})
  target_compile_features(${name} PRIVATE c_std_11)
  add_test(NAME ${name} COMMAND ${name})

  set(TMPDIR "${CMAKE_CURRENT_BINARY_DIR}/${name}.tmp")
  add_custom_command(
    TARGET ${name}
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TMPDIR})
  target_compile_definitions(${name} PUBLIC "ASSETS=\"${ASSETS}\"")
  target_compile_definitions(${name} PUBLIC "TMPDIR=\"${TMPDIR}\"")
endfunction()

far_add_test(read_fasta read_fasta.c)
