#ifndef PSYMSTILEMANAGER_H
#define PSYMSTILEMANAGER_H

#include "TileManager.h"
#include "BaseEngine.h"
#include <map>
#include <vector>

class PsymsTileManager : public TileManager
{
public:
	/**
	Constructor
	Creates a tile manager with custom size and grid dimensions
	*/
	PsymsTileManager(int iTileWidth, int iTileHeight, int iNumRows, int iNumCols);

	/**
	Destructor
	*/
	virtual ~PsymsTileManager() {}

	/**
	Draw a specific tile at a specific location
	This is called by the base TileManager when drawing tiles
	*/
	virtual void virtDrawTileAt(
		BaseEngine* pEngine,
		DrawingSurface* pSurface,
		int iMapX, int iMapY,
		int iStartPositionScreenX, int iStartPositionScreenY) const override;

	/**
	Check if a screen position is over a tile and return the tile coordinates
	Returns true if position is over a valid tile
	*/
	bool isPositionOverTile(int screenX, int screenY, int& outTileX, int& outTileY) const;

	/**
	Temporarily change a tile to black revert after 3 seconds
	*/
	void setTileToBlackTemporarily(BaseEngine* pEngine, int tileX, int tileY, int currentTime);

	/**
	Update tile states, restore tiles that have been black for 3 seconds
	*/
	void updateTileStates(BaseEngine* pEngine, int currentTime);

private:
	/**
	Get the color for a tile based on its value and position
	*/
	unsigned int getTileColor(int tileValue, int row, int col) const;

	/**
	Structure to track temporary tile state changes
	*/
	struct TileState
	{
		int originalValue;  // The original tile value before change
		int changeTime;     // Time when tile was changed
		bool isBlackened;   // Whether this tile is currently blackened
	};

	// Map of tile coordinates to their temporary state
	// Key is encoded as tileY * 1000 + tileX for ID
	mutable std::map<int, TileState> m_tileStates;
};

#endif