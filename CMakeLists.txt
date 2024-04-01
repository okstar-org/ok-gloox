cmake_minimum_required(VERSION 3.22)

project( gloox )

if (WIN32 AND CMAKE_BUILD_TYPE STREQUAL Debug)
add_definitions(-D_ITERATOR_DEBUG_LEVEL=0) 
endif()

add_subdirectory( src )


include( CheckFunctionExists )

#option( SETSOCKOPTS_EXISTS "We have setsockopt()" ON )
check_function_exists( setsockopt SETSOCKOPT_EXISTS )

if( SETSOCKOPT_EXISTS )
        write_file( ${CMAKE_CURRENT_SOURCE_DIR}/config.h "#define HAVE_SETSOCKOPT 1")
endif( SETSOCKOPT_EXISTS )

#include( FindZLIB )
#if( ZLIB_FOUND )
#    set( LIBS ${LIBS} ${ZLIB_LIBRARIES} )
#    set( INCLUDE_DIRS ${INCLUDE_DIRS} ${ZLIB_INCLUDE_DIR} )
    write_file( ${CMAKE_CURRENT_SOURCE_DIR}/config.h "#define HAVE_ZLIB 0" APPEND )
#endif( ZLIB_FOUND)
