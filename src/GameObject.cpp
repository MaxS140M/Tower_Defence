#include "header.h"
#include "GameObject.h"
#include "MainEngine.h"


//Game Object main class constructor
GameObject::GameObject(MainEngine* pEngine)
    : DisplayableObject(pEngine)
    , m_pMainEngine(pEngine)
    , m_iWorldX(0)
    , m_iWorldY(0)
    , m_iWidth(32)
    , m_iHeight(32)
    , m_bMarkedForDeletion(false)
{
}

//set oobject position
void GameObject::setWorldPosition(int x, int y)
{
    m_iWorldX = x;
    m_iWorldY = y;
}

//draw object on screen
void GameObject::renderSprite(SimpleImage& sprite, int srcX, int srcY,
    int spriteSize, float scale, bool flipHorizontal)
{
    if (!sprite.exists())
        return;

    DrawingSurface* pSurface = getEngine()->getForegroundSurface();
    int drawSize = (int)(spriteSize * scale);
    int drawX = getXCentre() - drawSize / 2;
    int drawY = getYCentre() - drawSize / 2;

    for (int py = 0; py < drawSize; py++)
    {
        for (int px = 0; px < drawSize; px++)
        {
            int srcPX = (int)(px / scale);
            int srcPY = (int)(py / scale);

            if (srcPX >= spriteSize) srcPX = spriteSize - 1;
            if (srcPY >= spriteSize) srcPY = spriteSize - 1;

            if (flipHorizontal)
                srcPX = spriteSize - 1 - srcPX;

            int pixelColor = sprite.getPixelColour(srcX + srcPX, srcY + srcPY);

            if (pixelColor == 0x000000)
                continue;

            int targetX = drawX + px;
            int targetY = drawY + py;

            if (targetX >= 0 && targetX < getEngine()->getWindowWidth() &&
                targetY >= 0 && targetY < getEngine()->getWindowHeight())
            {
                pSurface->setPixel(targetX, targetY, pixelColor);
            }
        }
    }
}