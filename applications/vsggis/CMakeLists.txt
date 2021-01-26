set(SOURCES
    vsggis.cpp
)

add_executable(vsggis ${SOURCES})

target_include_directories(vsggis PRIVATE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

set_target_properties(vsggis PROPERTIES OUTPUT_NAME vsggis)

target_link_libraries(vsggis
    vsg::vsg
)

install(TARGETS vsggis
        RUNTIME DESTINATION bin
)
