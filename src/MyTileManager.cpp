#include "header.h"
#include "MyTileManager.h"

void MyTileManager::virtDrawTileAt(
	BaseEngine* pEngine,
	DrawingSurface* pSurface,
	int iMapX, int iMapY,
	int iStartPositionScreenX, int iStartPositionScreenY) const
{
	// Get the map value for this tile
	int iMapValue = getMapValue(iMapX, iMapY);

	// Extract color from the lowest 12 bits of the random value
	unsigned int iColour = (unsigned int)((iMapValue & 0xf00) << 12)  // red
		+ (unsigned int)((iMapValue & 0xf0) << 8)   // green
		+ (unsigned int)((iMapValue & 0xf) << 4);   // blue

	// Draw an oval for each tile
	pSurface->drawOval(
		iStartPositionScreenX,                          // Left
		iStartPositionScreenY,                          // Top
		iStartPositionScreenX + getTileWidth() - 1,     // Right
		iStartPositionScreenY + getTileHeight() - 1,    // Bottom
		iColour);
}