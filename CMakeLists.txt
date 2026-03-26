cmake_minimum_required(VERSION 3.16)

project(ImageViewer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
)


string(TIMESTAMP YEAR "%Y")
string(TIMESTAMP MONTH "%m")
string(TIMESTAMP DAY "%d")
string(TIMESTAMP HOUR "%H")

# Convert hour (00–23) to decimal
math(EXPR HOUR_DEC "${HOUR}")

# Manual hex conversion (0–15 only needed realistically)
set(HEX_DIGITS "0;1;2;3;4;5;6;7;8;9;A;B;C;D;E;F")

if(HOUR_DEC LESS 16)
    list(GET HEX_DIGITS ${HOUR_DEC} HOUR_HASH)
else()
    # fallback for 16–23 (optional, simple version)
    math(EXPR HIGH "${HOUR_DEC} / 16")
    math(EXPR LOW "${HOUR_DEC} % 16")
    list(GET HEX_DIGITS ${HIGH} HIGH_HEX)
    list(GET HEX_DIGITS ${LOW} LOW_HEX)
    set(HOUR_HASH "${HIGH_HEX}${LOW_HEX}")
endif()

set(APP_VERSION "${YEAR}.${MONTH}.${DAY}.${HOUR_HASH}")

message(STATUS "App version: ${APP_VERSION}")



# Windows resource file
if(WIN32)
    list(APPEND SOURCES app.rc)
endif()

add_executable(viewer WIN32 ${SOURCES})

set_target_properties(viewer PROPERTIES
    OUTPUT_NAME_DEBUG debug_viewer
)

# -------------------------
# Windows (MSVC build)
# -------------------------
if(WIN32)

    set(SDL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libs/include)
    set(SDL_LIB_DIR ${CMAKE_SOURCE_DIR}/libs/lib)
    set(SDL_BIN_DIR ${CMAKE_SOURCE_DIR}/libs/bin)

    target_include_directories(viewer PRIVATE
        ${SDL_INCLUDE_DIR}
    )

    target_link_directories(viewer PRIVATE
        ${SDL_LIB_DIR}
    )

    target_link_libraries(viewer
        SDL2
        SDL2main
        SDL2_image
        SDL2_ttf
        SDL2_gfx
        dwmapi
    )

    if(MSVC)
        target_compile_options(viewer PRIVATE
            $<$<CONFIG:Release>:/O2>
            $<$<CONFIG:Debug>:/Od /Zi /Wall>
        )

        target_link_options(viewer PRIVATE
            $<$<CONFIG:Debug>:/DEBUG>
        )
    endif()

    set(SDL_DLLS
        SDL2.dll
        SDL2_image.dll
        SDL2_ttf.dll
        SDL2_gfx.dll
    )

    foreach(DLL ${SDL_DLLS})
        if(EXISTS "${SDL_BIN_DIR}/${DLL}")
            add_custom_command(TARGET viewer POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${SDL_BIN_DIR}/${DLL}
                $<TARGET_FILE_DIR:viewer>
            )
        endif()
    endforeach()

endif()

# -------------------------
# Linux build
# -------------------------
if(UNIX AND NOT APPLE)

    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_ttf REQUIRED)

    find_library(SDL2_GFX_LIBRARY SDL2_gfx)

    target_link_libraries(viewer
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_ttf::SDL2_ttf
        ${SDL2_GFX_LIBRARY}
    )

    target_compile_options(viewer PRIVATE
        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:Debug>:-O0 -g>
        )

endif()
target_compile_definitions(viewer PRIVATE APP_VERSION="${APP_VERSION}")
# -------------------------
# Copy resources for build
# -------------------------
add_custom_command(TARGET viewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/res
    $<TARGET_FILE_DIR:viewer>/
)

# -------------------------
# Install rules (Linux)
# -------------------------
install(TARGETS viewer
    RUNTIME DESTINATION bin
)

# Install resources next to the executable
install(DIRECTORY ${CMAKE_SOURCE_DIR}/res/
    DESTINATION share/imageviewer
)

# -------------------------
# CPack configuration
# -------------------------
set(CPACK_PACKAGE_NAME "imageviewer")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_CONTACT "your@email.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Simple SDL2 Image Viewer")

set(CPACK_GENERATOR "DEB;RPM;TGZ")

# Debian dependencies
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "libsdl2-2.0-0, libsdl2-image-2.0-0, libsdl2-ttf-2.0-0, libsdl2-gfx-1.0-0"
)

include(CPack)