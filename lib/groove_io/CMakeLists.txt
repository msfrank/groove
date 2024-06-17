
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build groove_io as a shared library
add_library(groove_io SHARED)
add_library(groove::groove_io ALIAS groove_io)

set(GROOVE_IO_INCLUDES
    include/groove_io/dataset_file.h
    include/groove_io/dataset_reader.h
    include/groove_io/dataset_writer.h
    include/groove_io/frame_walker.h
    include/groove_io/groove_index.h
    include/groove_io/index_frame.h
    include/groove_io/index_state.h
    include/groove_io/index_vector.h
    include/groove_io/index_walker.h
    include/groove_io/io_result.h
    include/groove_io/io_types.h
    include/groove_io/vector_walker.h
    )
set_target_properties(groove_io PROPERTIES PUBLIC_HEADER "${GROOVE_IO_INCLUDES}")

# generate flatbuffer files for index IDL
add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/include/groove_io/generated/index.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/groove_io/generated
    COMMAND
      ${FLATC} --cpp --scoped-enums --gen-mutable --gen-object-api --filename-suffix ''
      -o ${CMAKE_CURRENT_BINARY_DIR}/include/groove_io/generated
      ${CMAKE_CURRENT_SOURCE_DIR}/share/index.fbs
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/index.fbs
)

# generate header containing the index schema
add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/include/groove_io/generated/index_schema.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/groove_io/generated
    COMMAND
      tempo::tempo-bytes2code
      -g GROOVE_IO_GENERATED_SCHEMA_SCHEMA_H -n groove_io::schema::index
      ${CMAKE_CURRENT_SOURCE_DIR}/share/index.fbs
      ${CMAKE_CURRENT_BINARY_DIR}/include/groove_io/generated/index_schema.h
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/index.fbs
)

target_sources(groove_io PRIVATE
    src/dataset_file.cpp
    src/dataset_reader.cpp
    src/dataset_writer.cpp
    src/frame_walker.cpp
    src/groove_index.cpp
    src/index_frame.cpp
    src/index_state.cpp
    src/index_vector.cpp
    src/index_walker.cpp
    src/io_result.cpp
    src/vector_walker.cpp

    include/groove_io/internal/index_reader.h
    src/internal/index_reader.cpp

    # auto-generated headers
    ${CMAKE_CURRENT_BINARY_DIR}/include/groove_io/generated/index.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/groove_io/generated/index_schema.h
    )

# set the library version
set_target_properties(groove_io PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(groove_io PROPERTIES MACOSX_RPATH TRUE)
endif()

# set the public header include path differently on the target depending on the interface
target_include_directories(groove_io PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

target_link_libraries(groove_io
    PUBLIC
    groove::groove_data
    groove::groove_iterator
    groove::groove_model
    tempo::tempo_utils
    Arrow::arrow_shared
    flatbuffers::flatbuffers
    )

# install targets
install(TARGETS groove_io EXPORT groove-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/groove_io
    )

# install data
install(FILES share/index.fbs DESTINATION ${CMAKE_INSTALL_DATADIR}/groove_io)

# add testing subdirectory
add_subdirectory(test)