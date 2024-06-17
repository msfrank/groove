
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build groove_storage as a shared library
add_library(groove_storage SHARED)
add_library(groove::groove_storage ALIAS groove_storage)

set(GROOVE_STORAGE_INCLUDES
    include/groove_storage/dataset_synchronizer.h
    include/groove_storage/groove_shard.h
    include/groove_storage/groove_storage.h
    include/groove_storage/remote_collection.h
    include/groove_storage/shard_interval.h
    include/groove_storage/shard_state.h
    include/groove_storage/storage_client.h
    include/groove_storage/storage_result.h
    include/groove_storage/storage_types.h
    include/groove_storage/syncing_client.h
    include/groove_storage/sync_provider.h
    )
set_target_properties(groove_storage PROPERTIES PUBLIC_HEADER "${GROOVE_STORAGE_INCLUDES}")

# generate flatbuffer files for schema IDL
add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/include/groove_storage/generated/shard.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/groove_storage/generated
    COMMAND
      ${FLATC} --cpp --scoped-enums --gen-mutable --gen-object-api --filename-suffix ''
      -o ${CMAKE_CURRENT_BINARY_DIR}/include/groove_storage/generated
      ${CMAKE_CURRENT_SOURCE_DIR}/share/shard.fbs
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/shard.fbs
)

# generate header containing the shard schema
add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/include/groove_storage/generated/shard_schema.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/groove_storage/generated
    COMMAND
      tempo::tempo-bytes2code
      -g GROOVE_STORAGE_GENERATED_SCHEMA_SCHEMA_H -n groove_storage::shard::schema
      ${CMAKE_CURRENT_SOURCE_DIR}/share/shard.fbs
      ${CMAKE_CURRENT_BINARY_DIR}/include/groove_storage/generated/shard_schema.h
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/shard.fbs
)

target_sources(groove_storage PRIVATE
    src/dataset_synchronizer.cpp
    src/groove_shard.cpp
    src/groove_storage.cpp
    src/remote_collection.cpp
    src/shard_interval.cpp
    src/shard_state.cpp
    src/storage_client.cpp
    src/storage_result.cpp
    src/storage_types.cpp
    src/syncing_client.cpp
    src/sync_provider.cpp

    include/groove_storage/internal/shard_reader.h
    src/internal/shard_reader.cpp

    # auto-generated headers
    ${CMAKE_CURRENT_BINARY_DIR}/include/groove_storage/generated/shard.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/groove_storage/generated/shard_schema.h
)

# set the library version
set_target_properties(groove_storage PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(groove_storage PROPERTIES MACOSX_RPATH TRUE)
endif()

# set the public header include path differently on the target depending on the interface
target_include_directories(groove_storage PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

target_link_libraries(groove_storage
    PUBLIC
    groove::groove_data
    groove::groove_iterator
    groove::groove_model
    groove::groove_mount
    groove::groove_sync
    tempo::tempo_security
    tempo::tempo_utils
    absl::flat_hash_map
    PRIVATE
    flatbuffers::flatbuffers
    )

# install targets
install(TARGETS groove_storage EXPORT groove-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/groove_storage
    )

# install data
install(FILES share/shard.fbs DESTINATION ${CMAKE_INSTALL_DATADIR}/groove_storage)

# add testing subdirectory
add_subdirectory(test)
