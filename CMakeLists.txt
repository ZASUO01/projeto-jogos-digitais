cmake_minimum_required(VERSION 3.26)
# Configurando o padrão c++
set(CMAKE_CXX_STANDARD 17)

# Configurando o nome e executável do projeto
set(PROJECT_NAME "lion-casters")
project(${PROJECT_NAME})

#set(GLEW_ROOT "C:/Program Files/GLEW")
#set(GLEW_INCLUDE_DIRS "${GLEW_ROOT}/include")
#set(GLEW_LIBRARIES "${GLEW_ROOT}/lib/Release/x64/glew32.lib")
#
#find_package(SDL2_mixer REQUIRED)
#
#set(OPENGL "C:/Users/fabio/OneDrive/Documentos/opengl32/opengl32.dll")

find_package(SDL2 2.32.2 EXACT REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SDL2_mixer REQUIRED)

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
        Source/AudioSystem.cpp
        Source/AudioSystem.h
        Source/Main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::GLEW SDL2::SDL2 OpenGL::GL SDL2_mixer::SDL2_mixer)

#target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENGL} ${GLEW_LIBRARIES} SDL2::SDL2 SDL2_mixer::SDL2_mixer)