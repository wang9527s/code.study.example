set(TARGET_NAME wlogger)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(..)

aux_source_directory(. SRCS)

add_executable (${TARGET_NAME} ${SRCS})
target_link_libraries(${TARGET_NAME} PRIVATE pthread)
