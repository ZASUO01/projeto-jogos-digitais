cmake_minimum_required(VERSION 3.26)
# Configurando o padrão c++
set(CMAKE_CXX_STANDARD 17)

# Configurando o nome e executável do projeto
set(PROJECT_NAME "line-casters")
project(${PROJECT_NAME})

find_package(SDL2 REQUIRED)
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
        Source/Main.cpp
        Client/Client.cpp
        Client/Client.h
        Client/ClientOperations.cpp
        Client/ClientOperations.h
        Client/InputData.h
        Network/Platforms.h
        Network/Packet.cpp
        Network/Packet.h
        Network/NetUtils.cpp
        Network/NetUtils.h
        Network/Socket.cpp
        Network/Socket.h
        Network/Logger.cpp
        Network/Logger.h
        Network/Addresses.cpp
        Network/Addresses.h
        Network/Defs.h)

target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::GLEW SDL2::SDL2 OpenGL::GL)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif ()