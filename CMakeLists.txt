cmake_minimum_required(VERSION 3.15)
project(sic-xe-assembler)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER clang++)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g -O0")
# Uncomment to enable address sanitizer
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

# Source files
set(SOURCES
				src/main.cpp
				src/pass_1.cpp
				src/pass_2.cpp
				src/table_store.cpp
				src/utilities.cpp
)

# Executable
add_executable(assembler ${SOURCES})

# Custom target for run
add_custom_target(run
				COMMAND ${CMAKE_BINARY_DIR}/assembler
				DEPENDS assembler
				WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
				COMMENT "Running assembler"
)

# Custom target for test
add_custom_target(test
				COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/test.sic ${CMAKE_BINARY_DIR}/test.sic
				COMMAND ${CMAKE_BINARY_DIR}/assembler test.sic
				COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/noncsect.sic ${CMAKE_BINARY_DIR}/noncsect.sic
				COMMAND ${CMAKE_BINARY_DIR}/assembler noncsect.sic
				COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/fig2.11.sic ${CMAKE_BINARY_DIR}/fig2.11.sic
				COMMAND ${CMAKE_BINARY_DIR}/assembler fig2.11.sic
				DEPENDS assembler
				WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
				COMMENT "Running test"
)
