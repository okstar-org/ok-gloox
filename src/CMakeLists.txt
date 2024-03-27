file(GLOB gloox_SRCS *.cpp)

# 设置库类型（动态库或静态库）
option(BUILD_SHARED_LIBS "Build gloox as a shared library" ON)

if(BUILD_SHARED_LIBS)
  add_library(gloox SHARED ${gloox_SRCS})
else()
  add_library(gloox STATIC ${gloox_SRCS})
endif()

target_link_libraries(gloox ${LIBS})
include_directories(${gloox_SOURCE_DIR})
set_target_properties(gloox PROPERTIES MSVC_RUNTIME_LIBRARY
                                       "MultiThreaded$<$<CONFIG:Debug>:Debug>")

## 添加库文件的安装操作
#install(
#  TARGETS gloox
#  LIBRARY DESTINATION lib
#  ARCHIVE DESTINATION lib
#  RUNTIME DESTINATION bin)
#
## 添加头文件的安装操作
#install(
#  DIRECTORY ${gloox_SOURCE_DIR}
#  DESTINATION include
#  FILES_MATCHING
#  PATTERN "*.h"
#  PATTERN ".git" EXCLUDE
#  PATTERN "src/tests" EXCLUDE
#  PATTERN "build" EXCLUDE
#  PATTERN "ideas" EXCLUDE)
