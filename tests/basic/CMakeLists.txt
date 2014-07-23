set(PEG_FILES basic.peg)
Foreach(_pegFile ${PEG_FILES})
  String(REGEX REPLACE ".peg$" ".cpp" _cppFile "${_pegFile}")
  Set(_command "${CMAKE_SOURCE_DIR}/grakopp --whitespace= --no-nameguard ${_pegFile} > ${_cppFile}")
  Add_Custom_Command(
    OUTPUT ${_cppFile}
    COMMAND ${CMAKE_SOURCE_DIR}/grakopp --whitespace= --no-nameguard ${CMAKE_CURRENT_SOURCE_DIR}/${_pegFile} > ${_cppFile}
    DEPENDS ${_pegFile}
    VERBATIM)
EndForeach(_pegFile ${PEG_FILES})

add_executable(basic basic.cpp)
target_include_directories(basic PRIVATE libgrakopp)
target_link_libraries(basic libgrakopp)

# FIXME: Add --test option to generated main functions that does comparison.
add_test(basictest basic ${CMAKE_CURRENT_SOURCE_DIR}/basic-001-sequence.in sequence)