#include "header.h"
#include "PsymsPlayerObject.h"

/*
Constructor - initialize the player object at a starting position
*/
PsymsPlayerObject::PsymsPlayerObject(BaseEngine* pEngine, int iStartX, int iStartY)
	: DisplayableObject(pEngine)
	, m_bMovingUp(false)
	, m_bMovingDown(false)
	, m_bMovingLeft(false)
	, m_bMovingRight(false)
	, m_uiColor(0x00FFFF) // Cyan color
	, m_iCollisionFlashTimer(0)
	, m_pEngine(pEngine)
{
	// Set the object's position
	m_iCurrentScreenX = iStartX;
	m_iCurrentScreenY = iStartY;

	// Set the drawing area for this object
	m_iDrawWidth = OBJECT_WIDTH;
	m_iDrawHeight = OBJECT_HEIGHT;

	// Make object visible
	setVisible(true);
}

/*
Draw the object - custom appearance with a square and circle
*/
void PsymsPlayerObject::virtDraw()
{
	// Only draw if visible
	if (!isVisible())
		return;

	// Use white color if in collision flash mode
	unsigned int drawColor = m_uiColor;
	if (m_iCollisionFlashTimer > 0)
	{
		drawColor = 0xFFFFFF; // White flash on collision
	}

	// Draw outer square (filled rectangle)
	m_pEngine->drawForegroundRectangle(
		m_iCurrentScreenX,
		m_iCurrentScreenY,
		m_iCurrentScreenX + OBJECT_WIDTH,
		m_iCurrentScreenY + OBJECT_HEIGHT,
		drawColor
	);

	// Draw inner circle for contrast
	int centerX = m_iCurrentScreenX + OBJECT_WIDTH / 2;
	int centerY = m_iCurrentScreenY + OBJECT_HEIGHT / 2;
	m_pEngine->drawForegroundOval(
		centerX - 10,
		centerY - 10,
		centerX + 10,
		centerY + 10,
		0xFFFFFF // White circle
	);

	// Draw a small cross in the center to show direction
	m_pEngine->drawForegroundLine(
		centerX - 5, centerY,
		centerX + 5, centerY,
		0xFF0000 // Red
	);
	m_pEngine->drawForegroundLine(
		centerX, centerY - 5,
		centerX, centerY + 5,
		0xFF0000 // Red
	);
}

/*
Update the object - move based on current movement flags
*/
void PsymsPlayerObject::virtDoUpdate(int iCurrentTime)
{
	// Don't update if not visible
	if (!isVisible())
		return;

	// Don't update if game is paused
	if (m_pEngine->isPaused())
		return;

	// Update collision flash timer
	if (m_iCollisionFlashTimer > 0)
	{
		m_iCollisionFlashTimer--;
	}

	// Update position based on movement flags
	if (m_bMovingUp)
	{
		m_iCurrentScreenY -= MOVEMENT_SPEED;
		if (m_iCollisionFlashTimer == 0)
			m_uiColor = 0x00FF00; // Green when moving up
	}
	if (m_bMovingDown)
	{
		m_iCurrentScreenY += MOVEMENT_SPEED;
		if (m_iCollisionFlashTimer == 0)
			m_uiColor = 0xFF0000; // Red when moving down
	}
	if (m_bMovingLeft)
	{
		m_iCurrentScreenX -= MOVEMENT_SPEED;
		if (m_iCollisionFlashTimer == 0)
			m_uiColor = 0x0000FF; // Blue when moving left
	}
	if (m_bMovingRight)
	{
		m_iCurrentScreenX += MOVEMENT_SPEED;
		if (m_iCollisionFlashTimer == 0)
			m_uiColor = 0xFFFF00; // Yellow when moving right
	}

	// If not moving, use cyan
	if (!m_bMovingUp && !m_bMovingDown && !m_bMovingLeft && !m_bMovingRight)
	{
		if (m_iCollisionFlashTimer == 0)
			m_uiColor = 0x00FFFF;
	}

	// Keep object within screen bounds
	if (m_iCurrentScreenX < 0)
		m_iCurrentScreenX = 0;
	if (m_iCurrentScreenX + OBJECT_WIDTH > m_pEngine->getWindowWidth())
		m_iCurrentScreenX = m_pEngine->getWindowWidth() - OBJECT_WIDTH;
	if (m_iCurrentScreenY < 0)
		m_iCurrentScreenY = 0;
	if (m_iCurrentScreenY + OBJECT_HEIGHT > m_pEngine->getWindowHeight())
		m_iCurrentScreenY = m_pEngine->getWindowHeight() - OBJECT_HEIGHT;

	// Tell the engine to redraw this object
	redrawDisplay();
}

/*
Handle collision with another object
*/
void PsymsPlayerObject::handleCollision()
{
	// Flash white for 10 frames
	m_iCollisionFlashTimer = 10;

	// Force a redraw to show the flash
	redrawDisplay();
}

/*
Handle key press - set movement flags
*/
void PsymsPlayerObject::virtKeyDown(int iKeyCode)
{
	// Don't accept input when paused
	if (m_pEngine->isPaused())
		return;

	switch (iKeyCode)
	{
	case SDLK_w:
	case SDLK_UP:
		m_bMovingUp = true;
		break;
	case SDLK_s:
	case SDLK_DOWN:
		m_bMovingDown = true;
		break;
	case SDLK_a:
	case SDLK_LEFT:
		m_bMovingLeft = true;
		break;
	case SDLK_d:
	case SDLK_RIGHT:
		m_bMovingRight = true;
		break;
	}
}

/*
Handle key release - clear movement flags
*/
void PsymsPlayerObject::virtKeyUp(int iKeyCode)
{
	switch (iKeyCode)
	{
	case SDLK_w:
	case SDLK_UP:
		m_bMovingUp = false;
		break;
	case SDLK_s:
	case SDLK_DOWN:
		m_bMovingDown = false;
		break;
	case SDLK_a:
	case SDLK_LEFT:
		m_bMovingLeft = false;
		break;
	case SDLK_d:
	case SDLK_RIGHT:
		m_bMovingRight = false;
		break;
	}
}

/*
Handle mouse click - teleport object to clicked location
*/
void PsymsPlayerObject::virtMouseDown(int iButton, int iX, int iY)
{
	// Don't accept input when paused
	if (m_pEngine->isPaused())
		return;

	// Left mouse button teleports the object
	if (iButton == SDL_BUTTON_LEFT)
	{
		// Center the object on the click position
		m_iCurrentScreenX = iX - OBJECT_WIDTH / 2;
		m_iCurrentScreenY = iY - OBJECT_HEIGHT / 2;

		// Change color to indicate teleportation
		m_uiColor = 0xFF00FF; // Magenta

		// Redraw
		redrawDisplay();
	}
}