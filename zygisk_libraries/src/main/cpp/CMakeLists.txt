# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("fpsunlocker")

message("Build type: ${CMAKE_BUILD_TYPE}")
message("Building for ABI: ${ANDROID_ABI}")

set(C_FLAGS "-Werror=format -fdata-sections -ffunction-sections -O2 -fno-omit-frame-pointer -fstack-protector-strong -fPIC")
set(CXX_FLAGS "${CXX_FLAGS} -fno-exceptions -fno-rtti -O2 -fno-omit-frame-pointer -fstack-protector-strong -fPIC")
set(LINKER_FLAGS "-Wl,--hash-style=both -Wl,--gc-sections -Wl,--as-needed")

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(C_FLAGS "${C_FLAGS} -O2 -fvisibility=hidden -fvisibility-inlines-hidden")
    set(LINKER_FLAGS "${LINKER_FLAGS} -Wl,-exclude-libs,ALL -Wl,--gc-sections -Wl,--strip-all")
else ()
    set(C_FLAGS "${C_FLAGS} -O0")
endif ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_FLAGS} ${CXX_FLAGS}")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${LINKER_FLAGS}")

# Output root dir
set(OUTPUT_LIB_ROOT "${CMAKE_SOURCE_DIR}/../../../../out")

function(copy_and_strip_library target output_name)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_LIB_ROOT}/${ANDROID_ABI}"
        COMMAND_EXPAND_LISTS
        VERBATIM
    )
    if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_STRIP} --strip-all --remove-section=.comment "$<TARGET_FILE:${target}>"
            COMMAND_EXPAND_LISTS
            VERBATIM
        )
    endif()
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${target}>" "${OUTPUT_LIB_ROOT}/${ANDROID_ABI}/${output_name}.so"
        COMMAND_EXPAND_LISTS
        VERBATIM
    )
endfunction()

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.
add_library(${CMAKE_PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        unlockfps.cpp)

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
        android
        log)

copy_and_strip_library(${PROJECT_NAME} ${ANDROID_ABI})