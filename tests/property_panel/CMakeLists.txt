set(TEST_FILES
        property_panel_test.cpp
)

set(SRC_FILES
        ${CMAKE_SOURCE_DIR}/src/core/observable_object.cpp
        ${CMAKE_SOURCE_DIR}/src/core/observable_object.h
        ${CMAKE_SOURCE_DIR}/src/core/x_property.cpp
        ${CMAKE_SOURCE_DIR}/src/core/x_property.h
        ${CMAKE_SOURCE_DIR}/src/core/x_property_validator.cpp
        ${CMAKE_SOURCE_DIR}/src/core/x_property_validator.h
        ${CMAKE_SOURCE_DIR}/src/scene/scene_object.cpp
        ${CMAKE_SOURCE_DIR}/src/scene/scene_object.h
        ${CMAKE_SOURCE_DIR}/src/property_panel/property_editor_factory.cpp
        ${CMAKE_SOURCE_DIR}/src/property_panel/property_editor_factory.h
        ${CMAKE_SOURCE_DIR}/src/property_panel/editors/color_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/property_panel/editors/color_editor.h
        ${CMAKE_SOURCE_DIR}/src/property_panel/editors/text_editor_row.cpp
        ${CMAKE_SOURCE_DIR}/src/property_panel/editors/text_editor_row.h
        ${CMAKE_SOURCE_DIR}/src/property_panel/editors/double_editor_row.cpp
        ${CMAKE_SOURCE_DIR}/src/property_panel/editors/double_editor_row.h
        ${CMAKE_SOURCE_DIR}/src/property_panel/editors/int_editor_row.cpp
        ${CMAKE_SOURCE_DIR}/src/property_panel/editors/int_editor_row.h
        ${CMAKE_SOURCE_DIR}/src/property_panel/editors/vector3_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/property_panel/editors/vector3_editor.h
        ${CMAKE_SOURCE_DIR}/src/property_panel/editors/enum_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/property_panel/editors/enum_editor.h
        ${CMAKE_SOURCE_DIR}/src/property_panel/editors/reorderable_point_list_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/property_panel/editors/reorderable_point_list_editor.h
        ${CMAKE_SOURCE_DIR}/src/property_panel/property_section.h
        ${CMAKE_SOURCE_DIR}/src/property_panel/sections/object_section.cpp
        ${CMAKE_SOURCE_DIR}/src/property_panel/sections/object_section.h
)

add_executable(property_panel_test ${TEST_FILES} ${SRC_FILES})

target_include_directories(property_panel_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/core
        ${CMAKE_SOURCE_DIR}/src/scene
)

target_link_libraries(property_panel_test PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
)
