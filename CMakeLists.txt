cmake_minimum_required(VERSION 1.0)
project(RTSP)

#set(CXX_FLAGES "-g -w -std=c++11")
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -g -w -std=c++11 -Wall")
set(CMAKE_SYSTEM_PROCESSOR "arm")
#set(CMAKE_CXX_FLAGS "-g -w -std=c++11")

#set(PWDIR /home/yanghe/teddy/project_app/rtsp_s/)
set(PWDIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(TOOLSCHAIN_PATH /opt/hisi-linux/x86-arm/aarch64-himix200-linux)
SET(CMAKE_C_COMPILER ${TOOLSCHAIN_PATH}/bin/aarch64-himix200-linux-gcc)
SET(CMAKE_CXX_COMPILER ${TOOLSCHAIN_PATH}/bin/aarch64-himix200-linux-g++)

include_directories(
${PWDIR}
${PWDIR}/en_de_ts
)

aux_source_directory(${PWDIR} SRC_LIST)
aux_source_directory(${PWDIR}/en_de_ts TS_CRC_LIST)

add_executable(rtsp_s ${SRC_LIST} ${TS_CRC_LIST})

target_link_libraries(rtsp_s pthread)
