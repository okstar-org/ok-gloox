file( GLOB gloox_SRCS *.cpp )

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D_ITERATOR_DEBUG_LEVEL=0")

add_library( gloox ${gloox_SRCS} )
target_link_libraries( gloox ${LIBS} )
include_directories( ${gloox_SOURCE_DIR} )
set_target_properties(gloox PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

