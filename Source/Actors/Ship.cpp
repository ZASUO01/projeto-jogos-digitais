//
// Created by Lucas N. Ferreira on 03/08/23.
//

#include "Ship.h"
#include "../Game.h"
#include "../Components/CircleColliderComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponent.h"
#include "../Components/ParticleSystemComponent.h"

Ship::Ship(Game* game,
           const float height,
           const float forwardForce,
           const float rotationForce)
        : Actor(game)
        , mForwardSpeed(forwardForce)
        , mRotationForce(rotationForce)
        , mLaserCooldown(0.f)
        , mHeight(height)
        , mBurnCooldown(0.f)
        , mRotationCooldown(0.f)
        , mDrawComponent(nullptr)
        , mRigidBodyComponent(nullptr)
        , mCircleColliderComponent(nullptr)
        , mWeapon(nullptr)
        , mTurbine(nullptr)
{
    std::vector<Vector2> vertices = CreateShipVertices();
    
    // Create filled bright cyan component (drawn first, behind) - more visible
    new DrawComponent(this, vertices, 99, Vector3(0.0f, 1.0f, 1.0f), true); // Bright cyan
    
    // Create bright outline component (drawn on top) - white/cyan for visibility
    mDrawComponent = new DrawComponent(this, vertices, 100, Vector3(1.0f, 1.0f, 1.0f), false); // White outline
    
    mRigidBodyComponent = new RigidBodyComponent(this);
    mCircleColliderComponent = new CircleColliderComponent(this, mHeight / 3);

    std::vector<Vector2> particleVertices = CreateParticleVertices(1.0f);
    mWeapon = new ParticleSystemComponent(this, particleVertices, 20);

    std::vector<Vector2> turbineVertices = CreateParticleVertices(1.0f);
    mTurbine = new ParticleSystemComponent(this, turbineVertices, 20, 10, SystemType::Fire);
}

void Ship::OnProcessInput(const uint8_t* state)
{
    // Movimento para frente - enquanto W estiver pressionado (sem aceleração)
    if (state[SDL_SCANCODE_W]) {
        Vector2 velocity;
        velocity.x = mForwardSpeed * Math::Cos(GetRotation());
        velocity.y = mForwardSpeed * Math::Sin(GetRotation());

        mRigidBodyComponent->SetVelocity(velocity);

        if (mBurnCooldown <= 0.f) {
            mTurbine->EmitParticle(0.1f, 25000);
            mBurnCooldown = 0.2f;
        }
    } else {
        // Para imediatamente quando soltar a tecla
        mRigidBodyComponent->SetVelocity(Vector2::Zero);
    }

    // Rotação baseada em 8 direções fixas
    if (state[SDL_SCANCODE_A] && mRotationCooldown <= 0.f) {
        Direction currentDir = GetClosestDirection(GetRotation());
        Direction nextDir = GetNextDirection(currentDir, false); // anti-horário
        SetRotation(DirectionToRadians(nextDir));
        mRotationCooldown = 0.15f; // Cooldown para evitar múltiplas rotações no mesmo frame
    }

    if (state[SDL_SCANCODE_D] && mRotationCooldown <= 0.f) {
        Direction currentDir = GetClosestDirection(GetRotation());
        Direction nextDir = GetNextDirection(currentDir, true); // horário
        SetRotation(DirectionToRadians(nextDir));
        mRotationCooldown = 0.15f; // Cooldown para evitar múltiplas rotações no mesmo frame
    }

    // Disparo
    if (state[SDL_SCANCODE_SPACE]) {
        if (mLaserCooldown <= 0.f) {
            mWeapon->EmitParticle(1  , 16000);
            mLaserCooldown = 0.2f;
        }
    }
}

void Ship::OnUpdate(float deltaTime)
{
    mLaserCooldown -= deltaTime;
    if (mLaserCooldown <= 0) {
        mLaserCooldown = 0.f;
    }

    mBurnCooldown -= deltaTime;
    if (mBurnCooldown <= 0) {
        mBurnCooldown = 0.f;
    }

    mRotationCooldown -= deltaTime;
    if (mRotationCooldown <= 0) {
        mRotationCooldown = 0.f;
    }
}

std::vector<Vector2> Ship::CreateShipVertices() {
    std::vector<Vector2> vertices;

    float step = mHeight / 4;
    vertices.emplace_back(Vector2(2 * step, 0.0));
    vertices.emplace_back(Vector2(2 * -step, 2 * step));
    vertices.emplace_back(Vector2(-step, step));
    vertices.emplace_back(Vector2(-step, -step));
    vertices.emplace_back(Vector2(2* -step, 2 * -step));

    return vertices;
}

std::vector<Vector2> Ship::CreateParticleVertices(float size) {
    std::vector<Vector2> vertices;

    vertices.emplace_back(Vector2(-size, -size));
    vertices.emplace_back(Vector2(size, -size));
    vertices.emplace_back(Vector2(size, size));
    vertices.emplace_back(Vector2(-size, size));

    return vertices;
}

Ship::Direction Ship::GetClosestDirection(float rotation) const {
    // Normaliza o ângulo para o range [0, 2π)
    float normalizedRotation = Math::Fmod(rotation, Math::TwoPi);
    if (normalizedRotation < 0.0f) {
        normalizedRotation += Math::TwoPi;
    }
    
    // Converte para graus
    float degrees = Math::ToDegrees(normalizedRotation);
    
    // Normaliza para [0, 360)
    degrees = Math::Fmod(degrees, 360.0f);
    if (degrees < 0.0f) {
        degrees += 360.0f;
    }
    
    // Direções válidas em graus: 0, 30, 90, 150, 180, 210, 270, 330
    // Encontra a direção mais próxima
    float minDist = 360.0f;
    Direction closest = Direction::Right;
    
    float directions[] = {0.0f, 30.0f, 90.0f, 150.0f, 180.0f, 210.0f, 270.0f, 330.0f};
    Direction dirs[] = {Direction::Right, Direction::UpRight, Direction::Down, Direction::DownLeft,
                        Direction::Left, Direction::UpLeft, Direction::Up, Direction::DownRight};
    
    for (int i = 0; i < 8; ++i) {
        float dist = Math::Abs(degrees - directions[i]);
        // Considera também a distância pelo outro lado do círculo
        if (dist > 180.0f) {
            dist = 360.0f - dist;
        }
        if (dist < minDist) {
            minDist = dist;
            closest = dirs[i];
        }
    }
    
    return closest;
}

float Ship::DirectionToRadians(Direction dir) const {
    switch (dir) {
        case Direction::Right:      return Math::ToRadians(0.0f);
        case Direction::UpRight:    return Math::ToRadians(30.0f);
        case Direction::Down:       return Math::ToRadians(90.0f);
        case Direction::DownRight:  return Math::ToRadians(330.0f);  // ou -30°
        case Direction::DownLeft:  return Math::ToRadians(150.0f);
        case Direction::Left:       return Math::ToRadians(180.0f);
        case Direction::UpLeft:     return Math::ToRadians(210.0f);
        case Direction::Up:         return Math::ToRadians(270.0f);
        default:                    return 0.0f;
    }
}

Ship::Direction Ship::GetNextDirection(Direction current, bool clockwise) const {
    // Sequência das direções em ordem horária (começando de Right = 0°)
    // Ordem dos ângulos: 0°, 30°, 90°, 150°, 180°, 210°, 270°, 330°
    Direction sequence[] = {
        Direction::Right,      // 0°
        Direction::UpRight,   // 30°
        Direction::Down,       // 90°
        Direction::DownLeft,   // 150°
        Direction::Left,       // 180°
        Direction::UpLeft,     // 210°
        Direction::Up,         // 270°
        Direction::DownRight   // 330° (-30°)
    };
    
    // Encontra o índice atual
    int currentIndex = -1;
    for (int i = 0; i < 8; ++i) {
        if (sequence[i] == current) {
            currentIndex = i;
            break;
        }
    }
    
    if (currentIndex == -1) {
        return current; // Se não encontrar, retorna a mesma direção
    }
    
    // Calcula o próximo índice
    int nextIndex;
    if (clockwise) {
        nextIndex = (currentIndex + 1) % 8;
    } else {
        nextIndex = (currentIndex - 1 + 8) % 8;
    }
    
    return sequence[nextIndex];
}
