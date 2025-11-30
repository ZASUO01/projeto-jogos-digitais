cmake_minimum_required(VERSION 3.26)
# Configurando o padrão c++
set(CMAKE_CXX_STANDARD 17)

# Configurando o nome e executável do projeto
set(PROJECT_NAME "lion-casters")
project(${PROJECT_NAME})

find_package(SDL2 2.32.2 EXACT REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

# Procurar FFmpeg
find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h)
find_path(SWRESAMPLE_INCLUDE_DIR libswresample/swresample.h)

find_library(AVCODEC_LIBRARY avcodec)
find_library(AVFORMAT_LIBRARY avformat)
find_library(AVUTIL_LIBRARY avutil)
find_library(SWSCALE_LIBRARY swscale)
find_library(SWRESAMPLE_LIBRARY swresample)

if (AVCODEC_INCLUDE_DIR AND AVFORMAT_INCLUDE_DIR AND AVUTIL_INCLUDE_DIR AND SWSCALE_INCLUDE_DIR AND SWRESAMPLE_INCLUDE_DIR
    AND AVCODEC_LIBRARY AND AVFORMAT_LIBRARY AND AVUTIL_LIBRARY AND SWSCALE_LIBRARY AND SWRESAMPLE_LIBRARY)
    set(FFMPEG_FOUND TRUE)
    message(STATUS "FFmpeg found")
else()
    message(WARNING "FFmpeg not found - video playback will not work")
    set(FFMPEG_FOUND FALSE)
endif()

add_executable(${PROJECT_NAME}
        Source/Renderer/Shader.cpp
        Source/Renderer/Shader.h
        Source/Math.cpp
        Source/Math.h
        Source/Random.cpp
        Source/Random.h
        Source/Renderer/VertexArray.cpp
        Source/Renderer/VertexArray.h
        Source/Renderer/Renderer.cpp
        Source/Renderer/Renderer.h
        Source/Actors/Actor.cpp
        Source/Actors/Actor.h
        Source/Game.cpp
        Source/Game.h
        Source/Components/Component.cpp
        Source/Components/Component.h
        Source/Components/DrawComponent.cpp
        Source/Components/DrawComponent.h
        Source/Components/RigidBodyComponent.cpp
        Source/Components/RigidBodyComponent.h
        Source/Components/ParticleSystemComponent.cpp
        Source/Components/ParticleSystemComponent.h
        Source/Components/CircleColliderComponent.cpp
        Source/Components/CircleColliderComponent.h
        Source/Actors/Ship.cpp
        Source/Actors/Ship.h
        Source/Video/VideoPlayer.cpp
        Source/Video/VideoPlayer.h
        Source/Main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${AVCODEC_INCLUDE_DIR}
        ${AVFORMAT_INCLUDE_DIR}
        ${AVUTIL_INCLUDE_DIR}
        ${SWSCALE_INCLUDE_DIR}
        ${SWRESAMPLE_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE 
        GLEW::GLEW 
        SDL2::SDL2 
        OpenGL::GL)

if(FFMPEG_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE
            ${AVCODEC_LIBRARY}
            ${AVFORMAT_LIBRARY}
            ${AVUTIL_LIBRARY}
            ${SWSCALE_LIBRARY}
            ${SWRESAMPLE_LIBRARY})
endif()
