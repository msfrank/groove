
# build groove_sync as a shared library
add_library(groove_sync SHARED)
add_library(groove::groove_sync ALIAS groove_sync)

add_custom_command (
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_sync/sync_service.grpc.pb.cc
      ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_sync/sync_service.grpc.pb.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_sync
    COMMAND
      ${PROTOBUF_PROTOC} -I${CMAKE_CURRENT_SOURCE_DIR}/share
      --grpc_out=${CMAKE_CURRENT_BINARY_DIR}/generated/groove_sync
      --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
      ${CMAKE_CURRENT_SOURCE_DIR}/share/sync_service.proto
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/sync_service.proto
)

add_custom_command (
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_sync/sync_service.pb.cc
      ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_sync/sync_service.pb.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_sync
    COMMAND
      ${PROTOBUF_PROTOC} -I${CMAKE_CURRENT_SOURCE_DIR}/share
      --cpp_out=${CMAKE_CURRENT_BINARY_DIR}/generated/groove_sync
      ${CMAKE_CURRENT_SOURCE_DIR}/share/sync_service.proto
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/sync_service.proto
)

set(CHORD_INVOKE_INCLUDES
    ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_sync/sync_service.grpc.pb.h
    ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_sync/sync_service.pb.h
)
set_target_properties(groove_sync PROPERTIES PUBLIC_HEADER "${CHORD_INVOKE_INCLUDES}")

target_sources(groove_sync PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_sync/sync_service.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_sync/sync_service.grpc.pb.h
    ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_sync/sync_service.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_sync/sync_service.pb.h
)

# set the library version
set_target_properties(groove_sync PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(groove_sync PROPERTIES MACOSX_RPATH TRUE)
endif()

# set the public header include path differently on the target depending on the interface
target_include_directories(groove_sync PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(groove_sync PUBLIC gRPC::grpc++)

# install targets
install(TARGETS groove_sync EXPORT groove-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/groove_sync
)

# install data
install(FILES share/sync_service.proto DESTINATION ${CMAKE_INSTALL_DATADIR}/groove_sync)

# add testing subdirectory
#add_subdirectory(test)
