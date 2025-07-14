cmake_minimum_required(VERSION 3.31)
project(medi_cloud_recvsrt)

find_package(PkgConfig REQUIRED)

pkg_check_modules(AVFORMAT REQUIRED libavformat)
pkg_check_modules(AVCODEC REQUIRED libavcodec)
pkg_check_modules(AVUTIL REQUIRED libavutil)

pkg_check_modules(RABBITMQ REQUIRED librabbitmq)

pkg_check_modules(JSON REQUIRED nlohmann_json)

set(CMAKE_CXX_STANDARD 23)

include_directories(${AVFORMAT_INCLUDE_DIRS})
include_directories(${AVCODEC_INCLUDE_DIRS})
include_directories(${AVUTIL_INCLUDE_DIRS})

include_directories(${RABBITMQ_INCLUDE_DIRS})

include_directories(${JSON_INCLUDE_DIRS})

add_executable(medi_cloud_recvsrt src/srt_downloader.cpp
        src/ostream_ctx.cpp
        src/ostream_ctx.h
        src/srt_input.cpp
        src/srt_input.h
        src/util.cpp
        src/util.h
        src/srt_downloader.h
        src/main.cpp
        src/settings.cpp
        src/settings.h
        src/rabbitmq_client.cpp
        src/rabbitmq_client.h
        src/messages.cpp
        src/messages.h)

target_link_libraries(medi_cloud_recvsrt ${AVFORMAT_LIBRARIES})
target_link_libraries(medi_cloud_recvsrt ${AVCODEC_LIBRARIES})
target_link_libraries(medi_cloud_recvsrt ${AVUTIL_LIBRARIES})

target_link_libraries(medi_cloud_recvsrt ${RABBITMQ_LIBRARIES})

target_link_libraries(medi_cloud_recvsrt ${JSON_LIBRARIES})
