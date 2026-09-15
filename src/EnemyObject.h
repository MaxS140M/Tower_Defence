#ifndef ENEMY_OBJECT_H
#define ENEMY_OBJECT_H

#include "GameObject.h"
#include "ImageManager.h"
#include <vector>
#include <utility>

class MainEngine;

class EnemyObject : public GameObject
{
public:
    EnemyObject(MainEngine* pEngine, int wave);

    void virtDraw() override;
    void virtDoUpdate(int iCurrentTime) override;

    void takeDamage(int damage);
    bool isDead() const { return m_iHealth <= 0; }
    bool reachedBase() const { return m_bReachedBase; }
    bool isDeathAnimationComplete() const { return m_bDeathAnimComplete; }

    int getMoneyReward() const { return m_iMoneyReward; }

    // Pixel-perfect collision detection
    bool checkPixelCollision(int worldX, int worldY) const;

private:
    void updatePathFollowing();
    void buildCompletePath();
    int evaluateRoute(const std::vector<std::pair<int, int>>& route);
    int countTowersNearPath(const std::vector<std::pair<int, int>>& pathSegment, int range);

    float m_fWorldX;
    float m_fWorldY;
    int m_iHealth;
    int m_iMaxHealth;
    int m_iSpeed;
    int m_iMoneyReward;
    bool m_bReachedBase;

    // Pathfinding
    std::vector<std::pair<int, int>> m_completePath;
    int m_iCurrentPathIndex;
    int m_iTargetX;
    int m_iTargetY;
    int m_iChosenRouteIndex;

    // Sprite animation
    SimpleImage m_walkSprite;
    SimpleImage m_deathSprite;
    int m_iCurrentFrame;
    int m_iLastAnimTime;
    int m_iAnimDelay;
    bool m_bDeathAnimComplete;
};

#endif