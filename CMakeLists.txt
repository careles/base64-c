cmake_minimum_required(VERSION 3.21)

project(base64 VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

add_compile_options(-Wall -Wextra -pedantic)

if (BASE64_USE_OPENSSL) # 使用openssl进行编码/解码
    add_definitions(-DBASE64_USE_OPENSSL)
    if(OPENSSL_ROOT_DIR AND OPENSSL_LIBRARIES) # 指定openssl库
        set(CMAKE_PREFIX_PATH "${OPENSSL_ROOT_DIR}" ${CMAKE_PREFIX_PATH})
        find_package(OpenSSL REQUIRED)
    endif()
    find_package(OpenSSL) # 查找系统路径下的openssl库
    if(NOT OPENSSL_FOUND)
        # 使用子模块的openssl库
        execute_process(
                COMMAND git submodule update --init
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                RESULT_VARIABLE git_result        )
        if (NOT git_result EQUAL "0")
            message(FATAL_ERROR "Failed to download Git submodule.")
        endif()
        set(OPENSSL_ROOT_DIR ${PROJECT_SOURCE_DIR}/openssl)
        set(OPENSSL_LIBRARIES ${PROJECT_SOURCE_DIR}/openssl)
        set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
    endif()
    message("OPENSSL_ROOT_DIR = ${OPENSSL_ROOT_DIR}")
    message("OPENSSL_LIBRARIES = ${OPENSSL_LIBRARIES}")
    message("OPENSSL_INCLUDE_DIR = ${OPENSSL_INCLUDE_DIR}")
endif()

# share library
add_library(base64 SHARED ${PROJECT_SOURCE_DIR}/src/base64.c)
if (BASE64_USE_OPENSSL)
    add_dependencies(base64 openssl) # 将openssl设为依赖
    include_directories(base64 PRIVATE ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(base64 PRIVATE -L${OPENSSL_LIBRARIES} -lcrypto)
    add_custom_command(TARGET base64 POST_BUILD
            COMMAND install_name_tool -change /usr/local/lib/libcrypto.3.dylib
            @rpath/libcrypto.3.dylib ${PROJECT_BINARY_DIR}/libbase64.dylib)
endif()

# static library
add_library(base64_static STATIC ${PROJECT_SOURCE_DIR}/src/base64.c)
if (BASE64_USE_OPENSSL)
    add_dependencies(base64_static openssl_static)
    include_directories(base64_static PRIVATE ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(base64_static PRIVATE -L${OPENSSL_LIBRARIES} -lcrypto)
endif()
set_target_properties(base64_static PROPERTIES OUTPUT_NAME "base64")

# test
add_executable(base64Test ${PROJECT_SOURCE_DIR}/test/test_base64.c)
add_dependencies(base64Test base64)
include_directories(base64Test PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(base64Test PRIVATE "-Wl,-rpath,@loader_path"
        -L${PROJECT_BINARY_DIR} -lbase64)

# static test
add_executable(base64StaticTest ${PROJECT_SOURCE_DIR}/test/test_base64.c)
add_dependencies(base64StaticTest base64)
include_directories(base64StaticTest PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(base64StaticTest PRIVATE ${PROJECT_BINARY_DIR}/libbase64.a)

add_custom_target(openssl
        COMMAND test -f "${OPENSSL_ROOT_DIR}/libcrypto.dylib" ||
        make WORKING_DIRECTORY ${OPENSSL_ROOT_DIR})
add_dependencies(openssl openssl_config)

add_custom_target(openssl_static
        COMMAND test -f "${OPENSSL_ROOT_DIR}/libcrypto.a" ||
        make WORKING_DIRECTORY ${OPENSSL_ROOT_DIR})
add_dependencies(openssl_static openssl_config)

add_custom_target(openssl_config
        COMMAND test -f "${OPENSSL_ROOT_DIR}/Makefile" || ./Configure)

# debug
add_executable(base64Debug ${PROJECT_SOURCE_DIR}/test/test_base64.c ${PROJECT_SOURCE_DIR}/src/base64.c)
include_directories(base64Debug PRIVATE ${PROJECT_SOURCE_DIR}/src)
if (BASE64_USE_OPENSSL)
    add_dependencies(base64Debug openssl_static)
    include_directories(base64_static PRIVATE ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(base64Debug PRIVATE ${OPENSSL_ROOT_DIR}/libcrypto.a)
endif()
