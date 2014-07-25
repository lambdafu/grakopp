add_executable(astify astify.cpp)
target_include_directories(astify PRIVATE libgrakopp)
target_link_libraries(astify libgrakopp)
