//
// Created by Lucas N. Ferreira on 03/08/23.
//

#include "Ship.h"
#include "../Game.h"
#include "../Components/CircleColliderComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponent.h"
#include "../Components/LaserBeamComponent.h"
#include "../Actors/LaserBeam.h"

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
        , mInvincibilityTimer(0.0f)
        , mLives(3)
        , mShipColor(color)
        , mIsRedShip(isRedShip)
        , mDrawComponent(nullptr)
        , mColliderDrawComponent(nullptr)
        , mRigidBodyComponent(nullptr)
        , mCircleColliderComponent(nullptr)
{
    for (int i = 0; i < 3; i++) {
        mLivesActors[i] = nullptr;
    }

    std::vector<Vector2> vertices = CreateShipVertices();

    new DrawComponent(this, vertices, 99, mShipColor, true);
    mDrawComponent = new DrawComponent(this, vertices, 100, Vector3(1.0f, 1.0f, 1.0f), false);

    mRigidBodyComponent = new RigidBodyComponent(this);
    float colliderRadius = mHeight * 1.0f;
    mCircleColliderComponent = new CircleColliderComponent(this, colliderRadius);

    std::vector<Vector2> circleVertices = CreateColliderCircleVertices(colliderRadius);
    Vector3 glowColor = mIsRedShip ? Vector3(1.0f, 0.3f, 0.3f) : Vector3(0.3f, 1.0f, 1.0f);
    mColliderDrawComponent = new ColliderDrawComponent(this, circleVertices, 97, glowColor);
    mColliderDrawComponent->SetVisible(true);

    UpdateLivesDisplay();
}

void Ship::OnProcessInput(const uint8_t* state)
{
    bool up, down, left, right;

    if (mIsRedShip) {
        up = state[SDL_SCANCODE_UP];
        down = state[SDL_SCANCODE_DOWN];
        left = state[SDL_SCANCODE_LEFT];
        right = state[SDL_SCANCODE_RIGHT];
    } else {
        up = state[SDL_SCANCODE_W];
        down = state[SDL_SCANCODE_S];
        left = state[SDL_SCANCODE_A];
        right = state[SDL_SCANCODE_D];
    }

    if ((up && down) || (left && right)) {
        mRigidBodyComponent->SetVelocity(Vector2::Zero);
    } else {
        Vector2 velocity = Vector2::Zero;
        Direction targetDirection = GetClosestDirection(GetRotation());

        if (up && !down) {
            velocity.y = -mForwardSpeed;
        } else if (down && !up) {
            velocity.y = mForwardSpeed;
        }

        if (left && !right) {
            velocity.x = -mForwardSpeed;
        } else if (right && !left) {
            velocity.x = mForwardSpeed;
        }

        if (velocity.x != 0.0f || velocity.y != 0.0f) {
            if (up && !down) {
                if (left && !right) {
                    targetDirection = Direction::UpLeft;
                } else if (right && !left) {
                    targetDirection = Direction::DownRight;
                } else {
                    targetDirection = Direction::Up;
                }
            } else if (down && !up) {
                if (left && !right) {
                    targetDirection = Direction::DownLeft;
                } else if (right && !left) {
                    targetDirection = Direction::UpRight;
                } else {
                    targetDirection = Direction::Down;
                }
            } else {
                if (left && !right) {
                    targetDirection = Direction::Left;
                } else if (right && !left) {
                    targetDirection = Direction::Right;
                }
            }

            SetRotation(DirectionToRadians(targetDirection));
        }

        if (velocity.x != 0.0f && velocity.y != 0.0f) {
            float length = Math::Sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
            velocity.x = (velocity.x / length) * mForwardSpeed;
            velocity.y = (velocity.y / length) * mForwardSpeed;
        }

        mRigidBodyComponent->SetVelocity(velocity);
    }

    bool shoot = mIsRedShip ? (state[SDL_SCANCODE_RETURN] || state[SDL_SCANCODE_RCTRL]) : state[SDL_SCANCODE_SPACE];
    if (shoot) {
        if (mLaserCooldown <= 0.f) {
            Vector3 laserColor = mIsRedShip ? Vector3(1.0f, 0.0f, 0.0f) : Vector3(0.0f, 1.0f, 0.0f);
            Vector2 laserStart = GetPosition() + GetForward() * (mHeight / 2.0f);
            new LaserBeam(GetGame(), laserStart, GetRotation(), laserColor, this);
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

    if (mInvincibilityTimer > 0.0f) {
        mInvincibilityTimer -= deltaTime;
        if (mInvincibilityTimer <= 0.0f) {
            mInvincibilityTimer = 0.0f;
        }

        float blinkRate = 0.1f;
        bool shouldBeVisible = (static_cast<int>(mInvincibilityTimer / blinkRate) % 2) == 0;

        if (mDrawComponent) {
            mDrawComponent->SetVisible(shouldBeVisible);
        }
        if (mColliderDrawComponent) {
            mColliderDrawComponent->SetVisible(shouldBeVisible);
        }
    } else {
        if (mDrawComponent) {
            mDrawComponent->SetVisible(true);
        }
        if (mColliderDrawComponent) {
            mColliderDrawComponent->SetVisible(true);
        }
    }
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
    float normalizedRotation = Math::Fmod(rotation, Math::TwoPi);
    if (normalizedRotation < 0.0f) {
        normalizedRotation += Math::TwoPi;
    }

    float degrees = Math::ToDegrees(normalizedRotation);
    degrees = Math::Fmod(degrees, 360.0f);
    if (degrees < 0.0f) {
        degrees += 360.0f;
    }

    float minDist = 360.0f;
    Direction closest = Direction::Right;

    float directions[] = {0.0f, 30.0f, 90.0f, 150.0f, 180.0f, 210.0f, 270.0f, 330.0f};
    Direction dirs[] = {Direction::Right, Direction::UpRight, Direction::Down, Direction::DownLeft,
                        Direction::Left, Direction::UpLeft, Direction::Up, Direction::DownRight};

    for (int i = 0; i < 8; ++i) {
        float dist = Math::Abs(degrees - directions[i]);
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
        case Direction::DownRight:  return Math::ToRadians(330.0f);
        case Direction::DownLeft:  return Math::ToRadians(150.0f);
        case Direction::Left:       return Math::ToRadians(180.0f);
        case Direction::UpLeft:     return Math::ToRadians(210.0f);
        case Direction::Up:         return Math::ToRadians(270.0f);
        default:                    return 0.0f;
    }
}

Ship::Direction Ship::GetNextDirection(Direction current, bool clockwise) const {
    Direction sequence[] = {
        Direction::Right,
        Direction::UpRight,
        Direction::Down,
        Direction::DownLeft,
        Direction::Left,
        Direction::UpLeft,
        Direction::Up,
        Direction::DownRight
    };

    int currentIndex = -1;
    for (int i = 0; i < 8; ++i) {
        if (sequence[i] == current) {
            currentIndex = i;
            break;
        }
    }

    if (currentIndex == -1) {
        return current;
    }

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
    float size = 8.0f;
    vertices.emplace_back(Vector2(-size, -size));
    vertices.emplace_back(Vector2(size, -size));
    vertices.emplace_back(Vector2(size, size));
    vertices.emplace_back(Vector2(-size, size));
    return vertices;
}

std::vector<Vector2> Ship::CreateColliderCircleVertices(float radius) {
    std::vector<Vector2> vertices;
    constexpr int numSides = 32;
    constexpr float step = Math::TwoPi / numSides;

    for (int i = 0; i < numSides; i++) {
        float angle = i * step;
        vertices.emplace_back(Vector2(radius * Math::Cos(angle), radius * Math::Sin(angle)));
    }

    return vertices;
}

void Ship::UpdateLivesDisplay() {
    for (int i = 0; i < 3; i++) {
        if (mLivesActors[i] != nullptr) {
            mLivesActors[i]->SetState(ActorState::Destroy);
            mLivesActors[i] = nullptr;
        }
    }

    std::vector<Vector2> lifeSquare = CreateLifeSquareVertices();
    float spacing = 20.0f;
    float startX = -(spacing * (mLives - 1)) / 2.0f;

    for (int i = 0; i < mLives; i++) {
        Actor* lifeActor = new Actor(GetGame());
        lifeActor->SetState(ActorState::Active);

        float offsetX = startX + i * spacing;
        float offsetY = mHeight / 2 + 20.0f;
        lifeActor->SetPosition(GetPosition() + Vector2(offsetX, -offsetY));

        new DrawComponent(lifeActor, lifeSquare, 101, Vector3(1.0f, 1.0f, 1.0f), true);
        mLivesActors[i] = lifeActor;
    }
}
