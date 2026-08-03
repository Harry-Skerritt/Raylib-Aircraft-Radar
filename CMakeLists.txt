cmake_minimum_required(VERSION 3.31)
project(Radar C)

set(CMAKE_C_STANDARD 11)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

find_package(CURL REQUIRED)

include(FetchContent)

FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG        master
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        cjson
        GIT_REPOSITORY  https://github.com/DaveGamble/cJSON.git
        GIT_TAG         v1.7.18
)


FetchContent_MakeAvailable(raylib)
FetchContent_MakeAvailable(cjson)

add_executable(${PROJECT_NAME} MACOSX_BUNDLE main.c)

target_link_libraries(${PROJECT_NAME} PRIVATE
        raylib
        CURL::libcurl
        cjson
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${cjson_SOURCE_DIR}
)

if (APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
            "-framework OpenGL"
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
            "-framework SystemConfiguration"
            "-framework Security"
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.harryskerritt.flightradar"
            MACOSX_BUNDLE_BUNDLE_NAME "FlightRadar"
            MACOSX_BUNDLE_ICON_FILE "icon.icns"
    )

    set_source_files_properties(icon.icns PROPERTIES MACOSX_BUNDLE_LOCATION_PATH "Resources")
    target_sources(${PROJECT_NAME} PRIVATE icon.icns)
endif ()