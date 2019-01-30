include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(one_plus_one one_plus_one.cxx)

target_link_libraries(one_plus_one gtest gtest_main)
target_link_libraries(one_plus_one homoGBA_lib)

add_test(NAME one_plus_one
         COMMAND one_plus_one)
