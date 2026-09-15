#pragma once

#include "TileManager.h"

class MyTileManager : public TileManager
{
public:
	// Constructor - sets tile size to 20x20 pixels and map size to 15x15 tiles
	MyTileManager()
		: TileManager(20, 20, 15, 15)
	{
	}

	// Override the drawing function to customize how tiles appear
	virtual void virtDrawTileAt(
		BaseEngine* pEngine,
		DrawingSurface* pSurface,
		int iMapX, int iMapY,
		int iStartPositionScreenX, int iStartPositionScreenY) const override;
};