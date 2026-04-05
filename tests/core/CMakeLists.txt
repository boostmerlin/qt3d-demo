file(GLOB SRC_FILES ${CMAKE_SOURCE_DIR}/src/core/*.cpp)
file(GLOB TEST_FILES *.cpp)

add_executable(core_test ${TEST_FILES} ${SRC_FILES})

target_link_libraries(core_test PRIVATE common)
