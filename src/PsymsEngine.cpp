#include "header.h"
#include "PsymsEngine.h"
#include "PsymsPlayerObject.h" 
#include "PsymsAutomatedObject.h"
#include "PsymsTileManager.h" 


/*
Destructor - clean up tile manager
*/
PsymsEngine::~PsymsEngine()
{
	// Delete the tile manager if it was created
	if (m_pTileManager != nullptr)
	{
		delete m_pTileManager;
		m_pTileManager = nullptr;
	}
}

/*
Setup the background buffer with a unique gradient and geometric pattern design.
*/
void PsymsEngine::virtSetupBackgroundBuffer()
{
	// Seed random number generator
	static bool seeded = false;
	if (!seeded)
	{
		srand((unsigned int)time(NULL));
		seeded = true;
	}

	// Generate random colors for each border side
	unsigned int topBorderColor = getColour(rand() % 41);
	unsigned int bottomBorderColor = getColour(rand() % 41);
	unsigned int leftBorderColor = getColour(rand() % 41);
	unsigned int rightBorderColor = getColour(rand() % 41);

	// Border parameters
	int borderThickness = 20;
	int padding = 10;
	int totalBorderWidth = borderThickness + padding;

	// Create a diagonal gradient background from blue (top-left) to red (bottom-right)
	for (int iY = 0; iY < getWindowHeight(); iY++)
	{
		for (int iX = 0; iX < getWindowWidth(); iX++)
		{
			// Calculate gradient based on diagonal position
			float gradientFactor = (float)(iX + iY) / (float)(getWindowWidth() + getWindowHeight());

			//  blue (0x0000FF) to red (0xFF0000)
			int redComponent = (int)(0 + gradientFactor * 255);
			int greenComponent = 0;
			int blueComponent = (int)(255 - gradientFactor * 255);

			unsigned int gradientColor = (redComponent << 16) | (greenComponent << 8) | blueComponent;
			setBackgroundPixel(iX, iY, gradientColor);
		}
	}

	// Add a central circle
	int centerX = getWindowWidth() / 2;
	int centerY = getWindowHeight() / 2;
	for (int radius = 40; radius < 60; radius++)
	{
		for (int angle = 0; angle < 360; angle += 1)
		{
			int iX = centerX + (int)(radius * cos(angle * 3.14159 / 180.0));
			int iY = centerY + (int)(radius * sin(angle * 3.14159 / 180.0));
			if (iX >= 0 && iX < getWindowWidth() && iY >= 0 && iY < getWindowHeight())
			{
				setBackgroundPixel(iX, iY, 0xFFFFFF);
			}
		}
	}

	// Create and draw tile manager on the background
	if (m_pTileManager == nullptr)
	{
		m_pTileManager = new PsymsTileManager(25, 25, 8, 12);
	}

	// Position the tile manager lower and to the right
	m_pTileManager->setTopLeftPositionOnScreen(500, 120);
	m_pTileManager->drawAllTiles(this, getBackgroundSurface());

	// Draw border 
	// Top  
	for (int iY = padding; iY < totalBorderWidth; iY++)
	{
		for (int iX = 0; iX < getWindowWidth(); iX++)
		{
			setBackgroundPixel(iX, iY, topBorderColor);
		}
	}
	// Bottom 
	for (int iY = getWindowHeight() - totalBorderWidth; iY < getWindowHeight() - padding; iY++)
	{
		for (int iX = 0; iX < getWindowWidth(); iX++)
		{
			setBackgroundPixel(iX, iY, bottomBorderColor);
		}
	}
	// Left 
	for (int iX = padding; iX < totalBorderWidth; iX++)
	{
		for (int iY = 0; iY < getWindowHeight(); iY++)
		{
			setBackgroundPixel(iX, iY, leftBorderColor);
		}
	}
	// Right
	for (int iX = getWindowWidth() - totalBorderWidth; iX < getWindowWidth() - padding; iX++)
	{
		for (int iY = 0; iY < getWindowHeight(); iY++)
		{
			setBackgroundPixel(iX, iY, rightBorderColor);
		}
	}

	// Draw rectangles in corners using drawBackgroundRectangle()
	drawBackgroundRectangle(50, 50, 150, 120, 0xFFD700); // top-left
	drawBackgroundRectangle(getWindowWidth() - 150, 50, getWindowWidth() - 50, 120, 0x00CED1); //  top-right
	drawBackgroundRectangle(50, getWindowHeight() - 120, 150, getWindowHeight() - 50, 0xFF69B4); // bottom-left
	drawBackgroundRectangle(getWindowWidth() - 150, getWindowHeight() - 120, getWindowWidth() - 50, getWindowHeight() - 50, 0x32CD32); // bottom-right

	// Draw ovals/ellipses using drawBackgroundOval()
	drawBackgroundOval(200, 150, 300, 220, 0xFFFF00); // Yellow 
	drawBackgroundOval(getWindowWidth() - 300, 150, getWindowWidth() - 200, 220, 0xFF00FF); // Magenta 

	// Draw triangles using drawBackgroundTriangle()
	drawBackgroundTriangle(250, getWindowHeight() - 200, 200, getWindowHeight() - 100, 300, getWindowHeight() - 100, 0x00FFFF); // Cyan triangle 
	drawBackgroundTriangle(getWindowWidth() - 250, getWindowHeight() - 200, getWindowWidth() - 300, getWindowHeight() - 100, getWindowWidth() - 200, getWindowHeight() - 100, 0xFFA500); // Orange triangle

	// Draw text label for tile manager
	drawBackgroundString(500, 80, "Tile Manager:", 0xFFFFFF, NULL); 

	// Draw text to the BACKGROUND - this text will stay in place and objects will move over it
	drawBackgroundString(centerX - 150, centerY - 10, "Text Below Object", 0xFFFFFF, NULL);

	// Text in corners
	drawBackgroundString(60, 60, "TL", 0x000000, NULL);
	drawBackgroundString(getWindowWidth() - 140, 60, "TR", 0x000000, NULL);
	drawBackgroundString(60, getWindowHeight() - 100, "BL", 0x000000, NULL);
	drawBackgroundString(getWindowWidth() - 140, getWindowHeight() - 100, "BR", 0x000000, NULL);
}

/*
Initialize moving objects - basic implementation
*/
int PsymsEngine::virtInitialiseObjects()
{
	drawableObjectsChanged();
	destroyOldObjects(true);

	// Create array with 3 objects - 1 player + 2 automated
	createObjectArray(3);

	// Create the player object in the center of the screen
	storeObjectInArray(0, new PsymsPlayerObject(
		this,
		getWindowWidth() / 2 - 20,
		getWindowHeight() / 2 - 20  
	));

	// Create first automated object - moves right with sine wave
	storeObjectInArray(1, new PsymsAutomatedObject(
		this,
		100,                        // Start X
		200,                        // Start Y 
		2.5,                        // Speed X
		50.0,                       // Amplitude
		0.1                         // frequency
	));

	// Create second automated object - moves left with different parameters
	storeObjectInArray(2, new PsymsAutomatedObject(
		this,
		getWindowWidth() - 150,     // Start X 
		400,                        // Start Y 
		-1.8,                       // Speed X moving left
		70.0,                       // Amplitude 
		0.15                        // Frequency 
	));

	// Enable notification of objects about keyboard and mouse events
	notifyObjectsAboutKeys(true);
	notifyObjectsAboutMouse(true);

	return 3; // Return number of objects created
}

/*
Called before objects are updated - used to count frames for FPS calculation
*/
void PsymsEngine::virtMainLoopDoBeforeUpdate()
{
	// Increment frame counter
	m_iFrameCounter++;

	// Get current time in milliseconds
	int currentTime = getModifiedTime();
	int currentSecond = currentTime / 1000;

	// Check if a second has passed
	if (currentSecond != m_iLastSecond)
	{
		// Update FPS
		m_iFPS = m_iFrameCounter;
		m_iFrameCounter = 0;
		m_iLastSecond = currentSecond;
	}

	// Update tile states to restore tiles after 3 seconds
	if (m_pTileManager != nullptr)
	{
		m_pTileManager->updateTileStates(this, currentTime);
	}

	// Check tile interaction for the TOP automated object (object at index 1)
	PsymsAutomatedObject* topAutoObject = dynamic_cast<PsymsAutomatedObject*>(getDisplayableObject(1));
	if (topAutoObject != nullptr && topAutoObject->isVisible())
	{
		topAutoObject->checkTileInteraction(currentTime);
	}

	//Redrawn every frame, allowing the dynamic text to update
	redrawDisplay();
}

/*
Check for collisions between player and automated objects
*/
void PsymsEngine::virtMainLoopDoAfterUpdate()
{
	checkCollisions();
}

/*
Check for collisions between player (object 0) and automated objects (objects 1 and 2)
*/
void PsymsEngine::checkCollisions()
{
	// Don't check collisions if paused
	if (isPaused())
		return;

	// Get the player object (index 0)
	PsymsPlayerObject* player = dynamic_cast<PsymsPlayerObject*>(getDisplayableObject(0));
	if (player == nullptr || !player->isVisible())
		return;

	// Get player bounding box
	int playerLeft = player->getLeft();
	int playerRight = player->getRight();
	int playerTop = player->getTop();
	int playerBottom = player->getBottom();

	// Check collision with first automated object (index 1)
	PsymsAutomatedObject* auto1 = dynamic_cast<PsymsAutomatedObject*>(getDisplayableObject(1));
	if (auto1 != nullptr && auto1->isVisible())
	{
		// Circle-rectangle collision detection
		int circleCenterX = auto1->getCenterX();
		int circleCenterY = auto1->getCenterY();
		int circleRadius = auto1->getRadius();

		// Find closest point on rectangle to circle center
		int closestX = circleCenterX;
		int closestY = circleCenterY;

		if (circleCenterX < playerLeft) closestX = playerLeft;
		else if (circleCenterX > playerRight) closestX = playerRight;

		if (circleCenterY < playerTop) closestY = playerTop;
		else if (circleCenterY > playerBottom) closestY = playerBottom;

		// Calculate distance from closest point to circle center
		int deltaX = circleCenterX - closestX;
		int deltaY = circleCenterY - closestY;
		int distanceSquared = deltaX * deltaX + deltaY * deltaY;

		// Check if distance is less than radius (collision detected)
		if (distanceSquared < (circleRadius * circleRadius))
		{
			// Collision detected
			m_iCollisionCounter++;
			player->handleCollision();
			auto1->handleCollision();
		}
	}

	// Check collision with second automated object (index 2)
	PsymsAutomatedObject* auto2 = dynamic_cast<PsymsAutomatedObject*>(getDisplayableObject(2));
	if (auto2 != nullptr && auto2->isVisible())
	{
		// Circle-rectangle collision detection
		int circleCenterX = auto2->getCenterX();
		int circleCenterY = auto2->getCenterY();
		int circleRadius = auto2->getRadius();

		// Find closest point on rectangle to circle center
		int closestX = circleCenterX;
		int closestY = circleCenterY;

		if (circleCenterX < playerLeft) closestX = playerLeft;
		else if (circleCenterX > playerRight) closestX = playerRight;

		if (circleCenterY < playerTop) closestY = playerTop;
		else if (circleCenterY > playerBottom) closestY = playerBottom;

		// Calculate distance from closest point to circle center
		int deltaX = circleCenterX - closestX;
		int deltaY = circleCenterY - closestY;
		int distanceSquared = deltaX * deltaX + deltaY * deltaY;

		// Check if distance is less than radius (collision detected)
		if (distanceSquared < (circleRadius * circleRadius))
		{
			// Collision detected
			m_iCollisionCounter++;
			player->handleCollision();
			auto2->handleCollision();
		}
	}
}

/*
Draw strings on top of the scene
*/
void PsymsEngine::virtDrawStringsOnTop()
{
	// Current time display
	int currentTime = getModifiedTime();
	int seconds = (currentTime / 1000) % 60;
	int minutes = (currentTime / 60000) % 60;
	char timeBuffer[64];
	sprintf(timeBuffer, "Time: %02d:%02d", minutes, seconds);
	drawForegroundString(50, 150, timeBuffer, 0xFFFFFF, NULL);

	// Key press counter
	char counterBuffer[64];
	sprintf(counterBuffer, "Counter (C/R): %d", m_iKeyPressCounter);
	drawForegroundString(50, 250, counterBuffer, 0x00FF00, NULL);

	// Collision counter display
	char collisionBuffer[64];
	sprintf(collisionBuffer, "Collisions: %d", m_iCollisionCounter);
	drawForegroundString(50, 320, collisionBuffer, 0xFF6600, NULL);

	// FPS counter
	char fpsBuffer[64];
	sprintf(fpsBuffer, "FPS: %d", m_iFPS);
	drawForegroundString(50, 290, fpsBuffer, 0xFFFF00, NULL);

	//  Millisecond counter
	char msBuffer[64];
	sprintf(msBuffer, "Milliseconds: %d", currentTime);
	drawForegroundString(50, 210, msBuffer, 0xFF00FF, NULL);

	// Static header text
	drawForegroundString(50, 120, " Screen Info ", 0xFFFFFF, NULL);

	// My Title
	drawForegroundString(550, 30, "PSYMS ENGINE", 0xFFFFFF, NULL);

	// Controls
	drawForegroundString(50, 400, "Controls:", 0xFFFFFF, NULL);
	drawForegroundString(50, 430, "WASD / Arrow Keys - Move", 0x00FFFF, NULL);
	drawForegroundString(50, 460, "Left Click - Teleport", 0xFF00FF, NULL);
	drawForegroundString(50, 490, "Space - Pause", 0xFFFF00, NULL);
	drawForegroundString(50, 520, "C - Increment Counter", 0x00FF00, NULL);
	drawForegroundString(50, 550, "R - Reset Counter", 0xFF8800, NULL);

	// Pause indicator
	if (isPaused())
		drawForegroundString(550, 200, " Engine Paused ", 0xFF0000, NULL);
}

/*
Handle key presses - pass to objects 
*/
void PsymsEngine::virtKeyDown(int iKeyCode)
{
	switch (iKeyCode)
	{
	case SDLK_SPACE:
		// Toggle pause state
		if (isPaused())
			unpause();
		else
			pause();
		break;
	case SDLK_ESCAPE:
		setExitWithCode(0); // Exit the program
		break;
	case SDLK_c:
		// Increment the counter
		m_iKeyPressCounter++;
		break;
	case SDLK_r:
		// Reset the counter to 0
		m_iKeyPressCounter = 0;
		break;
	}
}

/*
Handle key releases - pass to objects 
*/
void PsymsEngine::virtKeyUp(int iKeyCode)
{

}

/*
Handle mouse clicks - pass to objects via notifyObjectsAboutMouse
*/
void PsymsEngine::virtMouseDown(int iButton, int iX, int iY)
{

}