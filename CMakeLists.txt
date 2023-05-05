cmake_minimum_required(VERSION 3.21)

project(base64 C)

set(CMAKE_C_STANDARD 99)

add_compile_options(-Wall -Wextra -pedantic)

if (BASE64_USE_OPENSSL)
    add_definitions(-DBASE64_USE_OPENSSL)
    if(NOT OPENSSL_ROOT_DIR AND NOT OPENSSL_LIBRARIES)
        message(FATAL_ERROR "OPENSSL_INCLUDE_DIR and OPENSSL_LIBRARIES will be used!")
    elseif(NOT OPENSSL_ROOT_DIR)
        message(FATAL_ERROR "OPENSSL_ROOT_DIR will be used!")
    elseif(NOT OPENSSL_LIBRARIES)
        message(FATAL_ERROR "OPENSSL_LIBRARIES will be used!")
    else()
        message("OPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR}")
        message("OPENSSL_LIBRARIES=${OPENSSL_LIBRARIES}")
        set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
    endif()
endif()

# share library
add_library(base64 SHARED ${PROJECT_SOURCE_DIR}/src/base64.c)
if (BASE64_USE_OPENSSL)
    include_directories(base64 PRIVATE ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(base64 PRIVATE "-Wl,-rpath,@loader_path" -L${OPENSSL_LIBRARIES} -lcrypto)
    add_custom_command(TARGET base64 POST_BUILD
            COMMAND install_name_tool -change /usr/local/lib/libcrypto.3.dylib @rpath/libcrypto.3.dylib ${PROJECT_BINARY_DIR}/libbase64.dylib)
endif()

# static library
add_library(base64_static STATIC ${PROJECT_SOURCE_DIR}/src/base64.c)
if (BASE64_USE_OPENSSL)
    include_directories(base64_static PRIVATE ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(base64_static PRIVATE -L${OPENSSL_LIBRARIES} -lcrypto)
endif()
set_target_properties(base64_static PROPERTIES OUTPUT_NAME "base64")

# test
add_executable(base64Test ${PROJECT_SOURCE_DIR}/test/test_base64.c)
add_dependencies(base64Test base64)
include_directories(base64Test PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(base64Test PRIVATE "-Wl,-rpath,@loader_path" -L${PROJECT_BINARY_DIR} -lbase64)
