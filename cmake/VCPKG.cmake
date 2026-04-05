# Created by merlin

option(OPT_VCPKG_UPDATE "Update vcpkg" OFF)

function(set_tool_chain toolchain)
    set(CMAKE_TOOLCHAIN_FILE ${toolchain} CACHE FILEPATH "using vcpkg tool" FORCE)
endfunction()

if (DEFINED ENV{VCPKG_ROOT})
    # clion fix?
    string(FIND "$ENV{VCPKG_ROOT}" "Microsoft Visual Studio" POSITION)
    if (POSITION GREATER_EQUAL 0)
        set(ENV{VCPKG_ROOT} "")
    else ()
        message("Use user defined vcpkg: " $ENV{VCPKG_ROOT})
        set_tool_chain("$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        return()
    endif ()
endif ()

if (CMAKE_HOST_WIN32)
    set(MY_HOME $ENV{USERPROFILE})
else ()
    set(MY_HOME $ENV{HOME})
endif ()

if (EXISTS "$ENV{VCPKG_INSTALL_ROOT}")
    set(VCPKG_ROOT "$ENV{VCPKG_INSTALL_ROOT}/vcpkg")
else ()
    message(STATUS "Vcpkg dir: ${MY_HOME}, set VCPKG_INSTALL_ROOT environment variable to change")
    set(VCPKG_ROOT "${MY_HOME}/.vcpkg/vcpkg")
endif ()
file(TO_CMAKE_PATH ${VCPKG_ROOT} VCPKG_ROOT)

message(STATUS "VCPKG_ROOT: " ${VCPKG_ROOT})

set(VCPKG_CMD "${VCPKG_ROOT}/vcpkg")

if (EXISTS "${VCPKG_ROOT}/.vcpkg-root")
    if (MY_VCPKG_UPDATE)
        message("Vcpkg already exists, try to update")
        execute_process(
                COMMAND git pull
                WORKING_DIRECTORY ${VCPKG_ROOT}
                RESULT_VARIABLE GIT_RESULT
        )
        if (NOT GIT_RESULT EQUAL 0)
            message(WARNING "Failed to update vcpkg")
        endif ()
        # run ports update?
        execute_process(
                COMMAND "${VCPKG_CMD}" update
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
        execute_process(
                COMMAND "${VCPKG_CMD}" upgrade
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    endif ()
else ()
    execute_process(
            COMMAND git clone --progress https://github.com/microsoft/vcpkg.git ${VCPKG_ROOT}
            RESULT_VARIABLE GIT_RESULT
    )
    if (NOT GIT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to clone vcpkg")
        return()
    endif ()
endif ()
execute_process(
        COMMAND "${VCPKG_CMD}" version --disable-metrics
        RESULT_VARIABLE VCPKG_RESULT
)

if (NOT VCPKG_RESULT EQUAL 0)
    message(STATUS "Try installing vcpkg")
    if (CMAKE_HOST_WIN32)
        set(VCPKG_INSTALL_CMD "bootstrap-vcpkg.bat")
    else ()
        set(VCPKG_INSTALL_CMD "bootstrap-vcpkg.sh")
    endif ()
    execute_process(
            COMMAND ${VCPKG_INSTALL_CMD}
            WORKING_DIRECTORY ${VCPKG_ROOT}
            RESULT_VARIABLE VCPKG_RESULT
    )
endif ()

if (NOT VCPKG_RESULT)
    set_tool_chain("${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif ()