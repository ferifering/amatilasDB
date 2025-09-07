set(SOURCES execution_manager.cpp executor_aggregate.cpp execution_sort.cpp execution_limit.cpp executor_semi_join.cpp)
add_library(execution STATIC ${SOURCES})

target_link_libraries(execution system record transaction planner)