find_library(POPT_LIBRARY popt)

list(APPEND CORE_LIBRARIES ${POPT_LIBRARY})

file(GLOB_RECURSE UNIT_TESTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
foreach(File IN LISTS UNIT_TESTS)
  # 移除target的后缀
  string(REGEX REPLACE "\\.cpp$" "" TARGET "${File}")
  add_executable(${TARGET} ${TARGET}.cpp)
  
  target_link_libraries(${TARGET} ${CORE_LIBRARIES})
  target_link_libraries(${TARGET} OpenSmbC gssapi_krb5 krb5)
  link_directories("${CMAKE_BINARY_DIR}/lib")
  add_dependencies(${TARGET} OpenSmbC)
endforeach()

add_definitions(-Werror "-D_U_=__attribute__((unused))")
