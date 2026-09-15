#include "header.h"
#include "PlayerObject.h"
#include "MainEngine.h"
#include "MainTileManager.h"


//Player Object constrcuctor subclass
PlayerObject::PlayerObject(MainEngine* pEngine)
	: GameObject(pEngine)
	, m_iVelocityX(0)
	, m_iVelocityY(0)
	, m_iSpeed(6)
	, m_iBuildRadius(120)
	, m_iCurrentFrame(0)
	, m_iLastAnimTime(0)
	, m_iAnimDelay(100)
	, m_iAnimRow(0)
	, m_bFacingLeft(false)
	, m_bIsMoving(false)
{
	// Set initial world position using inherited method
	m_iWorldX = 625;
	m_iWorldY = 875;

	// Load the spritesheet
	m_spriteSheet = ImageManager::loadImage("sprites/character.png", true);

	// Set drawing size
	m_iWidth = 96;
	m_iHeight = 96;
	setSize(96, 96);
	setVisible(true);
}
//draw player
void PlayerObject::virtDraw()
{
	if (!isVisible())
		return;

	DrawingSurface* pSurface = getEngine()->getForegroundSurface();

	int centerX = getXCentre();
	int centerY = getYCentre();

	const int SPRITE_SIZE = 48;
	const int SCALE = 2;
	const int DRAW_SIZE = SPRITE_SIZE * SCALE;

	int drawX = centerX - DRAW_SIZE / 2;
	int drawY = centerY - DRAW_SIZE / 2;

	int srcX = m_iCurrentFrame * SPRITE_SIZE;
	int srcY = m_iAnimRow * SPRITE_SIZE;

	if (m_spriteSheet.exists())
	{
		for (int py = 0; py < DRAW_SIZE; py++)
		{
			for (int px = 0; px < DRAW_SIZE; px++)
			{
				int sourceX = m_bFacingLeft ? (SPRITE_SIZE - 1 - (int)(px / SCALE)) : (int)(px / SCALE);
				int sourceY = (int)(py / SCALE);

				if (sourceX >= SPRITE_SIZE) sourceX = SPRITE_SIZE - 1;
				if (sourceY >= SPRITE_SIZE) sourceY = SPRITE_SIZE - 1;

				int pixelColor = m_spriteSheet.getPixelColour(srcX + sourceX, srcY + sourceY);

				if (pixelColor == 0x000000 || pixelColor == 0x3F3F3F || pixelColor == 0x404040)
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

	// Draw build radius circle
	int buildRadiusPixels = m_iBuildRadius;
	
	for (int thickness = -1; thickness <= 1; thickness++)
	{
		int currentRadius = buildRadiusPixels + thickness;
		for (int angle = 0; angle < 360; angle += 2)
		{
			bool shouldDraw = (angle / 10) % 2 == 0;
			
			int x = centerX + (int)(currentRadius * cos(angle * 3.14159 / 180.0));
			int y = centerY + (int)(currentRadius * sin(angle * 3.14159 / 180.0));

			if (x >= 0 && x < getEngine()->getWindowWidth() &&
				y >= 0 && y < getEngine()->getWindowHeight())
			{
				if (shouldDraw)
				{
					pSurface->setPixel(x, y, 0x00FFFF);
				}
				else
				{
					// Semi-transparent effect
					unsigned int bgPixel = pSurface->getPixel(x, y);
					int bgR = (bgPixel >> 16) & 0xFF;
					int bgG = (bgPixel >> 8) & 0xFF;
					int bgB = bgPixel & 0xFF;
					int finalR = (bgR + 0) / 2;
					int finalG = (bgG + 255) / 2;
					int finalB = (bgB + 255) / 2;
					
					pSurface->setPixel(x, y, (finalR << 16) | (finalG << 8) | finalB);
				}
			}
		}
	}
}

void PlayerObject::virtDoUpdate(int iCurrentTime)
{
	// Update world position based on velocity
	m_iWorldX += m_iVelocityX;
	m_iWorldY += m_iVelocityY;

	// Clamp to world bounds
	if (m_pMainEngine != nullptr && m_pMainEngine->getTileManager() != nullptr)
	{
		MainTileManager* tm = m_pMainEngine->getTileManager();
		int worldWidth = tm->getMapWidth() * tm->getTileWidth();
		int worldHeight = tm->getMapHeight() * tm->getTileHeight();

		if (m_iWorldX < 0) m_iWorldX = 0;
		if (m_iWorldY < 0) m_iWorldY = 0;
		if (m_iWorldX + getWidth() > worldWidth) m_iWorldX = worldWidth - getWidth();
		if (m_iWorldY + getHeight() > worldHeight) m_iWorldY = worldHeight - getHeight();
	}

	if (m_iVelocityX != 0 || m_iVelocityY != 0)
	{
		redrawDisplay();
	}

	if (m_iVelocityX < 0)
		m_bFacingLeft = true;
	else if (m_iVelocityX > 0)
		m_bFacingLeft = false;

	m_bIsMoving = (m_iVelocityX != 0 || m_iVelocityY != 0);

	if (m_bIsMoving)
		m_iAnimRow = 3;
	else
		m_iAnimRow = 0;

	if (iCurrentTime - m_iLastAnimTime > m_iAnimDelay)
	{
		m_iCurrentFrame = (m_iCurrentFrame + 1) % 8;
		m_iLastAnimTime = iCurrentTime;
	}
}

void PlayerObject::setVelocity(int vx, int vy)
{
	m_iVelocityX = vx;
	m_iVelocityY = vy;
}