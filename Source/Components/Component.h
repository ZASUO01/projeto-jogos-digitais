#pragma once
#include <SDL_stdinc.h>

class Component
{
public:
    Component(class Actor* owner, int updateOrder = 100);
    virtual ~Component();
    virtual void Update(float deltaTime);
    virtual void ProcessInput(const Uint8* keyState);
    virtual void DebugDraw(class Renderer* renderer);

    int GetUpdateOrder() const { return mUpdateOrder; }
    class Actor* GetOwner() const { return mOwner; }
    class Game* GetGame() const;

protected:
    class Actor* mOwner;
    int mUpdateOrder;
};
