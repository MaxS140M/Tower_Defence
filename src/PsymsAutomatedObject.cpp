#include "header.h"
#include "PsymsAutomatedObject.h"
#include "PsymsEngine.h"
#include "PsymsTileManager.h"
#include <cmath>

/*
Constructor - Initialize automated object with movement parameters
*/
PsymsAutomatedObject::PsymsAutomatedObject(BaseEngine* pEngine, int iStartX, int iStartY, double fSpeedX, double fAmplitude, double fFrequency)
	: DisplayableObject(pEngine)
	, m_pEngine(pEngine)
	, m_fX((double)iStartX)
	, m_fY((double)iStartY)
	, m_fSpeedX(fSpeedX)
	, m_fAmplitude(fAmplitude)
	, m_fFrequency(fFrequency)
	, m_fPhase(0.0)
	, m_fBaseY((double)iStartY)
	, m_iColorPhase(0)
	, m_iLastUpdateTime(0)
	, m_iCollisionFlashTimer(0)
	, m_iLastTileX(-1)
	, m_iLastTileY(-1)
{
	// Set initial screen position
	m_iCurrentScreenX = iStartX;
	m_iCurrentScreenY = iStartY;

	// Set object size
	setSize(OBJECT_RADIUS * 2, OBJECT_RADIUS * 2);

	// Make visible by default
	setVisible(true);
}

/*
Draw the object - pulsing circle with gradient effect
*/
void PsymsAutomatedObject::virtDraw()
{
	if (!isVisible())
		return;

	// Get drawing surface
	DrawingSurface* surface = m_pEngine->getForegroundSurface();

	// Calculate center of circle
	int centerX = m_iCurrentScreenX + OBJECT_RADIUS;
	int centerY = m_iCurrentScreenY + OBJECT_RADIUS;

	// Determine color
	unsigned int color;
	if (m_iCollisionFlashTimer > 0)
	{
		color = 0xFFFFFF; // White flash
	}
	else
	{
		// Pulsing color effect
		int r = (int)(128 + 127 * sin(m_iColorPhase * 0.05));
		int g = (int)(128 + 127 * sin(m_iColorPhase * 0.05 + 2.0));
		int b = (int)(128 + 127 * sin(m_iColorPhase * 0.05 + 4.0));
		color = (r << 16) | (g << 8) | b;
	}

	// Draw filled circle
	for (int dy = -OBJECT_RADIUS; dy <= OBJECT_RADIUS; dy++)
	{
		for (int dx = -OBJECT_RADIUS; dx <= OBJECT_RADIUS; dx++)
		{
			if (dx * dx + dy * dy <= OBJECT_RADIUS * OBJECT_RADIUS)
			{
				surface->setPixel(centerX + dx, centerY + dy, color);
			}
		}
	}

	// Draw darker outline
	unsigned int outlineColor = 0x000000;
	for (int angle = 0; angle < 360; angle += 5)
	{
		int x = centerX + (int)(OBJECT_RADIUS * cos(angle * 3.14159 / 180.0));
		int y = centerY + (int)(OBJECT_RADIUS * sin(angle * 3.14159 / 180.0));
		surface->setPixel(x, y, outlineColor);
	}
}

/*
Update position - autonomous sine wave movement with wall bouncing
*/
void PsymsAutomatedObject::virtDoUpdate(int iCurrentTime)
{
	// Skip update if not visible
	if (!isVisible())
		return;

	//  Skip update if game is paused
	if (m_pEngine->isPaused())
		return;

	// Initialize last update time on first call
	if (m_iLastUpdateTime == 0)
		m_iLastUpdateTime = iCurrentTime;

	// Calculate time delta (for frame-rate independent movement)
	int deltaTime = iCurrentTime - m_iLastUpdateTime;
	m_iLastUpdateTime = iCurrentTime;

	// Update horizontal position
	m_fX += m_fSpeedX;

	// Update phase for sine wave
	m_fPhase += m_fFrequency;

	// Calculate Y position using sine wave
	m_fY = m_fBaseY + m_fAmplitude * sin(m_fPhase);

	// Bounce off left and right walls
	if (m_fX <= 0)
	{
		m_fX = 0;
		m_fSpeedX = -m_fSpeedX; // Reverse direction
	}
	else if (m_fX >= m_pEngine->getWindowWidth() - (OBJECT_RADIUS * 2))
	{
		m_fX = m_pEngine->getWindowWidth() - (OBJECT_RADIUS * 2);
		m_fSpeedX = -m_fSpeedX; // Reverse direction
	}

	// Update screen position
	m_iCurrentScreenX = (int)m_fX;
	m_iCurrentScreenY = (int)m_fY;

	// Keep Y position on screen 
	if (m_iCurrentScreenY < 0)
	{
		m_fBaseY += 10;
		m_iCurrentScreenY = 0;
	}
	else if (m_iCurrentScreenY > m_pEngine->getWindowHeight() - (OBJECT_RADIUS * 2))
	{
		m_fBaseY -= 10;
		m_iCurrentScreenY = m_pEngine->getWindowHeight() - (OBJECT_RADIUS * 2);
	}

	// Update color phase for pulsing effect
	m_iColorPhase++;

	// Update collision flash timer
	if (m_iCollisionFlashTimer > 0)
		m_iCollisionFlashTimer--;

	// Redraw to show movement
	redrawDisplay();
}

/*
Check if this object should interact with tiles and change them
Call this only for the top automated object 
*/
void PsymsAutomatedObject::checkTileInteraction(int iCurrentTime)
{
	// TILE INTERACTION: Check if this object is over a tile and change it to black
	PsymsEngine* psymsEngine = dynamic_cast<PsymsEngine*>(m_pEngine);
	if (psymsEngine != nullptr)
	{
		PsymsTileManager* tileManager = psymsEngine->getTileManager();
		if (tileManager != nullptr)
		{
			// Check the center of the circle
			int centerX = m_iCurrentScreenX + OBJECT_RADIUS;
			int centerY = m_iCurrentScreenY + OBJECT_RADIUS;

			int tileX, tileY;
			if (tileManager->isPositionOverTile(centerX, centerY, tileX, tileY))
			{
				// Only change the tile if we've moved to a different tile
				// This prevents excessive redrawing and ensures clean tile changes
				if (tileX != m_iLastTileX || tileY != m_iLastTileY)
				{
					// Change the tile to black temporarily
					tileManager->setTileToBlackTemporarily(m_pEngine, tileX, tileY, iCurrentTime);

					// Remember which tile we just changed
					m_iLastTileX = tileX;
					m_iLastTileY = tileY;
				}
			}
			else
			{
				// Not over any tile, reset tracking
				m_iLastTileX = -1;
				m_iLastTileY = -1;
			}
		}
	}
}

/*
Handle collision with player - REVERSE DIRECTION
*/
void PsymsAutomatedObject::handleCollision()
{
	// Flash white for 10 frames
	m_iCollisionFlashTimer = 10;

	// REVERSE HORIZONTAL DIRECTION when colliding with player
	m_fSpeedX = -m_fSpeedX;
}