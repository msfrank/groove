
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build groove_model as a shared library
add_library(groove_model SHARED)
add_library(groove::groove_model ALIAS groove_model)

set(GROOVE_MODEL_INCLUDES
    include/groove_model/abstract_dataset.h
    include/groove_model/abstract_page_cache.h
    include/groove_model/abstract_page_store.h
    include/groove_model/base_column.h
    include/groove_model/base_page.h
    include/groove_model/category_column_iterator.h
    include/groove_model/column_traits.h
    include/groove_model/column_walker.h
    include/groove_model/conversion_utils.h
    include/groove_model/double_column_iterator.h
    include/groove_model/groove_database.h
    include/groove_model/groove_model.h
    include/groove_model/groove_schema.h
    include/groove_model/indexed_column_template.h
    include/groove_model/indexed_column_writer_template.h
    include/groove_model/indexed_page_template.h
    include/groove_model/indexed_variant_column.h
    include/groove_model/int64_column_iterator.h
    include/groove_model/model_result.h
    include/groove_model/model_types.h
    include/groove_model/model_walker.h
    include/groove_model/namespace_walker.h
    include/groove_model/page_id.h
    include/groove_model/page_traits.h
    include/groove_model/persistent_caching_page_store.h
    include/groove_model/rocksdb_store.h
    include/groove_model/schema_attr.h
    include/groove_model/schema_attr_parser.h
    include/groove_model/schema_attr_writer.h
    include/groove_model/schema_column.h
    include/groove_model/schema_model.h
    include/groove_model/schema_namespace.h
    include/groove_model/schema_state.h
    include/groove_model/schema_walker.h
    include/groove_model/shared_string_buffer.h
    include/groove_model/variant_key.h
    include/groove_model/variant_value.h
    )
set_target_properties(groove_model PROPERTIES PUBLIC_HEADER "${GROOVE_MODEL_INCLUDES}")

# generate flatbuffer files for schema IDL
add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/include/groove_model/generated/schema.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/groove_model/generated
    COMMAND
      ${FLATC} --cpp --scoped-enums --gen-mutable --gen-object-api --filename-suffix ''
      -o ${CMAKE_CURRENT_BINARY_DIR}/include/groove_model/generated
      ${CMAKE_CURRENT_SOURCE_DIR}/share/schema.fbs
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/schema.fbs
)

# generate header containing the schema schema
add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/include/groove_model/generated/schema_schema.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/groove_model/generated
    COMMAND
      tempo::tempo-bytes2code
      -g GROOVE_MODEL_GENERATED_SCHEMA_SCHEMA_H -n groove_model::schema::schema
      ${CMAKE_CURRENT_SOURCE_DIR}/share/schema.fbs
      ${CMAKE_CURRENT_BINARY_DIR}/include/groove_model/generated/schema_schema.h
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/schema.fbs
)

target_sources(groove_model PRIVATE
    src/base_column.cpp
    src/base_page.cpp
    src/category_column_iterator.cpp
    src/column_walker.cpp
    src/conversion_utils.cpp
    src/double_column_iterator.cpp
    src/groove_database.cpp
    src/groove_model.cpp
    src/groove_schema.cpp
    src/indexed_variant_column.cpp
    src/int64_column_iterator.cpp
    src/model_result.cpp
    src/model_types.cpp
    src/model_walker.cpp
    src/namespace_walker.cpp
    src/page_id.cpp
    src/persistent_caching_page_store.cpp
    src/rocksdb_store.cpp
    src/schema_attr.cpp
    src/schema_attr_parser.cpp
    src/schema_attr_writer.cpp
    src/schema_column.cpp
    src/schema_model.cpp
    src/schema_namespace.cpp
    src/schema_state.cpp
    src/schema_walker.cpp
    src/shared_string_buffer.cpp
    src/variant_key.cpp
    src/variant_value.cpp

    include/groove_model/internal/schema_reader.h
    src/internal/schema_reader.cpp

    # auto-generated headers
    ${CMAKE_CURRENT_BINARY_DIR}/include/groove_model/generated/schema.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/groove_model/generated/schema_schema.h
    )

# set the library version
set_target_properties(groove_model PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(groove_model PROPERTIES MACOSX_RPATH TRUE)
endif()

# set the public header include path differently on the target depending on the interface
target_include_directories(groove_model PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

target_link_libraries(groove_model
    PUBLIC
    groove::groove_data
    groove::groove_iterator
    tempo::tempo_utils
    Arrow::arrow_shared
    Boost::headers
    rocksdb::rocksdb
    PRIVATE
    flatbuffers::flatbuffers
    )

# install targets
install(TARGETS groove_model EXPORT groove-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/groove_model
    )

# install data
install(FILES share/schema.fbs DESTINATION ${CMAKE_INSTALL_DATADIR}/groove_model)

# add testing subdirectory
add_subdirectory(test)
