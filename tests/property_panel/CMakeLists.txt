set(TEST_FILES
        property_panel_test.cpp
)

set(SRC_FILES
        ${CMAKE_SOURCE_DIR}/src/property_panel/editors/vector3_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/property_panel/editors/vector3_editor.h
        ${CMAKE_SOURCE_DIR}/src/property_panel/editors/enum_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/property_panel/editors/enum_editor.h
        ${CMAKE_SOURCE_DIR}/src/property_panel/editors/reorderable_point_list_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/property_panel/editors/reorderable_point_list_editor.h
)

add_executable(property_panel_test ${TEST_FILES} ${SRC_FILES})

target_include_directories(property_panel_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(property_panel_test PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
)
