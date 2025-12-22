cmake_minimum_required(VERSION 3.10)
project(vk_vaapi_zero_copy_demo C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)

# system libs we need
pkg_check_modules(PC_VULKAN REQUIRED vulkan)      # libvulkan
pkg_check_modules(PC_LIBVA REQUIRED libva)        # libva
pkg_check_modules(PC_LIBDRM REQUIRED libdrm)      # libdrm
pkg_check_modules(PC_LIBAVCODEC REQUIRED libavcodec)
pkg_check_modules(PC_LIBAVUTIL REQUIRED libavutil)
pkg_check_modules(PC_LIBAVFORMAT REQUIRED libavformat)

include_directories(${PC_VULKAN_INCLUDE_DIRS} ${PC_LIBVA_INCLUDE_DIRS} ${PC_LIBDRM_INCLUDE_DIRS}
                    ${PC_LIBAVCODEC_INCLUDE_DIRS} ${PC_LIBAVUTIL_INCLUDE_DIRS} ${PC_LIBAVFORMAT_INCLUDE_DIRS})
link_directories(${PC_VULKAN_LIBRARY_DIRS} ${PC_LIBVA_LIBRARY_DIRS} ${PC_LIBDRM_LIBRARY_DIRS}
                 ${PC_LIBAVCODEC_LIBRARY_DIRS} ${PC_LIBAVUTIL_LIBRARY_DIRS} ${PC_LIBAVFORMAT_LIBRARY_DIRS})

add_compile_options(${PC_VULKAN_CFLAGS_OTHER} ${PC_LIBVA_CFLAGS_OTHER} ${PC_LIBDRM_CFLAGS_OTHER})

add_executable(vk_vaapi_demo
    src/main.cpp
    src/vulkan_export.cpp
    src/vaapi_import.cpp
    src/ffmpeg_vaapi_encode.cpp
)

target_link_libraries(vk_vaapi_demo
    ${PC_VULKAN_LIBRARIES}
    ${PC_LIBVA_LIBRARIES}
    ${PC_LIBDRM_LIBRARIES}
    ${PC_LIBAVCODEC_LIBRARIES}
    ${PC_LIBAVUTIL_LIBRARIES}
    ${PC_LIBAVFORMAT_LIBRARIES}
)
