cmake_minimum_required(VERSION 3.22)

project( ok-gloox )

if (WIN32)
    set(CMAKE_CXX_FALGS ${CMAKE_CXX_FALGS} /utf-8)
    add_definitions(-D_WIN -DUNICODE)

    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /utf-8")
    endif ()

    if(CMAKE_BUILD_TYPE STREQUAL Release)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DNDEBUG /MT /utf-8")
    endif()
    add_definitions(/UNICODE)
endif()

include( FindZLIB )
if( ZLIB_FOUND )
    message(STATUS "Find ZLIB_LIBRARIES=${ZLIB_LIBRARIES}")
    set( LIBS ${LIBS} ${ZLIB_LIBRARIES} )
    set( INCLUDE_DIRS ${INCLUDE_DIRS} ${ZLIB_INCLUDE_DIR} )
    write_file( ${CMAKE_CURRENT_SOURCE_DIR}/config.h "#define HAVE_ZLIB 1" APPEND)
endif( ZLIB_FOUND)

#if (UNIX)
#    find_package(OpenSSL)
#    if(OpenSSL_FOUND)
#        message(STATUS "Find OpenSSL=${OPENSSL_SSL_LIBRARY}")
#        set(LIBS ${LIBS} ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
#        write_file( ${CMAKE_CURRENT_SOURCE_DIR}/config.h "#define HAVE_OPENSSL 1" APPEND)
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

include_directories(.)
add_subdirectory( src )


get_filename_component(proj_loc "src" REALPATH)

set(proj_inc_dir ${CMAKE_INSTALL_INCLUDEDIR}/ok-gloox)

target_include_directories(ok-gloox
PUBLIC
    $<BUILD_INTERFACE:${proj_loc}>
    $<INSTALL_INTERFACE:${proj_inc_dir}>
)

configure_file(
        ${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        @ONLY
)

include(GNUInstallDirs)
# 添加库文件的安装操作
install(
        TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)

# 添加头文件的安装操作
install(
        DIRECTORY ${PROJECT_SOURCE_DIR}/src/
        DESTINATION include/${PROJECT_NAME}
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN ".git" EXCLUDE
        PATTERN "src/tests" EXCLUDE
        PATTERN "build" EXCLUDE
        PATTERN "ideas" EXCLUDE)


install(
        TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
        EXPORT ${PROJECT_NAME}Targets
        NAMESPACE ok-gloox::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(
        FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
