cmake_minimum_required(VERSION 3.26)
# Configurando o padrão c++
set(CMAKE_CXX_STANDARD 17)

# Configurando o nome e executável do projeto
set(PROJECT_NAME "lion-casters")
project(${PROJECT_NAME})

find_package(SDL2 2.32.2 EXACT REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

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
        Source/Renderer/Texture.cpp
        Source/Renderer/Texture.h
        Source/Renderer/Font.cpp
        Source/Renderer/Font.h
        Source/Renderer/VideoPlayer.cpp
        Source/Renderer/VideoPlayer.h
        Source/Renderer/AudioPlayer.cpp
        Source/Renderer/AudioPlayer.h
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
        Source/Components/GridDrawComponent.cpp
        Source/Components/GridDrawComponent.h
        Source/Components/LaserBeamComponent.cpp
        Source/Components/LaserBeamComponent.h
        Source/Components/TrailComponent.cpp
        Source/Components/TrailComponent.h
        Source/Actors/Ship.cpp
        Source/Actors/Ship.h
        Source/Actors/Floor.cpp
        Source/Actors/Floor.h
        Source/Actors/LaserBeam.cpp
        Source/Actors/LaserBeam.h
        Source/UI/UIElement.cpp
        Source/UI/UIElement.h
        Source/UI/UIImage.cpp
        Source/UI/UIImage.h
        Source/UI/UIText.cpp
        Source/UI/UIText.h
        Source/UI/UIButton.cpp
        Source/UI/UIButton.h
        Source/UI/UIRect.cpp
        Source/UI/UIRect.h
        Source/UI/UITriangle.cpp
        Source/UI/UITriangle.h
        Source/UI/UIVideo.cpp
        Source/UI/UIVideo.h
        Source/UI/Screens/UIScreen.cpp
        Source/UI/Screens/UIScreen.h
        Source/UI/Screens/MainMenu.cpp
        Source/UI/Screens/MainMenu.h
        Source/UI/Screens/GameOver.cpp
        Source/UI/Screens/GameOver.h
        Source/UI/Screens/HUD.cpp
        Source/UI/Screens/HUD.h
        Source/UI/Screens/OpeningScreen.cpp
        Source/UI/Screens/OpeningScreen.h
        Source/Main.cpp)

# Procurar por FFmpeg
find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h)

find_library(AVCODEC_LIBRARY avcodec)
find_library(AVFORMAT_LIBRARY avformat)
find_library(AVUTIL_LIBRARY avutil)
find_library(SWSCALE_LIBRARY swscale)

if(AVCODEC_INCLUDE_DIR AND AVFORMAT_INCLUDE_DIR AND AVUTIL_INCLUDE_DIR AND SWSCALE_INCLUDE_DIR
   AND AVCODEC_LIBRARY AND AVFORMAT_LIBRARY AND AVUTIL_LIBRARY AND SWSCALE_LIBRARY)
    target_include_directories(${PROJECT_NAME} PRIVATE 
        ${AVCODEC_INCLUDE_DIR}
        ${AVFORMAT_INCLUDE_DIR}
        ${AVUTIL_INCLUDE_DIR}
        ${SWSCALE_INCLUDE_DIR}
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        GLEW::GLEW 
        SDL2::SDL2 
        SDL2_image::SDL2_image 
        SDL2_ttf::SDL2_ttf 
        SDL2_mixer::SDL2_mixer
        OpenGL::GL
        ${AVCODEC_LIBRARY}
        ${AVFORMAT_LIBRARY}
        ${AVUTIL_LIBRARY}
        ${SWSCALE_LIBRARY}
    )
else()
    message(WARNING "FFmpeg libraries not found. Video playback will not work.")
    target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::GLEW SDL2::SDL2 SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf SDL2_mixer::SDL2_mixer OpenGL::GL)
endif()