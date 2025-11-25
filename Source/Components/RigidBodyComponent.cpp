//
// Created by Lucas N. Ferreira on 08/09/23.
//

#include <SDL.h>
#include "RigidBodyComponent.h"



#include "../Actors/Actor.h"
#include "../Game.h"

RigidBodyComponent::RigidBodyComponent(class Actor* owner, float mass, int updateOrder)
        :Component(owner, updateOrder)
        ,mMass(mass)
        ,mAngularSpeed(0.0f)
        ,mVelocity(Vector2::Zero)
        ,mAcceleration(Vector2::Zero)
{

}

void RigidBodyComponent::ApplyForce(const Vector2 &force)
{
        mAcceleration.x += force.x / mMass;
        mAcceleration.y += force.y / mMass;
}

void RigidBodyComponent::Update(float deltaTime)
{
        Vector2 position = mOwner->GetPosition();

        mVelocity.x += mAcceleration.x * deltaTime;
        if (mVelocity.x > MAX_VELOCITY) {
                mVelocity.x = MAX_VELOCITY;
        }else if (mVelocity.x < -MAX_VELOCITY) {
                mVelocity.x = -MAX_VELOCITY;
        }else {
                if (Math::Abs(mVelocity.x) < 0.1f) {
                        mVelocity.x = 0.0f;
                }
        }
        position.x += mVelocity.x * deltaTime;


        mVelocity.y += mAcceleration.y * deltaTime;
        if (mVelocity.y > MAX_VELOCITY) {
                mVelocity.y = MAX_VELOCITY;
        }else if (mVelocity.y < -MAX_VELOCITY) {
                mVelocity.y = -MAX_VELOCITY;
        }else {
                if (Math::Abs(mVelocity.y) < 0.1f) {
                        mVelocity.y = 0.0f;
                }
        }
        position.y += mVelocity.y * deltaTime;


        ScreenWrap(position);
        mOwner->SetPosition(position);
        mAcceleration.x = 0.0f;
        mAcceleration.y = 0.0f;

        float rotation = mOwner->GetRotation();
        rotation += mAngularSpeed * deltaTime;
        mOwner->SetRotation(rotation);
}

void RigidBodyComponent::ScreenWrap(Vector2 &position)
{
        if (position.x > Game::WINDOW_WIDTH) {
                position.x = 0;
        }else if (position.x < 0) {
                position.x = Game::WINDOW_WIDTH;
        }

        if (position.y > Game::WINDOW_HEIGHT) {
                position.y = 0;
        }else if (position.y < 0) {
                position.y = Game::WINDOW_HEIGHT;
        }
}
