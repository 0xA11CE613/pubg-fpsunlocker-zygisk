cmake_minimum_required(VERSION 3.4.1)

if (NOT DEFINED MODULE_NAME)
    message(FATAL_ERROR "MODULE_NAME is not set")
else ()
    project(${MODULE_NAME})
endif ()

message("Build type: ${CMAKE_BUILD_TYPE}")
message("Building for ABI: ${ANDROID_ABI}")

set(CMAKE_CXX_STANDARD 17)

set(LINKER_FLAGS "-ffixed-x18 -Wl,--hash-style=both")
set(C_FLAGS "-Werror=format -fdata-sections -ffunction-sections")
set(CXX_FLAGS "${CXX_FLAGS} -fno-exceptions -fno-rtti")

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

add_library(${MODULE_NAME} SHARED unlockfps.cpp module.h)
target_link_libraries(${MODULE_NAME} log)
copy_and_strip_library(${MODULE_NAME} ${ANDROID_ABI})