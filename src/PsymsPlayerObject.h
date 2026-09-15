#ifndef PSYMSPLAYEROBJECT_H
#define PSYMSPLAYEROBJECT_H

#include "DisplayableObject.h"
#include "BaseEngine.h"

/*
Custom player-controlled object that can be moved with keyboard (WASD/Arrow keys)
and mouse (click to teleport). 
*/
class PsymsPlayerObject : public DisplayableObject
{
public:
	// Constructor
	PsymsPlayerObject(BaseEngine* pEngine, int iStartX, int iStartY);

	// Destructor
	virtual ~PsymsPlayerObject() {}

	// Draw the object - shows a custom colored square with a circle inside
	void virtDraw() override;

	// Update the object's position based on time
	void virtDoUpdate(int iCurrentTime) override;

	// Handle keyboard input for movement
	void virtKeyDown(int iKeyCode) override;
	void virtKeyUp(int iKeyCode) override;

	// Handle mouse input for teleportation
	void virtMouseDown(int iButton, int iX, int iY) override;

	// Get bounding box for collision detection
	int getLeft() const { return m_iCurrentScreenX; }
	int getRight() const { return m_iCurrentScreenX + OBJECT_WIDTH; }
	int getTop() const { return m_iCurrentScreenY; }
	int getBottom() const { return m_iCurrentScreenY + OBJECT_HEIGHT; }

	// Handle collision with another object
	void handleCollision();

private:
	// Movement speed in pixels per update
	static const int MOVEMENT_SPEED = 5;

	// Object size
	static const int OBJECT_WIDTH = 40;
	static const int OBJECT_HEIGHT = 40;

	// Movement state flags
	bool m_bMovingUp;
	bool m_bMovingDown;
	bool m_bMovingLeft;
	bool m_bMovingRight;

	// Color that changes based on movement
	unsigned int m_uiColor;

	// Collision flash effect
	int m_iCollisionFlashTimer;

	// Keep track of the engine to access drawing functions
	BaseEngine* m_pEngine;
};

#endif