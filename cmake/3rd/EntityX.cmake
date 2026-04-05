include(FetchContent)

set(entityx_lib "entityx")

FetchContent_Declare(${entityx_lib}
        GIT_REPOSITORY https://github.com/alecthomas/entityx.git
        GIT_TAG origin/master
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        UPDATE_DISCONNECTED TRUE
)
set(ENTITYX_BUILD_SHARED OFF)
FetchContent_MakeAvailable(${entityx_lib})

