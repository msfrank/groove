
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build GrooveAgentRuntime static archive
add_library(GrooveAgentRuntime OBJECT
    src/channel_monitor.cpp
    include/groove_agent/channel_monitor.h
    src/dataset_config_parser.cpp
    include/groove_agent/dataset_config_parser.h
    src/storage_collection.cpp
    include/groove_agent/storage_collection.h
    src/mount_service.cpp
    include/groove_agent/mount_service.h
    src/storage_supervisor.cpp
    include/groove_agent/storage_supervisor.h
    src/sync_service.cpp
    include/groove_agent/sync_service.h
    src/groove_agent.cpp
    include/groove_agent/groove_agent.h
    )

target_include_directories(GrooveAgentRuntime PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )

# add the grpc generated includes path to the list of include directories
target_include_directories(GrooveAgentRuntime AFTER PRIVATE "${GRPC_INCLUDE_GENS_DIR}")

target_link_libraries(GrooveAgentRuntime PUBLIC
    groove::groove_io
    groove::groove_model
    groove::groove_mount
    groove::groove_storage
    groove::groove_sync
    tempo::tempo_command
    tempo::tempo_utils
    Arrow::arrow_shared
    gRPC::grpc++
    gRPC::grpcpp_channelz
    uv::uv
    )

# build groove-agent program
add_executable(groove-agent src/main.cpp)

set_target_properties(groove-agent PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

target_include_directories(groove-agent PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(groove-agent PUBLIC GrooveAgentRuntime)

# install targets
install(TARGETS groove-agent EXPORT groove-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

# add testing subdirectory
add_subdirectory(test)
