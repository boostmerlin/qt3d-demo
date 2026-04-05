set(DOCTEST_PATH ${CMAKE_CURRENT_SOURCE_DIR})
#set(DOCTEST_PATH ${CMAKE_CURRENT_BINARY_DIR})

#set(DOCTEST_VERSION 2.4.11)
#if (NOT EXISTS ${DOCTEST_PATH}/doctest.h)
#    message("doctest.h not found, download it")
#    file(DOWNLOAD https://raw.githubusercontent.com/onqtam/doctest/v${DOCTEST_VERSION}/doctest/doctest.h ${DOCTEST_PATH}/doctest.h STATUS status)
#    message("doctest download status: ${status}")
#endif ()

find_package(doctest CONFIG REQUIRED)

add_library(common INTERFACE)

target_include_directories(common INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
)

target_link_libraries(common INTERFACE
        doctest::doctest
        Qt${QT_VERSION_MAJOR}::Widgets
)

add_subdirectory(core)
