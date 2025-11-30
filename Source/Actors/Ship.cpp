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
           const float rotationForce,
           Vector3 color,
           bool isRedShip)
        : Actor(game)
        , mForwardSpeed(forwardForce)
        , mRotationForce(rotationForce)
        , mLaserCooldown(0.f)
        , mHeight(height)
        , mRotationCooldown(0.f)
        , mLives(3)
        , mShipColor(color)
        , mIsRedShip(isRedShip)
        , mDrawComponent(nullptr)
        , mRigidBodyComponent(nullptr)
        , mCircleColliderComponent(nullptr)
        , mWeapon(nullptr)
{
    // Inicializa actors de vida
    for (int i = 0; i < 3; i++) {
        mLivesActors[i] = nullptr;
    }
    
    std::vector<Vector2> vertices = CreateShipVertices();
    
    // Create filled component (drawn first, behind)
    new DrawComponent(this, vertices, 99, mShipColor, true);
    
    // Create outline component (drawn on top) - branco para contraste
    mDrawComponent = new DrawComponent(this, vertices, 100, Vector3(1.0f, 1.0f, 1.0f), false);
    
    mRigidBodyComponent = new RigidBodyComponent(this);
    mCircleColliderComponent = new CircleColliderComponent(this, mHeight / 3);

    // Cria partículas de tiro - nave vermelha usa partículas preenchidas e mais rápidas
    std::vector<Vector2> particleVertices = CreateParticleVertices(1.0f);
    if (mIsRedShip) {
        // Para nave vermelha: partículas mais rápidas e preenchidas
        mWeapon = new ParticleSystemComponent(this, particleVertices, 20, 10, SystemType::Shoot, Vector3(1.0f, 0.0f, 0.0f), true);
    } else {
        // Para nave ciano: partículas normais
        mWeapon = new ParticleSystemComponent(this, particleVertices, 20);
    }
    
    // Cria quadrados de vida
    UpdateLivesDisplay();
}

void Ship::OnProcessInput(const uint8_t* state)
{
    // Movimento direcional: W=up, S=down, A=left, D=right (nave 1)
    // Nave 2 usa setas: UP=up, DOWN=down, LEFT=left, RIGHT=right
    // Combinações diagonais são permitidas
    // Direções opostas cancelam o movimento
    // A rotação é automática baseada na direção do movimento
    
    bool up, down, left, right;
    
    if (mIsRedShip) {
        // Nave vermelha (nave 2) usa setas para movimento
        up = state[SDL_SCANCODE_UP];
        down = state[SDL_SCANCODE_DOWN];
        left = state[SDL_SCANCODE_LEFT];
        right = state[SDL_SCANCODE_RIGHT];
    } else {
        // Nave ciano (nave 1) usa WASD para movimento
        up = state[SDL_SCANCODE_W];
        down = state[SDL_SCANCODE_S];
        left = state[SDL_SCANCODE_A];
        right = state[SDL_SCANCODE_D];
    }
    
    // Se direções opostas estão pressionadas, não move nem rotaciona
    if ((up && down) || (left && right)) {
        mRigidBodyComponent->SetVelocity(Vector2::Zero);
    } else {
        Vector2 velocity = Vector2::Zero;
        Direction targetDirection = GetClosestDirection(GetRotation()); // Mantém direção atual se não houver movimento
        
        // Movimento vertical
        if (up && !down) {
            velocity.y = -mForwardSpeed; // Y negativo = para cima
        } else if (down && !up) {
            velocity.y = mForwardSpeed; // Y positivo = para baixo
        }
        
        // Movimento horizontal
        if (left && !right) {
            velocity.x = -mForwardSpeed; // X negativo = para esquerda
        } else if (right && !left) {
            velocity.x = mForwardSpeed; // X positivo = para direita
        }
        
        // Determina a direção baseada nas teclas pressionadas
        // A rotação é automática: a nave aponta na direção do movimento
        if (velocity.x != 0.0f || velocity.y != 0.0f) {
            if (up && !down) {
                // Movendo para cima (W)
                if (left && !right) {
                    targetDirection = Direction::UpLeft;      // 210° (cima-esquerda: W+A)
                } else if (right && !left) {
                    targetDirection = Direction::DownRight;   // 330° (cima-direita: W+D)
                } else {
                    targetDirection = Direction::Up;          // 270° (cima: W)
                }
            } else if (down && !up) {
                // Movendo para baixo (S)
                if (left && !right) {
                    targetDirection = Direction::DownLeft;    // 150° (baixo-esquerda: S+A)
                } else if (right && !left) {
                    targetDirection = Direction::UpRight;     // 30° (baixo-direita: S+D)
                } else {
                    targetDirection = Direction::Down;        // 90° (baixo: S)
                }
            } else {
                // Apenas movimento horizontal
                if (left && !right) {
                    targetDirection = Direction::Left;        // 180° (esquerda: A)
                } else if (right && !left) {
                    targetDirection = Direction::Right;       // 0° (direita: D)
                }
            }
            
            // Aplica rotação automaticamente baseada na direção do movimento
            SetRotation(DirectionToRadians(targetDirection));
        }
        
        // Normaliza velocidade diagonal para manter velocidade constante
        if (velocity.x != 0.0f && velocity.y != 0.0f) {
            float length = Math::Sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
            velocity.x = (velocity.x / length) * mForwardSpeed;
            velocity.y = (velocity.y / length) * mForwardSpeed;
        }
        
        mRigidBodyComponent->SetVelocity(velocity);
    }

    // Disparo: Nave 1: SPACE, Nave 2: ENTER ou RIGHT CTRL
    bool shoot = mIsRedShip ? (state[SDL_SCANCODE_RETURN] || state[SDL_SCANCODE_RCTRL]) : state[SDL_SCANCODE_SPACE];
    if (shoot) {
        if (mLaserCooldown <= 0.f) {
            float speed = mIsRedShip ? 24000.0f : 16000.0f; // Nave vermelha dispara mais rápido
            mWeapon->EmitParticle(1.0f, speed);
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

    mRotationCooldown -= deltaTime;
    if (mRotationCooldown <= 0) {
        mRotationCooldown = 0.f;
    }
    
    // Atualiza posição dos quadrados de vida para seguir a nave
    float spacing = 20.0f;
    float startX = -(spacing * (mLives - 1)) / 2.0f;
    float offsetY = mHeight / 2 + 20.0f;
    
    for (int i = 0; i < mLives; i++) {
        if (mLivesActors[i] != nullptr && mLivesActors[i]->GetState() == ActorState::Active) {
            float offsetX = startX + i * spacing;
            mLivesActors[i]->SetPosition(GetPosition() + Vector2(offsetX, -offsetY));
        }
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

std::vector<Vector2> Ship::CreateLifeSquareVertices() {
    std::vector<Vector2> vertices;
    float size = 8.0f; // Tamanho do quadrado de vida
    vertices.emplace_back(Vector2(-size, -size));
    vertices.emplace_back(Vector2(size, -size));
    vertices.emplace_back(Vector2(size, size));
    vertices.emplace_back(Vector2(-size, size));
    return vertices;
}

void Ship::UpdateLivesDisplay() {
    // Remove actors antigos de vida
    for (int i = 0; i < 3; i++) {
        if (mLivesActors[i] != nullptr) {
            mLivesActors[i]->SetState(ActorState::Destroy);
            mLivesActors[i] = nullptr;
        }
    }
    
    // Cria novos actors de vida baseados no número atual de vidas
    std::vector<Vector2> lifeSquare = CreateLifeSquareVertices();
    float spacing = 20.0f; // Espaçamento entre quadrados
    float startX = -(spacing * (mLives - 1)) / 2.0f; // Centraliza os quadrados
    
    for (int i = 0; i < mLives; i++) {
        // Cria um actor filho para cada vida
        Actor* lifeActor = new Actor(GetGame());
        lifeActor->SetState(ActorState::Active);
        
        // Posiciona acima da nave (será atualizado no OnUpdate)
        float offsetX = startX + i * spacing;
        float offsetY = mHeight / 2 + 20.0f;
        lifeActor->SetPosition(GetPosition() + Vector2(offsetX, -offsetY));
        
        // Cria componente de vida (preenchido, cor branca)
        new DrawComponent(lifeActor, lifeSquare, 101, Vector3(1.0f, 1.0f, 1.0f), true);
        
        mLivesActors[i] = lifeActor;
    }
}
