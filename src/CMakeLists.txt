aux_source_directory(. SRC_LIST)

add_executable(connPool ${SRC_LIST})

target_link_libraries(connPool mysqlclient mymuduo pthread)
