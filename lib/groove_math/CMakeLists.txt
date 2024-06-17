
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build groove_math as a shared library
add_library(groove_math SHARED)
add_library(groove::groove_math ALIAS groove_math)

set(GROOVE_MATH_INCLUDES
    include/groove_math/average_function.h
    include/groove_math/base_reducer.h
    include/groove_math/math_result.h
    include/groove_math/math_types.h
    include/groove_math/maximum_function.h
    include/groove_math/minimum_function.h
    include/groove_math/reducer_template.h
    include/groove_math/reducer_traits.h
    include/groove_math/samplecount_function.h
    include/groove_math/sum_function.h
    )
set_target_properties(groove_math PROPERTIES PUBLIC_HEADER "${GROOVE_MATH_INCLUDES}")

target_sources(groove_math PRIVATE
    src/average_function.cpp
    src/base_reducer.cpp
    src/math_result.cpp
    src/math_types.cpp
    src/maximum_function.cpp
    src/minimum_function.cpp
    src/samplecount_function.cpp
    src/sum_function.cpp
    )

# set the library version
set_target_properties(groove_math PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(groove_math PROPERTIES MACOSX_RPATH TRUE)
endif()

# set the public header include path differently on the target depending on the interface
target_include_directories(groove_math PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

target_link_libraries(groove_math
    PUBLIC
    groove::groove_data
    groove::groove_iterator
    tempo::tempo_utils
    )

# install targets
install(TARGETS groove_math EXPORT groove-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/groove_math
    )

# add testing subdirectory
add_subdirectory(test)
