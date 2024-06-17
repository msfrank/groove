
# build groove_mount as a shared library
add_library(groove_mount SHARED)
add_library(groove::groove_mount ALIAS groove_mount)

add_custom_command (
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_mount/mount_service.grpc.pb.cc
      ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_mount/mount_service.grpc.pb.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_mount
    COMMAND
      ${PROTOBUF_PROTOC} -I${CMAKE_CURRENT_SOURCE_DIR}/share
      --grpc_out=${CMAKE_CURRENT_BINARY_DIR}/generated/groove_mount
      --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
      ${CMAKE_CURRENT_SOURCE_DIR}/share/mount_service.proto
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/mount_service.proto
)

add_custom_command (
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_mount/mount_service.pb.cc
      ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_mount/mount_service.pb.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_mount
    COMMAND
      ${PROTOBUF_PROTOC} -I${CMAKE_CURRENT_SOURCE_DIR}/share
      --cpp_out=${CMAKE_CURRENT_BINARY_DIR}/generated/groove_mount
      ${CMAKE_CURRENT_SOURCE_DIR}/share/mount_service.proto
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/mount_service.proto
)

set(CHORD_INVOKE_INCLUDES
    ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_mount/mount_service.grpc.pb.h
    ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_mount/mount_service.pb.h
)
set_target_properties(groove_mount PROPERTIES PUBLIC_HEADER "${CHORD_INVOKE_INCLUDES}")

target_sources(groove_mount PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_mount/mount_service.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_mount/mount_service.grpc.pb.h
    ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_mount/mount_service.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/generated/groove_mount/mount_service.pb.h
)

# set the library version
set_target_properties(groove_mount PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(groove_mount PROPERTIES MACOSX_RPATH TRUE)
endif()

# set the public header include path differently on the target depending on the interface
target_include_directories(groove_mount PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(groove_mount PUBLIC gRPC::grpc++)

# install targets
install(TARGETS groove_mount EXPORT groove-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/groove_mount
)

# install data
install(FILES share/mount_service.proto DESTINATION ${CMAKE_INSTALL_DATADIR}/groove_mount)

# add testing subdirectory
#add_subdirectory(test)
