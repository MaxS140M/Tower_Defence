#pragma once

#include "DisplayableObject.h"
#include "ImageManager.h"

class MainEngine;
class EnemyObject;

class BulletObject : public DisplayableObject
{
public:
    BulletObject(MainEngine* pEngine, int startX, int startY, EnemyObject* target, int damage);

    void virtDraw() override;
    void virtDoUpdate(int iCurrentTime) override;

    bool shouldBeRemoved() const { return m_bShouldRemove; }

private:
    MainEngine* m_pEngine;
    EnemyObject* m_pTarget;
    int m_iDamage;
    int m_iSpeed;
    bool m_bShouldRemove;


    // World position
    float m_fWorldX;
    float m_fWorldY;

    // Arrow sprite and rotation
    SimpleImage m_arrowSprite;
    float m_fRotationAngle;  // Angle in radians

    // Helper method for pixel-perfect collision between bullet sprite and enemy sprite
    bool checkSpriteToSpriteCollision(EnemyObject* enemy);
};