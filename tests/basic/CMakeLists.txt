set(PEG_FILES basic.peg)
peg_files("" False ${PEG_FILES})

add_executable(basic basic.cpp)
target_include_directories(basic PRIVATE libgrakopp)
target_link_libraries(basic libgrakopp)

# FIXME: Add --test option to generated main functions that does comparison.
add_test(basic001 basic ${CMAKE_CURRENT_SOURCE_DIR}/basic-001-disjunction.in disjunction)
add_test(basic002 basic ${CMAKE_CURRENT_SOURCE_DIR}/basic-002-sequence.in sequence)
