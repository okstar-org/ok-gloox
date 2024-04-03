file(GLOB gloox_SRCS *.cpp)

include_directories(${gloox_SOURCE_DIR})

message(STATUS "LIBS=${LIBS}")
add_library(gloox STATIC ${gloox_SRCS})
target_link_libraries(gloox ${LIBS})

#if (WIN32)
#    set_target_properties(gloox PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
#endif ()
