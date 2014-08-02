set(PEG_FILES basic.peg)
peg_files("" False ${PEG_FILES})

add_executable(basic _basic.cpp)
target_compile_options(basic PRIVATE -DGRAKOPP_MAIN)
target_include_directories(basic PRIVATE libgrakopp)
target_link_libraries(basic libgrakopp)

peg_test(
  basic-001-disjunction 
  basic-002-sequence
  basic-003-group
  basic-004-optional
  basic-005-optional
  basic-006-closure
  basic-007-closure
  basic-008-closure
  basic-009-positive_closure
  basic-010-positive_closure
  basic-011-positive_closure
  )
