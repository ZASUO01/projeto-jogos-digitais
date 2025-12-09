#pragma once
#include "Actor.h"

class LaserBeam : public Actor
{
public:
    LaserBeam(class Game* game, const Vector2& startPos, float rotation, Vector3 color, class Ship* ownerShip);
    ~LaserBeam();
    
    void OnUpdate(float deltaTime) override;
    
    class LaserBeamComponent* GetLaserComponent() const { return mLaserComponent; }
    class Ship* GetOwnerShip() const { return mOwnerShip; }

private:
    class LaserBeamComponent* mLaserComponent;
    class Ship* mOwnerShip;
    class AudioPlayer* mShootSound;
};

