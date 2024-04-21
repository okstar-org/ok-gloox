cmake_minimum_required(VERSION 3.22)

project( gloox )

if (WIN32)
    set(CMAKE_CXX_FALGS ${CMAKE_CXX_FALGS} /utf-8)
    add_definitions(-D_WIN -DUNICODE)

    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /utf-8")
        add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)
    endif ()

    if(CMAKE_BUILD_TYPE STREQUAL Release)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DNDEBUG /MT /utf-8")
    endif()
endif()

#include( FindZLIB )
#if( ZLIB_FOUND )
#    message(STATUS "Find ZLIB_LIBRARIES=${ZLIB_LIBRARIES}")
#    set( LIBS ${LIBS} ${ZLIB_LIBRARIES} )
#    set( INCLUDE_DIRS ${INCLUDE_DIRS} ${ZLIB_INCLUDE_DIR} )
#    write_file( ${CMAKE_CURRENT_SOURCE_DIR}/config.h "#define HAVE_ZLIB 1" APPEND)
#endif( ZLIB_FOUND)

#if (UNIX)
#    find_package(OpenSSL)
#    if(OpenSSL_FOUND)
#        message(STATUS "Find OpenSSL=${OPENSSL_SSL_LIBRARY}")
#        set(LIBS ${LIBS} ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
#    endif ()
#endif ()

include( CheckFunctionExists )

#option( SETSOCKOPTS_EXISTS "We have setsockopt()" ON )
check_function_exists( setsockopt SETSOCKOPT_EXISTS )

if( SETSOCKOPT_EXISTS )
    write_file( ${CMAKE_CURRENT_SOURCE_DIR}/config.h "#define HAVE_SETSOCKOPT 1")
endif( SETSOCKOPT_EXISTS )

message(STATUS "CMAKE_CXX_FALGS=${CMAKE_CXX_FALGS}")
message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

add_subdirectory( src )


