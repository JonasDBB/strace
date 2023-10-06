cmake_minimum_required(VERSION 3.24)
project(strace C)

set(CMAKE_C_STANDARD 11)

add_executable(strace)
add_subdirectory(src)
target_compile_options(strace PUBLIC
		-Wall
		-Werror
		-Wextra
		-g3
		-fsanitize=address
)
target_link_options(strace PUBLIC
		-fsanitize=address
)

target_include_directories(strace PUBLIC inc)
