#pragma once

#include "DisplayableObject.h"
#include "ImageManager.h"

class MainEngine;

// Intermediate class between DisplayableObject and game-specific objects
// world position, sprite handling, and safe self-destruction
class GameObject : public DisplayableObject
{
public:
    GameObject(MainEngine* pEngine);
    virtual ~GameObject() {}

    // World position
    int getWorldX() const { return m_iWorldX; }
    int getWorldY() const { return m_iWorldY; }
    void setWorldPosition(int x, int y);

    // Size
    virtual int getWidth() const { return m_iWidth; }
    virtual int getHeight() const { return m_iHeight; }

    // Safe self-destruction
    void markForDeletion() { m_bMarkedForDeletion = true; }
    bool isMarkedForDeletion() const { return m_bMarkedForDeletion; }

    // Sprite rendering helper 
    void renderSprite(SimpleImage& sprite, int srcX, int srcY, 
                      int spriteSize, float scale, bool flipHorizontal = false);

protected:
    MainEngine* m_pMainEngine;
    int m_iWorldX;
    int m_iWorldY;
    int m_iWidth;
    int m_iHeight;
    bool m_bMarkedForDeletion;
};