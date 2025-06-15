
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # 添加调试信息 否则gdb无法命中断点
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")
endif()


include_directories(../include)

file(GLOB_RECURSE UNIT_TESTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
foreach(File IN LISTS UNIT_TESTS)
  # 移除target的后缀
  string(REGEX REPLACE "\\.cpp$" "" TARGET "${File}")
  add_executable(${TARGET} ${TARGET}.cpp)
  
  target_link_libraries(${TARGET}  PRIVATE pthread)
endforeach()
