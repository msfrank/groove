
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build groove_data as a shared library
add_library(groove_data SHARED)
add_library(groove::groove_data ALIAS groove_data)

set(GROOVE_DATA_INCLUDES
    include/groove_data/array_utils.h
    include/groove_data/base_frame.h
    include/groove_data/base_vector.h
    include/groove_data/category.h
    include/groove_data/category_builder.h
    include/groove_data/category_double_vector.h
    include/groove_data/category_frame.h
    include/groove_data/category_int64_vector.h
    include/groove_data/category_string_vector.h
    include/groove_data/comparison_utils.h
    include/groove_data/data_result.h
    include/groove_data/data_types.h
    include/groove_data/double_frame.h
    include/groove_data/double_double_vector.h
    include/groove_data/double_int64_vector.h
    include/groove_data/double_string_vector.h
    include/groove_data/int64_double_vector.h
    include/groove_data/int64_int64_vector.h
    include/groove_data/int64_string_vector.h
    include/groove_data/int64_frame.h
    include/groove_data/table_utils.h
    )
set_target_properties(groove_data PROPERTIES PUBLIC_HEADER "${GROOVE_DATA_INCLUDES}")

target_sources(groove_data PRIVATE
    src/array_utils.cpp
    src/base_frame.cpp
    src/base_vector.cpp
    src/category.cpp
    src/category_builder.cpp
    src/category_double_vector.cpp
    src/category_frame.cpp
    src/category_int64_vector.cpp
    src/category_string_vector.cpp
    src/comparison_utils.cpp
    src/data_result.cpp
    src/data_types.cpp
    src/double_frame.cpp
    src/double_double_vector.cpp
    src/double_int64_vector.cpp
    src/double_string_vector.cpp
    src/int64_double_vector.cpp
    src/int64_int64_vector.cpp
    src/int64_string_vector.cpp
    src/int64_frame.cpp
    src/table_utils.cpp
    )

# set the library version
set_target_properties(groove_data PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(groove_data PROPERTIES MACOSX_RPATH TRUE)
endif()

# set the public header include path differently on the target depending on the interface
target_include_directories(groove_data PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

target_link_libraries(groove_data
    PUBLIC
    groove::groove_units
    tempo::tempo_utils
    Arrow::arrow_shared
    )

# install targets
install(TARGETS groove_data EXPORT groove-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/groove_data
    )

# add testing subdirectory
add_subdirectory(test)
