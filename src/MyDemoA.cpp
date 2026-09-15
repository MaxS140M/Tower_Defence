#include "header.h"
#include "MyDemoA.h"
#include "ImageManager.h"
#include "MyObjectB.h"

void MyDemoA::virtSetupBackgroundBuffer()
{
	// Fill background with black
	fillBackground(0x000000);

	// Create a starfield background
	for (int iX = 0; iX < getWindowWidth(); iX++)
	{
		for (int iY = 0; iY < getWindowHeight(); iY++)
		{
			switch (rand() % 100)
			{
			case 0: setBackgroundPixel(iX, iY, 0xFF0000); break; // Red
			case 1: setBackgroundPixel(iX, iY, 0x00FF00); break; // Green
			case 2: setBackgroundPixel(iX, iY, 0x0000FF); break; // Blue
			case 3: setBackgroundPixel(iX, iY, 0xFFFF00); break; // Yellow
			case 4: setBackgroundPixel(iX, iY, 0x00FFFF); break; // Cyan
			case 5: setBackgroundPixel(iX, iY, 0xFF00FF); break; // Magenta
			}
		}
	}

	// Optional: Draw an image if demo.png exists
	// SimpleImage image = ImageManager::loadImage("demo.png", true);
	// image.renderImage(getBackgroundSurface(), 0, 0, 10, 10,
	//     image.getWidth(), image.getHeight());

	// Initialize and draw the tile manager
	for (int i = 0; i < 15; i++)
	{
		for (int j = 0; j < 15; j++)
		{
			tm.setMapValue(i, j, rand());
		}
	}

	tm.setTopLeftPositionOnScreen(50, 50);
	tm.drawAllTiles(this, getBackgroundSurface());
}

void MyDemoA::virtMouseDown(int iButton, int iX, int iY)
{
	printf("Mouse clicked at %d %d\n", iX, iY);

	if (iButton == SDL_BUTTON_LEFT)
	{
		// Handle left click on tiles
		if (tm.isValidTilePosition(iX, iY))
		{
			int mapX = tm.getMapXForScreenX(iX);
			int mapY = tm.getMapYForScreenY(iY);
			int value = tm.getMapValue(mapX, mapY);
			tm.setAndRedrawMapValueAt(mapX, mapY, value + rand(), this, getBackgroundSurface());
			redrawDisplay();
		}
	}
	else if (iButton == SDL_BUTTON_RIGHT)
	{
		// Draw a blue oval on right click
		lockBackgroundForDrawing();
		drawBackgroundOval(iX - 10, iY - 10, iX + 10, iY + 10, 0x0000ff);
		unlockBackgroundForDrawing();
		redrawDisplay();
	}
}

void MyDemoA::virtKeyDown(int iKeyCode)
{
	switch (iKeyCode)
	{
	case SDLK_SPACE:
		// Redraw the entire background when space is pressed
		lockBackgroundForDrawing();
		virtSetupBackgroundBuffer();
		unlockBackgroundForDrawing();
		redrawDisplay();
		break;
	}
}

int MyDemoA::virtInitialiseObjects()
{
	// Record the fact that we are about to change the array
	// so it doesn't get used elsewhere without reloading it
	drawableObjectsChanged();

	// Destroy any existing objects
	destroyOldObjects(true);

	// Creates an array big enough for the number of objects that you want
	createObjectArray(1);

	// Store the object in the array
	// You MUST set the array entry after the last one that you create to NULL,
	// so that the system knows when to stop
	storeObjectInArray(0, new MyObjectB(this));

	// NOTE: We also need to destroy the objects, but the method at the
	// top of this function will destroy all objects pointed at by the
	// array elements so we can ignore that here

	// Make all objects visible
	setAllObjectsVisible(true);

	return 0;
}