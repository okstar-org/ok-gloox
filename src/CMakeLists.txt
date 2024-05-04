file(GLOB gloox_SRCS *.cpp)

include_directories(${gloox_SOURCE_DIR})

message(STATUS "LIBS=${LIBS}")
add_library(${PROJECT_NAME} STATIC ${gloox_SRCS})
target_link_libraries(${PROJECT_NAME} ${LIBS})

if (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()



