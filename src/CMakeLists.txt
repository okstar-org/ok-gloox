file(GLOB gloox_SRCS *.cpp)

include_directories(${gloox_SOURCE_DIR})

# default is OFF
if (NOT BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS OFF)
endif ()

add_library(${PROJECT_NAME} ${gloox_SRCS})

target_link_libraries(${PROJECT_NAME} ${LIBS})

if (WIN32)
    if(CMAKE_BUILD_TYPE MATCHES Release)
        add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)
    elseif(CMAKE_BUILD_TYPE MATCHES Debug)
        add_definitions(-D_ITERATOR_DEBUG_LEVEL=2)        
    endif()  
    set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()



