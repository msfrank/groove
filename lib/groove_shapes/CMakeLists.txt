
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build groove_shapes as a shared library
add_library(groove_shapes SHARED)
add_library(groove::groove_shapes ALIAS groove_shapes)

set(GROOVE_SHAPES_INCLUDES
    include/groove_shapes/bar_annotation_shape.h
    include/groove_shapes/bar_group_shape.h
    include/groove_shapes/bar_stack_shape.h
    include/groove_shapes/base_shape.h
    include/groove_shapes/event_table_shape.h
    include/groove_shapes/pie_annotation_shape.h
    include/groove_shapes/pie_group_shape.h
    include/groove_shapes/series_annotation_shape.h
    include/groove_shapes/series_group_shape.h
    include/groove_shapes/series_stack_shape.h
    include/groove_shapes/shape_provider.h
    include/groove_shapes/shapes_result.h
    include/groove_shapes/shape_traits.h
    include/groove_shapes/shape_types.h
    include/groove_shapes/summary_group_shape.h
    )
set_target_properties(groove_shapes PROPERTIES PUBLIC_HEADER "${GROOVE_SHAPES_INCLUDES}")

target_sources(groove_shapes PRIVATE
    src/bar_annotation_shape.cpp
    src/bar_group_shape.cpp
    src/bar_stack_shape.cpp
    src/base_shape.cpp
    src/event_table_shape.cpp
    src/pie_annotation_shape.cpp
    src/pie_group_shape.cpp
    src/series_annotation_shape.cpp
    src/series_group_shape.cpp
    src/series_stack_shape.cpp
    src/shape_provider.cpp
    src/shapes_result.cpp
    src/shape_types.cpp
    src/summary_group_shape.cpp
    )

# set the library version
set_target_properties(groove_shapes PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(groove_shapes PROPERTIES MACOSX_RPATH TRUE)
endif()

# set the public header include path differently on the target depending on the interface
target_include_directories(groove_shapes PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

target_link_libraries(groove_shapes
    PUBLIC
    groove::groove_data
    groove::groove_iterator
    groove::groove_math
    groove::groove_model
    groove::groove_units
    tempo::tempo_utils
    )

# install targets
install(TARGETS groove_shapes EXPORT groove-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/groove_shapes
    )

# add testing subdirectory
add_subdirectory(test)