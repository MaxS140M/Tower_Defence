#include "header.h"
#include "BaseEngine.h"
#include "MyObjectB.h"

MyObjectB::MyObjectB(BaseEngine* pEngine)
	: DisplayableObject(100, 200, pEngine, 100, 200, true)
{
	// Constructor using initializer list
	// Parameters: x=100, y=200, engine pointer, width=100, height=200, useTopLeft=true
	// This sets:
	// - m_iCurrentScreenX = 100 (starting X position)
	// - m_iCurrentScreenY = 200 (starting Y position)
	// - m_iDrawWidth = 100 (width of drawing area)
	// - m_iDrawHeight = 200 (height of drawing area)
	// - Drawing position is relative to top-left corner
}

void MyObjectB::virtDraw()
{
	// Draw a green rectangle filling the entire drawing area
	getEngine()->drawForegroundRectangle(
		m_iCurrentScreenX, m_iCurrentScreenY,
		m_iCurrentScreenX + m_iDrawWidth - 1,
		m_iCurrentScreenY + m_iDrawHeight - 1,
		0x00ff00); // Green color
}

void MyObjectB::virtDoUpdate(int iCurrentTime)
{
	// Change position if player presses a key
	if (getEngine()->isKeyPressed(SDLK_UP))
		m_iCurrentScreenY -= 2;
	if (getEngine()->isKeyPressed(SDLK_DOWN))
		m_iCurrentScreenY += 2;
	if (getEngine()->isKeyPressed(SDLK_LEFT))
		m_iCurrentScreenX -= 2;
	if (getEngine()->isKeyPressed(SDLK_RIGHT))
		m_iCurrentScreenX += 2;

	// Prevent object from moving off the edge of the screen
	if (m_iCurrentScreenX < 0)
		m_iCurrentScreenX = 0;
	if (m_iCurrentScreenX >= getEngine()->getWindowWidth() - m_iDrawWidth)
		m_iCurrentScreenX = getEngine()->getWindowWidth() - m_iDrawWidth;

	if (m_iCurrentScreenY < 0)
		m_iCurrentScreenY = 0;
	if (m_iCurrentScreenY >= getEngine()->getWindowHeight() - m_iDrawHeight)
		m_iCurrentScreenY = getEngine()->getWindowHeight() - m_iDrawHeight;

	// Ensure that the objects get redrawn on the display
	redrawDisplay();
}