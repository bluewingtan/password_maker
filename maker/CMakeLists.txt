cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

set(TARGET_NAME maker)
if(NOT CMAKE_BUILD_TYPE) 

    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif(NOT CMAKE_BUILD_TYPE)

if(NOT CMAKE_BINARY_DIR) 
    set(CMAKE_BINARY_DIR "./bin")
endif(NOT CMAKE_BINARY_DIR)

add_executable(${TARGET_NAME})

target_compile_features(${TARGET_NAME} PUBLIC cxx_std_11 cxx_constexpr)

target_include_directories(${TARGET_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
    $<INSTALL_INTERFACE:include>
)

target_sources(${TARGET_NAME} PUBLIC "src/maker.cpp")

# Copy configuration files and dist files
add_custom_command(
    TARGET ${TARGET_NAME}
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copy configuration files and dist files."
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/config ${CMAKE_BINARY_DIR}/${TARGET_NAME}/config
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/dist ${CMAKE_BINARY_DIR}/${TARGET_NAME}/dist
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${TARGET_NAME}/generated
)