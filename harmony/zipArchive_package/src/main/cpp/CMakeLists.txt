# the minimum version of CMake.
cmake_minimum_required(VERSION 3.4.1)
project(zipArchive_package)
set(CMAKE_VERBOSE_MAKEFILE on)

#file(GLOB rnoh_zipArchive_package_SRC CONFIGURE_DEPENDS *.cpp)
#add_library(rnoh_zipArchive_package SHARED ${rnoh_zipArchive_package_SRC})
#target_include_directories(rnoh_zipArchive_package PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#target_link_libraries(rnoh_zipArchive_package PUBLIC rnoh)


set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(DEFINED PACKAGE_FIND_FILE)
    include(${PACKAGE_FIND_FILE})
endif()

include_directories(${NATIVERENDER_ROOT_PATH}
                    ${NATIVERENDER_ROOT_PATH}/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/napi)

add_library(nativi_minizip SHARED
            napi/minizip.cpp
            napi/minizipNapi.cpp
            napi/mz_zip_rw.cpp
            )

target_link_libraries(nativi_minizip PUBLIC libace_napi.z.so)
target_link_libraries(nativi_minizip PUBLIC libhilog_ndk.z.so)

#将三方库加入工程中
target_link_libraries(nativi_minizip PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/minizip-ng/${OHOS_ARCH}/lib/libminizip.so)
#将三方库的头文件加入工程中
target_include_directories(nativi_minizip PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/minizip-ng/${OHOS_ARCH}/include/minizip)