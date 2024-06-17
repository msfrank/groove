
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build groove_units as a shared library
add_library(groove_units SHARED)
add_library(groove::groove_units ALIAS groove_units)

set(GROOVE_UNITS_INCLUDES
    include/groove_units/magnitude.h
    include/groove_units/quantity.h
    include/groove_units/reference_system_manager.h
    include/groove_units/unicode_utils.h
    include/groove_units/unit_dimension.h
    include/groove_units/unit_reference_system.h
    include/groove_units/unit_scale.h
    include/groove_units/units_result.h
    )
set_target_properties(groove_units PROPERTIES PUBLIC_HEADER "${GROOVE_UNITS_INCLUDES}")

# generate antlr4 files for units grammar
add_custom_command (
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/src/UnitsLexer.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/src/UnitsLexer.h
      ${CMAKE_CURRENT_BINARY_DIR}/src/UnitsLexer.interp
      ${CMAKE_CURRENT_BINARY_DIR}/src/UnitsLexer.tokens
      ${CMAKE_CURRENT_BINARY_DIR}/src/UnitsParserBaseListener.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/src/UnitsParserBaseListener.h
      ${CMAKE_CURRENT_BINARY_DIR}/src/UnitsParser.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/src/UnitsParser.h
      ${CMAKE_CURRENT_BINARY_DIR}/src/UnitsParser.interp
      ${CMAKE_CURRENT_BINARY_DIR}/src/UnitsParser.tokens
      ${CMAKE_CURRENT_BINARY_DIR}/src/UnitsParserListener.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/src/UnitsParserListener.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/src
    COMMAND
      ${ANTLR}
      -o ${CMAKE_CURRENT_BINARY_DIR}/src
      -lib ${CMAKE_CURRENT_SOURCE_DIR}/share
      -Werror
      -Dlanguage=Cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/share/UnitsLexer.g4 ${CMAKE_CURRENT_SOURCE_DIR}/share/UnitsParser.g4
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/UnitsLexer.g4 ${CMAKE_CURRENT_SOURCE_DIR}/share/UnitsParser.g4
)

target_sources(groove_units PRIVATE
    src/magnitude.cpp
    src/quantity.cpp
    src/reference_system_manager.cpp
    src/unicode_utils.cpp
    src/unit_dimension.cpp
    src/unit_reference_system.cpp
    src/unit_scale.cpp
    src/units_result.cpp

    include/groove_units/internal/units_listener.h
    src/internal/units_listener.cpp

    # auto-generated sources
    ${CMAKE_CURRENT_BINARY_DIR}/src/UnitsLexer.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/src/UnitsLexer.h
    ${CMAKE_CURRENT_BINARY_DIR}/src/UnitsParserBaseListener.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/src/UnitsParserBaseListener.h
    ${CMAKE_CURRENT_BINARY_DIR}/src/UnitsParser.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/src/UnitsParser.h
    ${CMAKE_CURRENT_BINARY_DIR}/src/UnitsParserListener.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/src/UnitsParserListener.h
    )

# set the library version
set_target_properties(groove_units PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(groove_units PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(groove_units PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

# set the public header include path differently on the target depending on the interface
target_include_directories(groove_units PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

# include private headers
target_include_directories(groove_units PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src)

target_link_libraries(groove_units
    PUBLIC
    tempo::tempo_config
    tempo::tempo_utils
    PRIVATE
    antlr::antlr
    )

# install targets
install(TARGETS groove_units EXPORT groove-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/groove_units
    )

# install data
install(FILES share/UnitsLexer.g4 share/UnitsParser.g4 DESTINATION ${CMAKE_INSTALL_DATADIR}/groove_units)

# add testing subdirectory
add_subdirectory(test)
