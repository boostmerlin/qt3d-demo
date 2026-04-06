set(TEST_FILES
        render_node_test.cpp
)

set(SRC_FILES
        ${CMAKE_SOURCE_DIR}/src/q3dextension/extras/layer_manager.cpp
        ${CMAKE_SOURCE_DIR}/src/q3dextension/extras/layer_manager.h
        ${CMAKE_SOURCE_DIR}/src/q3dextension/extras/behavior_component.cpp
        ${CMAKE_SOURCE_DIR}/src/q3dextension/extras/behavior_component.h
        ${CMAKE_SOURCE_DIR}/src/visualization/graph3d/nodes/render_node.cpp
        ${CMAKE_SOURCE_DIR}/src/visualization/graph3d/nodes/render_node.h
)

add_executable(render_node_test ${TEST_FILES} ${SRC_FILES})

target_include_directories(render_node_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/q3dextension/extras
        ${CMAKE_SOURCE_DIR}/src/visualization/graph3d/nodes
)

target_link_libraries(render_node_test PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::3DCore
        Qt${QT_VERSION_MAJOR}::3DRender
)
