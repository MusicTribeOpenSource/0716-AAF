
cmake_minimum_required(VERSION 3.20)
project(IgniteDevelopment)
include_guard()
## CMake package management...
include(CPM.cmake)

# Add or Disable optional components
option( USE_VALUE_TREE_DEBUGGER "Value Tree Debugger" OFF )
option( IGNITE_VST_PLUGIN "IGNITE VST Plugin" OFF )
#Compile commands, useful for some IDEs like VS-Code
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

#Adds all the module sources so they appear correctly in the IDE
set_property(GLOBAL PROPERTY USE_FOLDERS YES)
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)

if(USE_VALUE_TREE_DEBUGGER)
  add_definitions(-DUSE_VALUE_TREE_DEBUGGER=0)
endif()

# Keyboard/mouse front-panel simulator for desktop builds without the MCU (FP_TEST_KEYBRD).
# Default on for macOS, where there is no other front-panel input path; opt in elsewhere.
if(APPLE)
    set(IGNITE_KEY_SIMULATOR_DEFAULT ON)
else()
    set(IGNITE_KEY_SIMULATOR_DEFAULT OFF)
endif()
option(IGNITE_KEY_SIMULATOR "Build the keyboard/mouse front-panel key simulator (desktop only)" ${IGNITE_KEY_SIMULATOR_DEFAULT})
if(IGNITE_KEY_SIMULATOR)
    add_definitions(-DFP_TEST_KEYBRD=1)
endif()

set(ENABLE_HARDWARE_TEST TRUE)

if(ENABLE_HARDWARE_TEST)
    add_definitions(-DENABLE_HARDWARE_TEST=1)
else()
    add_definitions(-DENABLE_HARDWARE_TEST=0)
endif()


if("${ENABLE_OPTICK}" STREQUAL "")
    set(USE_OPTICK_VAL 0)
else()
    set(USE_OPTICK_VAL ${ENABLE_OPTICK})
endif()

set(USE_VST 0)
set(USE_AU 0)

if(BUILD_VST)
    set(USE_VST 1)
endif()

# If we are using MSVC we want static runtime linkage
if (MSVC)
    # Adding standards conformance flag
    # Catches errors skipped in regular Windows build
    ## Link https: //learn.microsoft.com/en-us/cpp/build/reference/permissive-standards-conformance?view=msvc-170
    add_compile_options(/permissive-)

    set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_options(
        $<$<CONFIG:Release>:/MT> # Runtime library: Multi-threaded
        $<$<CONFIG:RelWithDebInfo>:/MT> # Runtime library: Multi-threaded                           
        $<$<CONFIG:Debug>:/MTd> # Runtime library: Multi-threaded Debug
	#$<$<CONFIG:Debug>:/MP> # Compile with Multiple Core Debug
        /MP /Gm- /Zi
        /wd4100
    )
endif()

# If we are compiling for macOS we want to target OS versions down to 10.11
if (APPLE)
    set (CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE INTERNAL "")
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

endif()

# Compile with werror in release builds
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
    CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -Wno-unknown-pragmas -ffast-math -Ofast" )
    #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Werror")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /WX")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(PATCH_FLAG_FILE "${CMAKE_BINARY_DIR}/juce_patch_applied.flag")

# Fetch JUCE from respective git repositories...
# IF you want to instead point it to a local version, you can invoke CMake with  -DCPM_JUCE_SOURCE="Path_To_JUCE_git_repo"
CPMAddPackage( 
    NAME JUCE
    GITHUB_REPOSITORY juce-framework/JUCE
    GIT_TAG 7.0.12
    DOWNLOAD_ONLY YES
)

if(JUCE_ADDED)
    message(STATUS "Downloaded JUCE repository into \"${JUCE_SOURCE_DIR}\"")
    add_subdirectory(${JUCE_SOURCE_DIR} ./cmake_build_juce)

    # Check if patch is already applied
    if(NOT EXISTS ${PATCH_FLAG_FILE})
	    # Apply patch to JUCE
	    execute_process(
		    COMMAND git apply ${CMAKE_SOURCE_DIR}/patch/juce_multitouch.patch
		    WORKING_DIRECTORY ${JUCE_SOURCE_DIR}
		    RESULT_VARIABLE patch_result
		    )
	    if(patch_result)
		    message(FATAL_ERROR "Failed to apply patch to JUCE: ${patch_result}")
	    else()
		    message(STATUS "Patch applied successfully to JUCE.")
		    file(WRITE ${PATCH_FLAG_FILE} "JUCE patch applied")
	    endif()
    else()
	    message(STATUS "Patch already applied to JUCE. Skipping patch step.")
    endif()

    # macOS desktop build: keep AppKit text-input calls on the main thread (see patch file).
    if(APPLE)
        set(MAC_PATCH_FLAG_FILE "${CMAKE_BINARY_DIR}/juce_mac_patch_applied.flag")
        if(NOT EXISTS ${MAC_PATCH_FLAG_FILE})
            execute_process(
                COMMAND git apply ${CMAKE_SOURCE_DIR}/patch/juce_mac_textinput_mainthread.patch
                WORKING_DIRECTORY ${JUCE_SOURCE_DIR}
                RESULT_VARIABLE mac_patch_result
            )
            if(mac_patch_result)
                message(FATAL_ERROR "Failed to apply macOS patch to JUCE: ${mac_patch_result}")
            else()
                message(STATUS "macOS patch applied successfully to JUCE.")
                file(WRITE ${MAC_PATCH_FLAG_FILE} "JUCE macOS patch applied")
            endif()
        else()
            message(STATUS "macOS patch already applied to JUCE. Skipping patch step.")
        endif()
    endif()
endif()

# Add JUCE directory...
add_subdirectory(timestamp)
add_subdirectory(modules)
add_subdirectory(apps)

# Add CPack support
set(CPACK_GENERATOR "7Z")
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_SOURCE_DIR}/.build_cache/dist)

set(CPACK_STRIP_FILES YES)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS YES)

set(CPACK_SOURCE_IGNORE_FILES
    /.git
    /dist
    /.*build.*
    /\\\\.DS_Store
    /JUCE
)
set(
    CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
)

include(CPack)
