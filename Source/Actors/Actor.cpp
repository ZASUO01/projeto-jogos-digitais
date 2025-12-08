//
// Created by pedro-souza on 23/11/2025.
//

// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
//
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include "Actor.h"
#include "../Game.h"
#include "../Components/Component.h"
#include <algorithm>

Actor::Actor(Game* game)
        : mState(ActorState::Active)
        , mPosition(Vector2::Zero)
        , mScale(Vector2(1.0f, 1.0f))
        , mRotation(0.0f)
        , mGame(game)
{
    mGame->AddActor(this);
}

Actor::~Actor()
{
    mGame->RemoveActor(this);
    const unsigned int size = mComponents.size();
    for (unsigned int i = 0; i < size; i++) {
        delete mComponents[i];
    }
    mComponents.clear();
}

void Actor::Update(const float deltaTime)
{
    if (mState == ActorState::Active)
    {
        OnUpdate(deltaTime);
        unsigned int size = mComponents.size();
        for (unsigned int i = 0; i < size; i++) {
            mComponents[i]->Update(deltaTime);
        }
    }
}

void Actor::OnUpdate(float deltaTime){}

void Actor::ProcessInput(const Uint8* keyState)
{
   if (mState == ActorState::Active)
    {
        OnProcessInput(keyState);
        unsigned int size = mComponents.size();
       for (unsigned int i = 0; i < size; i++) {
           mComponents[i]->ProcessInput(keyState);
       }
    }
}

void Actor::OnProcessInput(const Uint8* keyState){}

void Actor::AddComponent(Component* c)
{
    mComponents.emplace_back(c);
    std::sort(
        mComponents.begin(),
        mComponents.end(),
        [](const Component* a, const Component* b) {
            return a->GetUpdateOrder() < b->GetUpdateOrder();
        });
}

Matrix4 Actor::GetModelMatrix() const
{
    Matrix4 scaleMat = Matrix4::CreateScale(mScale.x, mScale.y, 1.0f);
    Matrix4 rotMat   = Matrix4::CreateRotationZ(mRotation);
    Matrix4 transMat = Matrix4::CreateTranslation(Vector3(mPosition.x, mPosition.y, 0.0f));
    return scaleMat * rotMat * transMat;
}
