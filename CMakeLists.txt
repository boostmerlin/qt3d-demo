cmake_minimum_required(VERSION 3.21)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

project(qt3d-demo VERSION 0.1 LANGUAGES CXX)

option(QT3D_DEMO_USE_VCPKG "Enable vcpkg toolchain integration" OFF)
option(QT3D_DEMO_ENABLE_BREAKPAD "Link Breakpad support" OFF)
option(QT3D_DEMO_BUILD_TESTS "Build test targets" OFF)

if (QT3D_DEMO_USE_VCPKG)
    include(VCPKG)
endif ()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets 3DCore 3DRender 3DInput 3DExtras LinguistTools)

set(TS_FILES qt3d-demo_zh_CN.ts)

set(PROJECT_SOURCES
        ${TS_FILES}
        resources/resources.qrc
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui
        src/hierarchy.cpp
        src/hierarchy.h
        src/property_panel.cpp
        src/property_panel.h
        src/core/observable_object.cpp
        src/core/observable_object.h
        src/core/x_property.cpp
        src/core/x_property.h
        src/core/x_property_validator.cpp
        src/core/x_property_validator.h
        src/q3dextension/extras/behavior_component.cpp
        src/q3dextension/extras/behavior_component.h
        src/q3dextension/extras/camera_controller.cpp
        src/q3dextension/extras/camera_controller.h
        src/q3dextension/extras/custom_renderer.cpp
        src/q3dextension/extras/custom_renderer.h
        src/q3dextension/extras/gizmo_axis.cpp
        src/q3dextension/extras/gizmo_axis.h
        src/q3dextension/extras/layer_manager.cpp
        src/q3dextension/extras/layer_manager.h
        src/q3dextension/extras/text3d_entity.cpp
        src/q3dextension/extras/text3d_entity.h
        src/q3dextension/computational_geometry/line_geometry.cpp
        src/q3dextension/computational_geometry/line_geometry.h
        src/q3dextension/computational_geometry/line_mesh.cpp
        src/q3dextension/computational_geometry/line_mesh.h
        src/q3dextension/computational_geometry/ring_geometry.cpp
        src/q3dextension/computational_geometry/ring_geometry.h
        src/q3dextension/computational_geometry/ring_mesh.cpp
        src/q3dextension/computational_geometry/ring_mesh.h
        src/q3dextension/computational_geometry/edge_mesh.cpp
        src/q3dextension/computational_geometry/edge_mesh.h
        src/q3dextension/computational_geometry/extrude_shape_geometry.cpp
        src/q3dextension/computational_geometry/extrude_shape_geometry.h
        src/q3dextension/computational_geometry/extrude_shape_mesh.cpp
        src/q3dextension/computational_geometry/extrude_shape_mesh.h
        src/q3dextension/utils/geometry_util.cpp
        src/q3dextension/utils/geometry_util.h
        src/q3dextension/curves/curve.cpp
        src/q3dextension/curves/curve.h
        src/q3dextension/curves/line_curve.cpp
        src/q3dextension/curves/line_curve.h
        src/q3dextension/curves/bezier_curve.cpp
        src/q3dextension/curves/bezier_curve.h
        src/q3dextension/curves/bezier_curve.tpp
        src/q3dextension/shapes/shape.cpp
        src/q3dextension/shapes/shape.h
        src/q3dextension/shapes/polygon_profile_shape.cpp
        src/q3dextension/shapes/polygon_profile_shape.h
        src/visualization/graph3d/graph3d_scene.cpp
        src/visualization/graph3d/graph3d_scene.h
        src/visualization/graph3d/render_node_factory.cpp
        src/visualization/graph3d/render_node_factory.h
        src/visualization/graph3d/scene_model.cpp
        src/visualization/graph3d/scene_model.h
        src/visualization/graph3d/core/render_node.cpp
        src/visualization/graph3d/core/render_node.h
        src/property_panel.cpp
        src/property_panel.h
        src/demo/scene_object.cpp
        src/demo/scene_object.h
        src/demo/edge_render_node.cpp
        src/demo/extrude_object.cpp
        src/demo/extrude_object.h
        src/demo/extrude_render_node.cpp
        src/demo/extrude_render_node.h
        src/demo/polygon_object.cpp
        src/demo/polygon_object.h
        src/demo/polygon_render_node.cpp
        src/demo/polygon_render_node.h
        src/demo/primitive_object.cpp
        src/demo/primitive_object.h
        src/demo/primitive_render_node.cpp
        src/demo/primitive_render_node.h
        src/demo/scene_controller.cpp
        src/demo/scene_controller.h
        src/q3dextension/computational_geometry/polygon_geometry.cpp
        src/q3dextension/computational_geometry/polygon_geometry.h
        src/q3dextension/computational_geometry/polygon_mesh.cpp
        src/q3dextension/computational_geometry/polygon_mesh.h
)

if(WIN32)
    if (MSVC)
        add_compile_options(/bigobj)
    else()
        add_compile_options(-Wa,-mbig-obj)
    endif()
elseif(APPLE)
    add_compile_options(-fPIC)
elseif(UNIX AND NOT APPLE)
    add_compile_options(-fPIC -fpermissive)
endif()

qt_add_executable(qt3d-demo
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

target_link_libraries(qt3d-demo PRIVATE
        Qt6::Widgets
        Qt6::3DCore
        Qt6::3DRender
        Qt6::3DInput
        Qt6::3DExtras
)

target_include_directories(qt3d-demo PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/core
        ${CMAKE_SOURCE_DIR}/src/demo
        ${CMAKE_SOURCE_DIR}/src/utils
        ${CMAKE_SOURCE_DIR}/src/q3dextension/extras
        ${CMAKE_SOURCE_DIR}/src/q3dextension/utils
        ${CMAKE_SOURCE_DIR}/src/q3dextension/curves
        ${CMAKE_SOURCE_DIR}/src/q3dextension/shapes
        ${CMAKE_SOURCE_DIR}/src/visualization/graph3d
        ${CMAKE_SOURCE_DIR}/src/visualization/graph3d/core
)

if (QT3D_DEMO_ENABLE_BREAKPAD)
    find_package(unofficial-breakpad CONFIG REQUIRED)
    target_link_libraries(qt3d-demo PRIVATE
            unofficial::breakpad::libbreakpad
            unofficial::breakpad::libbreakpad_client
    )
endif ()

set_target_properties(qt3d-demo PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS qt3d-demo
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(qt3d-demo)

if (QT3D_DEMO_BUILD_TESTS)
    add_subdirectory(tests)
endif ()
