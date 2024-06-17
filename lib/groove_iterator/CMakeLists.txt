
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build groove_iterator as a shared library
add_library(groove_iterator SHARED)
add_library(groove::groove_iterator ALIAS groove_iterator)

set(GROOVE_ITERATOR_INCLUDES
    include/groove_iterator/base_iterator.h
    include/groove_iterator/filter_iterator_template.h
    include/groove_iterator/map_iterator_template.h
    include/groove_iterator/peek_iterator_template.h
    include/groove_iterator/range_iterator_template.h
    )
set_target_properties(groove_iterator PROPERTIES PUBLIC_HEADER "${GROOVE_ITERATOR_INCLUDES}")

target_sources(groove_iterator PRIVATE
    src/base_iterator.cpp
    )

# set the library version
set_target_properties(groove_iterator PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(groove_iterator PROPERTIES MACOSX_RPATH TRUE)
endif()

# set the public header include path differently on the target depending on the interface
target_include_directories(groove_iterator PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

target_link_libraries(groove_iterator
    PUBLIC
    tempo::tempo_utils
    )

# install targets
install(TARGETS groove_iterator EXPORT groove-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/groove_iterator
    )

# add testing subdirectory
add_subdirectory(test)
