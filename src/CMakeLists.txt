set(GAMEENGINE_INCLUDES
    game
    game/memory
    game/net
)

set(HOOKER_SRC hooker/dllmain.cpp hooker/hooker.cpp hooker/setupglobals.cpp hooker/setuphooks.cpp)

set(GAMEENGINE_SRC
    game/memory/alloc.cpp
    game/memory/newdel.cpp
    game/net/commbuff.cpp
    game/net/listener.cpp
    game/net/reliable.cpp
    game/net/unreliable.cpp
)

if(CMAKE_CONFIGURATION_TYPES)
    # Glob all the header files together to add to the project for msvc/xcode.
    # Not ideal as CMake won't notice if you add any until something else prompts a CMake run
    # but this is largely a convenience feature for MSVC/XCode users.
    file(GLOB_RECURSE GAMEENGINE_HEADERS "*.h")
    set(_GAMEENGINE_SOURCES ${GAMEENGINE_SRC})

    if(NOT STANDALONE)
        list(APPEND _GAMEENGINE_SOURCES ${HOOKER_SRC})
    endif()

    # Filter out headers from included 3rd party libraries.
    list(FILTER GAMEENGINE_HEADERS EXCLUDE REGEX "libs/.*")
    list(FILTER GAMEENGINE_HEADERS EXCLUDE REGEX "base/.*")

    foreach(_GAMEENGINE_SOURCE IN ITEMS ${_GAMEENGINE_SOURCES})
        get_filename_component(_GAMEENGINE_SOURCE_PATH "${_GAMEENGINE_SOURCE}" PATH)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" _GAMEENGINE_GROUP_PATH "${_GAMEENGINE_SOURCE_PATH}")
        string(REPLACE "/" "\\" _GAMEENGINE_GROUP_PATH "${_GAMEENGINE_GROUP_PATH}")
        source_group("Source Files\\${_GAMEENGINE_GROUP_PATH}" FILES "${_GAMEENGINE_SOURCE}")
    endforeach()

    foreach(_GAMEENGINE_HEADER IN ITEMS ${GAMEENGINE_HEADERS})
        get_filename_component(_GAMEENGINE_HEADER_PATH "${_GAMEENGINE_HEADER}" PATH)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" _GAMEENGINE_HGROUP_PATH "${_GAMEENGINE_HEADER_PATH}")
        string(REPLACE "/" "\\" _GAMEENGINE_HGROUP_PATH "${_GAMEENGINE_HGROUP_PATH}")
        source_group("Header Files\\${_GAMEENGINE_HGROUP_PATH}" FILES "${_GAMEENGINE_HEADER}")
    endforeach()
endif()

# Enable Thyme debug logging.
if(LOGGING)
    set(CAPTNLOG_LEVEL $<$<CONFIG:Debug>:5>$<$<CONFIG:RelWithDebInfo>:3>$<$<CONFIG:Release>:3>$<$<CONFIG:MinSizeRel>:3>)
else()
    set(CAPTNLOG_LEVEL 0)
endif()

# Enable Thyme debug assertions.
if(ASSERTIONS)
    set(CAPTNASSERT_LEVEL $<$<CONFIG:Debug>:2>$<$<CONFIG:RelWithDebInfo>:1>$<$<CONFIG:Release>:1>$<$<CONFIG:MinSizeRel>:1>)
else()
    set(CAPTNASSERT_LEVEL 0)
endif()

# Setup various included libraries
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../libs/captnlog ${CMAKE_CURRENT_BINARY_DIR}/captnlog EXCLUDE_FROM_ALL)

# Build and link the EXE or DLL.
if(STANDALONE)
    add_executable(remnant ${GAMEENGINE_SRC} ${GAMEENGINE_HEADERS})
    target_include_directories(remnant PUBLIC ${GAMEENGINE_INCLUDES})
else()
    add_library(remnant SHARED ${HOOKER_SRC} ${GAMEENGINE_SRC} ${GAMEENGINE_HEADERS})
    target_include_directories(remnant BEFORE PUBLIC hooker ${GAMEENGINE_INCLUDES})
    target_compile_definitions(remnant PUBLIC GAME_DLL=1)
endif()

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_libraries(remnant winmm)

    if(STANDALONE)
        target_link_libraries(remnant ws2_32)
    else()
        target_link_libraries(remnant wsock32) # Original links wsock32, not ws2_32
    endif()
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(remnant rt)
endif()

target_compile_definitions(remnant PUBLIC $<$<CONFIG:DEBUG>:GAME_DEBUG=1>)

# Enable using bools in place of bitfields for boolean class members.
# This is for matching the ABI of the original game.
if(NO_BITFIELDS)
    target_compile_definitions(remnant PUBLIC GAME_NO_BITFIELDS=1)
endif()

# Enable Thyme debug logging.
if(LOGGING)
    target_compile_definitions(remnant PUBLIC GAME_LOGGING=1)
endif()

# Enable Thyme debug assertions.
if(ASSERTIONS)
    target_compile_definitions(remnant PUBLIC GAME_ASSERTS=1)
endif()

target_link_libraries(remnant base captnlog)

if(DDRAW_FOUND)
    target_compile_definitions(remnant PRIVATE BUILD_WITH_DDRAW=1)
    target_link_libraries(remnant DirectX::ddraw)
endif()

if(DSOUND_FOUND)
    target_compile_definitions(remnant PRIVATE BUILD_WITH_DSOUND=1)
    target_link_libraries(remnant DirectX::dsound)
endif()

if(SDL2_FOUND)
    target_compile_definitions(remnant PRIVATE BUILD_WITH_SDL2=1)
    target_link_libraries(remnant SDL2::SDL2)
endif()

if(OPENAL_FOUND)
    target_compile_definitions(remnant PRIVATE BUILD_WITH_OPENAL=1)
    target_link_libraries(remnant ${OPENAL_LIBRARIES})
    target_include_directories(remnant PUBLIC ${OPENAL_INCLUDE_DIRS})
endif()
