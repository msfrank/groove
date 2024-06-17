
# do not run moc automatically.
set(CMAKE_AUTOMOC OFF)

# build GrooveRuntime static archive
add_library(GrooveRuntime OBJECT
    src/data_converter.cpp
    include/groove/data_converter.h
    src/groove.cpp
    include/groove/groove.h
    src/groove_convert_tool.cpp
    include/groove/groove_convert_tool.h
    src/groove_describe_tool.cpp
    include/groove/groove_describe_tool.h
    src/model_config_parser.cpp
    include/groove/model_config_parser.h
    src/schema_builder.cpp
    include/groove/schema_builder.h
    )

target_include_directories(GrooveRuntime PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_definitions(GrooveRuntime PRIVATE
    "FULL_VERSION=\"${FULL_VERSION}\""
    )

target_link_libraries(GrooveRuntime
    groove::groove_data
    groove::groove_io
    groove::groove_model
    tempo::tempo_command
    tempo::tempo_config
    tempo::tempo_utils
    ${SANITIZER_LIBS}
    ${PROFILER_LIBS}
    )

# build groove program
add_executable(groove src/main.cpp)

set_target_properties(groove PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

target_include_directories(groove PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(groove GrooveRuntime)

install(TARGETS groove EXPORT groove-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )