
# vars used to enable subdirectories to extend the build of the vsgGIS library in a loose coupled way
set(EXTRA_DEFINES)
set(EXTRA_INCLUDES)

SET(HEADER_PATH ${CMAKE_SOURCE_DIR}/include/vsgGIS)

set(HEADERS
    ${HEADER_PATH}/gdal_utils.h
    ${HEADER_PATH}/meta_utils.h
    ${HEADER_PATH}/TileDatabase.h
 )

set(SOURCES
    gdal_utils.cpp
    meta_utils.cpp
    TileDatabase.cpp
)

add_library(vsgGIS ${HEADERS} ${SOURCES})

set_property(TARGET vsgGIS PROPERTY VERSION ${VSGGIS_VERSION_MAJOR}.${VSGGIS_VERSION_MINOR}.${VSGGIS_VERSION_PATCH})
set_property(TARGET vsgGIS PROPERTY SOVERSION ${VSGGIS_SOVERSION})
set_property(TARGET vsgGIS PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET vsgGIS PROPERTY CXX_STANDARD 17)

target_compile_definitions(vsgGIS PRIVATE ${EXTRA_DEFINES})

target_include_directories(vsgGIS PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/vsgGIS>
    $<INSTALL_INTERFACE:include>
    ${GDAL_INCLUDE_DIR}
    ${EXTRA_INCLUDES}
)

target_link_libraries(vsgGIS
    PUBLIC
        vsg::vsg
    PRIVATE
        ${GDAL_LIBRARY}
        ${EXTRA_LIBRARIES}
)

install(TARGETS vsgGIS ${INSTALL_TARGETS_DEFAULT_FLAGS})

if (BUILD_SHARED_LIBS)
    target_compile_definitions(vsgGIS INTERFACE VSGGIS_SHARED_LIBRARY)
endif()


install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/vsgGIS DESTINATION include)

vsg_add_cmake_support_files(
    CONFIG_TEMPLATE
        vsgGISConfig.cmake.in
)
