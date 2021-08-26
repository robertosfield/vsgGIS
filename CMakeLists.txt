cmake_minimum_required(VERSION 3.7)

project(vsgGIS
    VERSION 0.0.0
    DESCRIPTION "Utility library adding GIS features to VulkanSceneGraph."
    LANGUAGES CXX
)
set(VSGGIS_SOVERSION 0)
SET(VSGGIS_RELEASE_CANDIDATE 0)

# set the use of C++17 globally as all examples require it
set(CMAKE_CXX_STANDARD 17)


# Find Vulkan and the VSG
if (VULKAN_SDK)
    set(ENV{VULKAN_SDK} ${VULKAN_SDK})
endif()

find_package(Vulkan REQUIRED)
find_package(vsg REQUIRED)
find_package(GDAL REQUIRED)

setup_build_vars()
setup_dir_vars()

add_target_clang_format(
    FILES
        include/vsgGIS/*.h
        src/vsgGIS/*.cpp
)
add_target_clobber()
add_target_cppcheck(
    FILES
        ${CMAKE_SOURCE_DIR}/src/vsgGIS/*.cpp
        ${CMAKE_SOURCE_DIR}/include/vsgGIS/*.h
    )
add_target_docs(
    FILES
        include
)
add_target_uninstall()

add_option_maintainer(
    PREFIX vsgGIS
    RCLEVEL VSGGIS_RELEASE_CANDIDATE
)

# source directories for vsgGIS library
add_subdirectory(src)
add_subdirectory(applications)

add_feature_summary()
