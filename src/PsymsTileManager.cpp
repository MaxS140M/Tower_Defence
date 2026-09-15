#include "header.h"
#include "PsymsTileManager.h"
#include <cmath>

/*
Constructor - initialize the tile manager with custom dimensions
*/
PsymsTileManager::PsymsTileManager(int iTileWidth, int iTileHeight, int iNumRows, int iNumCols)
	: TileManager(iTileWidth, iTileHeight)
{
	// Set the map size
	setMapSize(iNumCols, iNumRows);

	// Create tiles - all start as white (value 0)
	for (int row = 0; row < iNumRows; row++)
	{
		for (int col = 0; col < iNumCols; col++)
		{
			// All tiles start as white
			setMapValue(col, row, 0);
		}
	}
}

/*
Get color for a tile based on its value
*/
unsigned int PsymsTileManager::getTileColor(int tileValue, int row, int col) const
{
	// Base colors for different tile types
	unsigned int baseColor;

	switch (tileValue)
	{
	case 0: // orange (default)
		baseColor = 0xF54927;
		break;
	case -1: // Black tiles (touched by ball)
		baseColor = 0x000000;
		break;
	default:
		baseColor = 0x808080; // Gray fallback
		break;
	}

	return baseColor;
}

/*
Check if a screen position is over a tile and return the tile coordinates
*/
bool PsymsTileManager::isPositionOverTile(int screenX, int screenY, int& outTileX, int& outTileY) const
{
	// Get the tile manager's top-left screen position from protected member variables
	int topLeftX = m_iBaseScreenX;
	int topLeftY = m_iBaseScreenY;

	// Convert screen coordinates to tile coordinates
	int relativeX = screenX - topLeftX;
	int relativeY = screenY - topLeftY;

	// Check if position is within the tile grid bounds
	if (relativeX < 0 || relativeY < 0)
		return false;

	// Calculate which tile this position is over
	outTileX = relativeX / getTileWidth();
	outTileY = relativeY / getTileHeight();

	// Verify the tile coordinates are within the map bounds
	if (outTileX < 0 || outTileX >= getMapWidth() || outTileY < 0 || outTileY >= getMapHeight())
		return false;

	return true;
}

/*
Temporarily change a tile to black
*/
void PsymsTileManager::setTileToBlackTemporarily(BaseEngine* pEngine, int tileX, int tileY, int currentTime)
{
	// Check if tile coordinates are valid
	if (tileX < 0 || tileX >= getMapWidth() || tileY < 0 || tileY >= getMapHeight())
		return;

	// Create unique key for this tile
	int tileKey = tileY * 1000 + tileX;

	// Check if this tile is already blackened
	if (m_tileStates.find(tileKey) != m_tileStates.end())
	{
		// Tile is already blackened, update the time to extend the duration
		m_tileStates[tileKey].changeTime = currentTime;
		return;
	}

	// Store the original tile value
	TileState state;
	state.originalValue = getMapValue(tileX, tileY);
	state.changeTime = currentTime;
	state.isBlackened = true;

	m_tileStates[tileKey] = state;

	// Change the tile value to -1
	setMapValue(tileX, tileY, -1);

	// Redraw the tile to the background surface immediately
	drawAllTiles(pEngine, pEngine->getBackgroundSurface());
	pEngine->redrawDisplay();
}

/*
Update tile states - restore tiles after 3 seconds
*/
void PsymsTileManager::updateTileStates(BaseEngine* pEngine, int currentTime)
{
	std::vector<int> tilesToRestore;

	// Check all blackened tiles
	for (auto& pair : m_tileStates)
	{
		int tileKey = pair.first;
		TileState& state = pair.second;

		// Check if 3 seconds (3000 milliseconds) have passed
		if (state.isBlackened && (currentTime - state.changeTime) >= 3000)
		{
			// Extract tile coordinates from key
			int tileY = tileKey / 1000;
			int tileX = tileKey % 1000;

			// Restore the original tile value
			setMapValue(tileX, tileY, state.originalValue);

			// Mark for removal from tracking
			tilesToRestore.push_back(tileKey);
		}
	}

	// Remove restored tiles from the state map
	for (int key : tilesToRestore)
	{
		m_tileStates.erase(key);
	}

	// If any tiles were restored, redraw the tile manager
	if (!tilesToRestore.empty())
	{
		drawAllTiles(pEngine, pEngine->getBackgroundSurface());
		pEngine->redrawDisplay();
	}
}

/*
Draw a tile at the specified position
*/
void PsymsTileManager::virtDrawTileAt(
	BaseEngine* pEngine,
	DrawingSurface* pSurface,
	int iMapX, int iMapY,
	int iStartPositionScreenX, int iStartPositionScreenY) const
{
	// Get the tile value at this map position
	int tileValue = getMapValue(iMapX, iMapY);
	int screenX = iStartPositionScreenX;
	int screenY = iStartPositionScreenY;

	// Get the color for this tile
	unsigned int tileColor = getTileColor(tileValue, iMapY, iMapX);

	// Draw the filled tile by drawing each pixel
	for (int y = screenY; y < screenY + getTileHeight(); y++)
	{
		for (int x = screenX; x < screenX + getTileWidth(); x++)
		{
			pSurface->setPixel(x, y, tileColor);
		}
	}

	// Draw a thin border around each tile for visual separation
	// Top border
	for (int x = screenX; x < screenX + getTileWidth(); x++)
	{
		pSurface->setPixel(x, screenY, 0x808080); // Gray border
	}
	// Bottom border
	for (int x = screenX; x < screenX + getTileWidth(); x++)
	{
		pSurface->setPixel(x, screenY + getTileHeight() - 1, 0x808080);
	}
	// Left border
	for (int y = screenY; y < screenY + getTileHeight(); y++)
	{
		pSurface->setPixel(screenX, y, 0x808080);
	}
	// Right border
	for (int y = screenY; y < screenY + getTileHeight(); y++)
	{
		pSurface->setPixel(screenX + getTileWidth() - 1, y, 0x808080);
	}
}