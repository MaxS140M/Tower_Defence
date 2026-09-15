#include "header.h"
#include "BulletObject.h"
#include "MainEngine.h"
#include "EnemyObject.h"
#include "ImagePixelMapping.h"
#include <cmath>


//Bullet Constructor
BulletObject::BulletObject(MainEngine* pEngine, int startX, int startY, EnemyObject* target, int damage)
    : DisplayableObject(pEngine)
    , m_pEngine(pEngine)
    , m_pTarget(target)
    , m_iDamage(damage)
    , m_iSpeed(8)
    , m_bShouldRemove(false)
    , m_fWorldX((float)startX)
    , m_fWorldY((float)startY)
    , m_fRotationAngle(0.0f)
//Bullet Image
{
    m_arrowSprite = ImageManager::loadImage("resources/arrow.png", true);
    
    if (m_arrowSprite.exists())
    {
        m_arrowSprite.setTransparencyColour(0x000000);
    }
    
    setSize(12, 4);
    setVisible(true);
    setPosition(startX, startY);
}
//Draw bullet Method
void BulletObject::virtDraw()
{
    if (!isVisible())
        return;

    if (m_arrowSprite.exists())
    {
        const int DRAW_WIDTH = 32;
        const int DRAW_HEIGHT = 18;
        
        DrawingSurface* pSurface = getEngine()->getForegroundSurface();
        
        int centerX = getXCentre();
        int centerY = getYCentre();
        
        float cosAngle = cos(m_fRotationAngle);
        float sinAngle = sin(m_fRotationAngle);
        
        int imgWidth = m_arrowSprite.getWidth();
        int imgHeight = m_arrowSprite.getHeight();
        
        //loop through each pixel drawing to screen
        for (int py = 0; py < DRAW_HEIGHT; py++)
        {
            for (int px = 0; px < DRAW_WIDTH; px++)
            {
                int srcX = (px * imgWidth) / DRAW_WIDTH;
                int srcY = (py * imgHeight) / DRAW_HEIGHT;
                
                if (srcX >= imgWidth) srcX = imgWidth - 1;
                if (srcY >= imgHeight) srcY = imgHeight - 1;
                
                int pixelColor = m_arrowSprite.getPixelColour(srcX, srcY);
                
                if (pixelColor == 0x000000)
                    continue;
                //rotate around center
                float relX = px - (DRAW_WIDTH / 2.0f);
                float relY = py - (DRAW_HEIGHT / 2.0f);
                //apply rotation matrix
                int rotatedX = (int)(relX * cosAngle - relY * sinAngle);
                int rotatedY = (int)(relX * sinAngle + relY * cosAngle);
                
                int screenX = centerX + rotatedX;
                int screenY = centerY + rotatedY;
                
                if (screenX >= 0 && screenX < getEngine()->getWindowWidth() &&
                    screenY >= 0 && screenY < getEngine()->getWindowHeight())
                {
                    pSurface->setPixel(screenX, screenY, pixelColor);
                }
            }
        }
    }
    //fallback to draw yellow circle if sprite didnt load
    else
    {
        DrawingSurface* pSurface = getEngine()->getForegroundSurface();
        int centerX = getXCentre();
        int centerY = getYCentre();
        int radius = 2;

        for (int y = -radius; y <= radius; y++)
        {
            int width = (int)sqrt(radius * radius - y * y);
            for (int x = -width; x <= width; x++)
            {
                int px = centerX + x;
                int py = centerY + y;

                if (px >= 0 && px < getEngine()->getWindowWidth() &&
                    py >= 0 && py < getEngine()->getWindowHeight())
                {
                    pSurface->setPixel(px, py, 0xFFFF00);
                }
            }
        }
    }
}

void BulletObject::virtDoUpdate(int iCurrentTime)
{
    if (m_pTarget == nullptr || m_pTarget->isDead())
    {
        m_bShouldRemove = true;
        return;
    }

    int targetCenterX = m_pTarget->getWorldX() + m_pTarget->getWidth() / 2;
    int targetCenterY = m_pTarget->getWorldY() + m_pTarget->getHeight() / 2;

    float dx = targetCenterX - m_fWorldX;
    float dy = targetCenterY - m_fWorldY;
    float distance = sqrt(dx * dx + dy * dy);

    // checks every pixel in the overlap region between bullet and enemy sprites
    if (checkSpriteToSpriteCollision(m_pTarget))
    {
        // Hit detected at pixel level!
        m_pTarget->takeDamage(m_iDamage);
        m_bShouldRemove = true;
        return;
    }

    // Move bullet towards target
    if (distance > 0)
    {
        m_fRotationAngle = atan2(dy, dx);
        float vx = (dx / distance) * m_iSpeed;
        float vy = (dy / distance) * m_iSpeed;
        m_fWorldX += vx;
        m_fWorldY += vy;
    }

    setPosition((int)m_fWorldX, (int)m_fWorldY);
    redrawDisplay();
}

// Advanced pixel-perfect collision: bullet sprite vs enemy sprite
bool BulletObject::checkSpriteToSpriteCollision(EnemyObject* enemy)
{
    if (!m_arrowSprite.exists())
    {
        // Fallback to point-based collision if no bullet sprite
        int bulletCenterX = (int)m_fWorldX;
        int bulletCenterY = (int)m_fWorldY;
        return enemy->checkPixelCollision(bulletCenterX, bulletCenterY);
    }

    const int BULLET_DRAW_WIDTH = 32;
    const int BULLET_DRAW_HEIGHT = 18;

    int bulletCenterX = getXCentre();
    int bulletCenterY = getYCentre();

    // Get bullet's bounding box in world coordinates
    int bulletLeft = bulletCenterX - BULLET_DRAW_WIDTH / 2;
    int bulletTop = bulletCenterY - BULLET_DRAW_HEIGHT / 2;
    int bulletRight = bulletLeft + BULLET_DRAW_WIDTH;
    int bulletBottom = bulletTop + BULLET_DRAW_HEIGHT;

    // Get enemy's bounding box
    int enemyLeft = enemy->getWorldX();
    int enemyTop = enemy->getWorldY();
    int enemyRight = enemyLeft + enemy->getWidth();
    int enemyBottom = enemyTop + enemy->getHeight();

    // Check if bounding boxes overlap
    if (bulletRight < enemyLeft || bulletLeft > enemyRight ||
        bulletBottom < enemyTop || bulletTop > enemyBottom)
    {
        return false;
    }

    // Calculate overlap region
    int overlapLeft = (bulletLeft > enemyLeft) ? bulletLeft : enemyLeft;
    int overlapTop = (bulletTop > enemyTop) ? bulletTop : enemyTop;
    int overlapRight = (bulletRight < enemyRight) ? bulletRight : enemyRight;
    int overlapBottom = (bulletBottom < enemyBottom) ? bulletBottom : enemyBottom;

    // Check every pixel in the overlap region
    for (int worldY = overlapTop; worldY < overlapBottom; worldY++)
    {
        for (int worldX = overlapLeft; worldX < overlapRight; worldX++)
        {
            // Check if this pixel is non-transparent in the bullet
            bool bulletHasPixel = false;

            // Convert world coordinate to bullet-local coordinate
            int bulletLocalX = worldX - bulletLeft;
            int bulletLocalY = worldY - bulletTop;

            if (bulletLocalX >= 0 && bulletLocalX < BULLET_DRAW_WIDTH &&
                bulletLocalY >= 0 && bulletLocalY < BULLET_DRAW_HEIGHT)
            {
                // Apply rotation transformation
                float cosAngle = cos(m_fRotationAngle);
                float sinAngle = sin(m_fRotationAngle);

                // Convert to centered coordinates
                float relX = bulletLocalX - (BULLET_DRAW_WIDTH / 2.0f);
                float relY = bulletLocalY - (BULLET_DRAW_HEIGHT / 2.0f);

                // Apply inverse rotation
                int unrotatedX = (int)(relX * cosAngle + relY * sinAngle);
                int unrotatedY = (int)(-relX * sinAngle + relY * cosAngle);

                // Convert back to sprite space
                unrotatedX += BULLET_DRAW_WIDTH / 2;
                unrotatedY += BULLET_DRAW_HEIGHT / 2;

                // Map to source sprite coordinates
                int imgWidth = m_arrowSprite.getWidth();
                int imgHeight = m_arrowSprite.getHeight();

                if (unrotatedX >= 0 && unrotatedX < BULLET_DRAW_WIDTH &&
                    unrotatedY >= 0 && unrotatedY < BULLET_DRAW_HEIGHT)
                {
                    int srcX = (unrotatedX * imgWidth) / BULLET_DRAW_WIDTH;
                    int srcY = (unrotatedY * imgHeight) / BULLET_DRAW_HEIGHT;

                    if (srcX >= 0 && srcX < imgWidth && srcY >= 0 && srcY < imgHeight)
                    {
                        int pixelColor = m_arrowSprite.getPixelColour(srcX, srcY);
                        if (pixelColor != 0x000000) // Non-transparent
                        {
                            bulletHasPixel = true;
                        }
                    }
                }
            }

            // If bullet has a pixel here, check if enemy also has a pixel
            if (bulletHasPixel)
            {
                if (enemy->checkPixelCollision(worldX, worldY))
                {
                    return true;
                }
            }
        }
    }

    return false;
}