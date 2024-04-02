cmake_minimum_required(VERSION 3.22)

project( gloox )

if (WIN32 AND CMAKE_BUILD_TYPE STREQUAL Debug)
add_definitions(-D_ITERATOR_DEBUG_LEVEL=0) 
endif()

include( FindZLIB )
if( ZLIB_FOUND )
    message(STATUS "Find ZLIB_LIBRARIES=${ZLIB_LIBRARIES}")
    set( LIBS ${LIBS} ${ZLIB_LIBRARIES} )
    set( INCLUDE_DIRS ${INCLUDE_DIRS} ${ZLIB_INCLUDE_DIR} )
    write_file( ${CMAKE_CURRENT_SOURCE_DIR}/config.h "#define HAVE_ZLIB 1" APPEND)
endif( ZLIB_FOUND)

find_package(OpenSSL)
if(OpenSSL_FOUND)
    message(STATUS "Find OpenSSL")
    set(LIBS ${LIBS} OpenSSL::SSL OpenSSL::Crypto)
endif ()


include( CheckFunctionExists )

#option( SETSOCKOPTS_EXISTS "We have setsockopt()" ON )
check_function_exists( setsockopt SETSOCKOPT_EXISTS )

if( SETSOCKOPT_EXISTS )
    write_file( ${CMAKE_CURRENT_SOURCE_DIR}/config.h "#define HAVE_SETSOCKOPT 1")
endif( SETSOCKOPT_EXISTS )


add_subdirectory( src )


