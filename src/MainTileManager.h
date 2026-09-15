#ifndef MAINTILEMANAGER_H
#define MAINTILEMANAGER_H

#include "TileManager.h"
#include "BaseEngine.h"
#include "ImageManager.h"
#include <map>

class MainTileManager : public TileManager
{
public:
	// Tile types for tower defense
	enum TileType
	{
		TILE_GRASS = 0,      // Empty, can place towers
		TILE_PATH = 1,       // Enemy path, cannot place towers
		TILE_START = 2,      // Enemy spawn point
		TILE_END = 3,        // Enemy goal/exit
		TILE_TOWER = 4,      // Has a tower placed
		TILE_BLOCKED = 5,    // Blocked terrain, cannot place towers
		TILE_JUNCTION = 6,   // Path junction/decision point
		TILE_OBJECT = 7      // Has a random object, cannot place towers
	};

	void resetMap();

	/**
	Constructor
	Creates a tile manager for tower defense with specified dimensions
	*/
	MainTileManager(int iTileWidth, int iTileHeight, int iMapWidth, int iMapHeight);

	/**
	Destructor
	*/
	virtual ~MainTileManager() {}

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
	Check if a tower can be placed at the specified tile coordinates
	Returns true if the tile is valid for tower placement
	*/
	bool canPlaceTower(int iMapX, int iMapY) const;

	/**
	Place a tower at the specified tile coordinates
	Updates both background and foreground surfaces with animation
	*/
	void placeTower(int iMapX, int iMapY, BaseEngine* pEngine);

	/**
	Remove a tower from the specified tile coordinates
	Restores the tile to grass and updates surfaces with animation
	*/
	void removeTower(int iMapX, int iMapY, BaseEngine* pEngine);

	/**
	Create a straight path between two points
	Used for creating enemy paths
	*/
	void createStraightPath(int startX, int startY, int endX, int endY);

	/**
	Get the screen position (center) of a tile
	Useful for spawning enemies or positioning objects
	*/
	void getTileCenterPosition(int iMapX, int iMapY, int& outX, int& outY) const;

	/**
	Find the start position for enemy spawning
	Returns true if a start tile is found
	*/
	bool getStartPosition(int& x, int& y) const;

	/**
	Find the end position (goal) for enemies
	Returns true if an end tile is found
	*/
	bool getEndPosition(int& x, int& y) const;

	/**
	Initialize a simple default level layout
	Creates a basic path from start to end
	*/
	void createDefaultLevel();

	/**
	Update tile animations - call this each frame
	*/
	void updateTileAnimations(BaseEngine* pEngine, int currentTime);

	/**
	Load a map from a 2D vector array
	Overrides the current map with the given data
	*/
	void loadMap(const std::vector<std::vector<int>>& mapData);

	/**
	Check if a tile is a junction (decision point)
	*/
	bool isJunction(int x, int y) const;

	/**
	Get all junction positions on the map
	*/
	std::vector<std::pair<int, int>> getJunctions() const;

	/**
	Place random objects on buildable tiles
	*/
	void placeRandomObjects(int count);

private:
	// Images for tiles
	SimpleImage m_pathTileImage;
	SimpleImage m_grassTile1Image;
	SimpleImage m_grassTile2Image;
	SimpleImage m_houseImage;  // NEW: House image for player's base

	// Enemy spawn portal animation frames
	SimpleImage m_portalFrame1;
	SimpleImage m_portalFrame2;
	SimpleImage m_portalFrame3;
	int m_iPortalAnimFrame;
	int m_iLastPortalAnimTime;

	// Random object images
	SimpleImage m_objectImages[7];

	// Map to store which object index is at each tile
	std::map<int, int> m_tileObjects; // Key: tile position, Value: object index (0-5)

	/**
	Get the color for a specific tile type
	*/
	unsigned int getTileColor(int tileValue) const;

	// Structure to track tile animations
	struct TileAnimation
	{
		int startTime;
		int duration;
		unsigned int startColor;
		unsigned int endColor;
		bool active;
	};

	// Map of tile positions to their animation states
	mutable std::map<int, TileAnimation> m_tileAnimations;

	// Helper to get map key from coordinates
	int getTileKey(int x, int y) const { return y * getMapWidth() + x; }
};

#endif