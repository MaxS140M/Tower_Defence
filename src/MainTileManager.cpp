#include "header.h"
#include "MainTileManager.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <queue>
#include <fstream>

//Main Tile Manager Constructor
MainTileManager::MainTileManager(int iTileWidth, int iTileHeight, int iMapWidth, int iMapHeight)
	: TileManager(iTileHeight, iTileWidth, iMapWidth, iMapHeight)
	, m_iPortalAnimFrame(0)
	, m_iLastPortalAnimTime(0)
{
	// Load tiles
	m_pathTileImage = ImageManager::loadImage("resources/PathTile.png", true);
	m_grassTile1Image = ImageManager::loadImage("resources/GrassTile1.png", true);
	m_grassTile2Image = ImageManager::loadImage("resources/GrassTile2.png", true);
	m_houseImage = ImageManager::loadImage("resources/base.png", true);

	// Load enemy portal animation frames
	m_portalFrame1 = ImageManager::loadImage("resources/portal1.png", true);
	m_portalFrame2 = ImageManager::loadImage("resources/portal2.png", true);
	m_portalFrame3 = ImageManager::loadImage("resources/portal3.png", true);

	// Load random object images
	for (int i = 0; i < 7; i++)
	{
		std::string objectPath = "resources/object" + std::to_string(i + 1) + ".png";
		m_objectImages[i] = ImageManager::loadImage(objectPath, true);
		std::cout << "Object" << (i + 1) << ".png loaded: " << (m_objectImages[i].exists() ? "YES" : "NO") << std::endl;
	}
	// image load check
	std::cout << "PlaceTile1.png loaded: " << (m_grassTile1Image.exists() ? "YES" : "NO") << std::endl;
	std::cout << "PlaceTile2.png loaded: " << (m_grassTile2Image.exists() ? "YES" : "NO") << std::endl;
	std::cout << "Portal frames loaded: " << (m_portalFrame1.exists() ? "YES" : "NO") << std::endl;
	std::cout << "House.png loaded: " << (m_houseImage.exists() ? "YES" : "NO") << std::endl;

	// Initialize with default level
	createDefaultLevel();

	// Place random objects
	placeRandomObjects(80);
}
void MainTileManager::virtDrawTileAt(
	BaseEngine* pEngine,
	DrawingSurface* pSurface,
	int iMapX, int iMapY,
	int iStartPositionScreenX, int iStartPositionScreenY) const
{
	int tileValue = getMapValue(iMapX, iMapY);

	// Draw animated portal for START tile
	if (tileValue == TILE_START)
	{
		SimpleImage currentFrame;

		// Select frame based on animation
		if (m_iPortalAnimFrame == 0 && m_portalFrame1.exists())
			currentFrame = m_portalFrame1;
		else if (m_iPortalAnimFrame == 1 && m_portalFrame2.exists())
			currentFrame = m_portalFrame2;
		else if (m_iPortalAnimFrame == 2 && m_portalFrame3.exists())
			currentFrame = m_portalFrame3;

		if (currentFrame.exists())
		{
			currentFrame.renderImageWithMask(pSurface, 0, 0,
				iStartPositionScreenX, iStartPositionScreenY,
				getTileWidth(), getTileHeight());
			return;
		}
		else
		{
			// Fallback to red if portal images not loaded
			unsigned int color = 0xFF0000;
			for (int y = iStartPositionScreenY; y < iStartPositionScreenY + getTileHeight(); y++)
			{
				for (int x = iStartPositionScreenX; x < iStartPositionScreenX + getTileWidth(); x++)
				{
					pSurface->setPixel(x, y, color);
				}
			}
			return;
		}
	}

	// END tile
	if (tileValue == TILE_END)
	{
		if (iMapX == 22 && iMapY == 4)
		{
			if (m_houseImage.exists())
			{
				// Stretch over 3x3
				int houseWidth = getTileWidth() * 3;
				int houseHeight = getTileHeight() * 3;
				m_houseImage.renderImageWithMask(pSurface, 0, 0,
					iStartPositionScreenX, iStartPositionScreenY,
					houseWidth, houseHeight);
				return;
			}
			else
			{
				// Fallback to blue if house image not loaded
				unsigned int color = 0x0000FF;
				int houseWidth = getTileWidth() * 3;
				int houseHeight = getTileHeight() * 3;
				for (int y = iStartPositionScreenY; y < iStartPositionScreenY + houseHeight; y++)
				{
					for (int x = iStartPositionScreenX; x < iStartPositionScreenX + houseWidth; x++)
					{
						pSurface->setPixel(x, y, color);
					}
				}
				return;
			}
		}
		else
		{
			//not part of base
			return;
		}
	}

	// PATH and JUNCTION tiles
	if ((tileValue == TILE_PATH || tileValue == TILE_JUNCTION) && m_pathTileImage.exists())
	{
		m_pathTileImage.renderImageWithMask(pSurface, 0, 0,
			iStartPositionScreenX, iStartPositionScreenY,
			getTileWidth(), getTileHeight());
		return;
	}

	// OBJECT tiles - draw grass background
	if (tileValue == TILE_OBJECT)
	{
		bool isCheckerDark = (iMapX + iMapY) % 2 == 0;

		if (isCheckerDark && m_grassTile1Image.exists())
		{
			m_grassTile1Image.renderImage(pSurface, 0, 0,
				iStartPositionScreenX, iStartPositionScreenY,
				getTileWidth(), getTileHeight());
		}
		else if (!isCheckerDark && m_grassTile2Image.exists())
		{
			m_grassTile2Image.renderImage(pSurface, 0, 0,
				iStartPositionScreenX, iStartPositionScreenY,
				getTileWidth(), getTileHeight());
		}
		// Draw object on top 
		int tileKey = getTileKey(iMapX, iMapY);
		auto it = m_tileObjects.find(tileKey);
		if (it != m_tileObjects.end())
		{
			int objectIndex = it->second;
			if (objectIndex >= 0 && objectIndex < 7 && m_objectImages[objectIndex].exists())
			{
				const int OBJECT_SIZE = 32;

				int offsetX = (getTileWidth() - OBJECT_SIZE) / 2;
				int offsetY = (getTileHeight() - OBJECT_SIZE) / 2;

				m_objectImages[objectIndex].renderImageWithMask(pSurface, 0, 0,
					iStartPositionScreenX + offsetX,
					iStartPositionScreenY + offsetY,
					OBJECT_SIZE, OBJECT_SIZE);
			}
		}
		return;
	}

	// GRASS tiles checkered pattern
	if (tileValue == TILE_TOWER || tileValue == TILE_GRASS)
	{
		bool isCheckerDark = (iMapX + iMapY) % 2 == 0;

		if (isCheckerDark && m_grassTile1Image.exists())
		{
			m_grassTile1Image.renderImage(pSurface, 0, 0,
				iStartPositionScreenX, iStartPositionScreenY,
				getTileWidth(), getTileHeight());
			return;
		}
		else if (!isCheckerDark && m_grassTile2Image.exists())
		{
			m_grassTile2Image.renderImage(pSurface, 0, 0,
				iStartPositionScreenX, iStartPositionScreenY,
				getTileWidth(), getTileHeight());
			return;
		}
		else
		{
			// Fallback
			unsigned int color = isCheckerDark ? 0xFF0000 : 0x00FF00;
			for (int y = iStartPositionScreenY; y < iStartPositionScreenY + getTileHeight(); y++)
			{
				for (int x = iStartPositionScreenX; x < iStartPositionScreenX + getTileWidth(); x++)
				{
					pSurface->setPixel(x, y, color);
				}
			}
			return;
		}
	}
}

unsigned int MainTileManager::getTileColor(int tileValue) const
{
	switch (tileValue)
	{
	case TILE_GRASS:
		return 0x228B22;
	case TILE_PATH:
		return 0x8B4513;
	case TILE_START:
		return 0xFF0000;
	case TILE_END:
		return 0x0000FF;
	case TILE_TOWER:
		return 0x228B22;
	case TILE_BLOCKED:
		return 0x696969;
	case TILE_JUNCTION:
		return 0xFFFF00;
	default:
		return 0x000000;
	}
}
//check if tower can be place
bool MainTileManager::canPlaceTower(int iMapX, int iMapY) const
{
	//check if coordinates are valid
	if (iMapX < 0 || iMapX >= getMapWidth() || iMapY < 0 || iMapY >= getMapHeight())
		return false;

	int tileValue = getMapValue(iMapX, iMapY);

	//can only place towers on grass tiles
	return (tileValue == TILE_GRASS);
}

//place tower method
void MainTileManager::placeTower(int iMapX, int iMapY, BaseEngine* pEngine)
{
	if (canPlaceTower(iMapX, iMapY))
	{
		setMapValue(iMapX, iMapY, TILE_TOWER);
		setAndRedrawMapValueAt(iMapX, iMapY, TILE_TOWER, pEngine, pEngine->getBackgroundSurface());
		setAndRedrawMapValueAt(iMapX, iMapY, TILE_TOWER, pEngine, pEngine->getForegroundSurface());
	}
}
//remove tower method
void MainTileManager::removeTower(int iMapX, int iMapY, BaseEngine* pEngine)
{
	if (getMapValue(iMapX, iMapY) == TILE_TOWER)
	{
		setMapValue(iMapX, iMapY, TILE_GRASS);
		setAndRedrawMapValueAt(iMapX, iMapY, TILE_GRASS, pEngine, pEngine->getBackgroundSurface());
		setAndRedrawMapValueAt(iMapX, iMapY, TILE_GRASS, pEngine, pEngine->getForegroundSurface());
	}
}
// Update Enemy Portal tile animation
void MainTileManager::updateTileAnimations(BaseEngine* pEngine, int currentTime)
{
	// Only portal animation remains
	if (currentTime - m_iLastPortalAnimTime > 400)
	{
		m_iPortalAnimFrame = (m_iPortalAnimFrame + 1) % 3;
		m_iLastPortalAnimTime = currentTime;

		int startX, startY;
		if (getStartPosition(startX, startY))
		{
			drawToSurface(pEngine, pEngine->getBackgroundSurface(), startX, startY);
			pEngine->redrawDisplay();
		}
	}
}

//Helper method to create striaght pathh
void MainTileManager::createStraightPath(int startX, int startY, int endX, int endY)
{
	// Create horizontal path
	if (startY == endY)
	{
		int minX = (startX < endX) ? startX : endX;
		int maxX = (startX > endX) ? startX : endX;
		for (int x = minX; x <= maxX; x++)
		{
			setMapValue(x, startY, TILE_PATH);
		}
	}
	// Create vertical path
	else if (startX == endX)
	{
		int minY = (startY < endY) ? startY : endY;
		int maxY = (startY > endY) ? startY : endY;
		for (int y = minY; y <= maxY; y++)
		{
			setMapValue(startX, y, TILE_PATH);
		}
	}
}

void MainTileManager::getTileCenterPosition(int iMapX, int iMapY, int& outX, int& outY) const
{
	// Return center position in world coordinates
	outX = iMapX * getTileWidth() + getTileWidth() / 2;
	outY = iMapY * getTileHeight() + getTileHeight() / 2;
}

bool MainTileManager::getStartPosition(int& x, int& y) const
{
	// Find the first START tile
	for (int iY = 0; iY < getMapHeight(); iY++)
	{
		for (int iX = 0; iX < getMapWidth(); iX++)
		{
			if (getMapValue(iX, iY) == TILE_START)
			{
				x = iX;
				y = iY;
				return true;
			}
		}
	}
	return false;
}

bool MainTileManager::getEndPosition(int& x, int& y) const
{
	// Return the bottom of base co-ordinates
	for (int iY = 0; iY < getMapHeight(); iY++)
	{
		for (int iX = 0; iX < getMapWidth(); iX++)
		{
			if (getMapValue(iX, iY) == TILE_END)
			{
				x = 23;
				y = 6; 
				return true;
			}
		}
	}
	return false;
}
//Create Level
void MainTileManager::createDefaultLevel()
{
	// Initialize all tiles as grass
	int w = getMapWidth();
	int h = getMapHeight();

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			setMapValue(x, y, TILE_GRASS);
		}
	}

	// START 
	setMapValue(0, 7, TILE_START);

	//  INITIAL SHARED PATH 
	setMapValue(1, 7, TILE_PATH);
	setMapValue(2, 7, TILE_PATH);
	setMapValue(3, 7, TILE_PATH);
	setMapValue(4, 7, TILE_PATH);

	// FIRST JUNCTION at (5, 7) 
	setMapValue(5, 7, TILE_JUNCTION);

	// UPPER ROUTE
	setMapValue(5, 7 - 1, TILE_PATH);
	setMapValue(5, 7 - 2, TILE_PATH); 
	setMapValue(5, 7 - 3, TILE_PATH);  
	setMapValue(5, 7 - 4, TILE_PATH); 

	// Go across at y=3
	for (int x = 6; x <= 19; x++)
	{
		setMapValue(x, 3, TILE_PATH);
	}
	// Come back down
	setMapValue(19, 4, TILE_PATH);
	setMapValue(19, 5, TILE_PATH);
	setMapValue(19, 6, TILE_PATH);

	// LOWER ROUTE 
	setMapValue(5, 7 + 1, TILE_PATH);
	setMapValue(5, 7 + 2, TILE_PATH);
	setMapValue(5, 7 + 3, TILE_PATH);
	setMapValue(5, 7 + 4, TILE_PATH); 

	// Go across at y=11
	for (int x = 6; x <= 19; x++)
	{
		setMapValue(x, 11, TILE_PATH);
	}

	// Come back up
	setMapValue(19, 10, TILE_PATH);
	setMapValue(19, 9, TILE_PATH);
	setMapValue(19, 8, TILE_PATH);

	//  MERGE JUNCTION at (19, 7)
	setMapValue(19, 7, TILE_JUNCTION);

	// === FINAL SHARED PATH ===
	setMapValue(20, 7, TILE_PATH);
	setMapValue(21, 7, TILE_PATH);
	setMapValue(22, 7, TILE_PATH);
	setMapValue(23, 7, TILE_PATH);

	// END / Base
	setMapValue(22, 4, TILE_END);
	setMapValue(23, 4, TILE_END);
	setMapValue(24, 4, TILE_END);
	setMapValue(22, 5, TILE_END);
	setMapValue(23, 5, TILE_END);
	setMapValue(24, 5, TILE_END);
	setMapValue(22, 6, TILE_END);
	setMapValue(23, 6, TILE_END);
	setMapValue(24, 6, TILE_END);
}

//is junction helper method
bool MainTileManager::isJunction(int x, int y) const
{
    return getMapValue(x, y) == TILE_JUNCTION;
}

std::vector<std::pair<int, int>> MainTileManager::getJunctions() const
{
    std::vector<std::pair<int, int>> junctions;
    
    for (int y = 0; y < getMapHeight(); y++)
    {
        for (int x = 0; x < getMapWidth(); x++)
        {
            if (isJunction(x, y))
            {
                junctions.push_back(std::make_pair(x, y));
            }
        }
    }

    return junctions;
}
//Map Reset method
void MainTileManager::resetMap()
{
	// Remove all towers from the map, resetting to original state
	for (int x = 0; x < getMapWidth(); x++)
	{
		for (int y = 0; y < getMapHeight(); y++)
		{
			int value = getMapValue(x, y);
			// If it's a tower, convert back to grass
			if (value == TILE_TOWER)
			{
				setMapValue(x, y, TILE_GRASS);
			}
		}
	}
}
//Load Map method
void MainTileManager::loadMap(const std::vector<std::vector<int>>& mapData)
{
    if (mapData.empty())
        return;
    
    int height = mapData.size();
    int width = mapData[0].size();
    
    for (int y = 0; y < height && y < getMapHeight(); y++)
    {
        for (int x = 0; x < width && x < getMapWidth(); x++)
        {
            setMapValue(x, y, mapData[y][x]);
        }
    }
}
//place random ojects method
void MainTileManager::placeRandomObjects(int count)
{
	std::vector<std::pair<int, int>> buildableTiles;

	// Find all buildable tiles
	for (int y = 0; y < getMapHeight(); y++)
	{
		for (int x = 0; x < getMapWidth(); x++)
		{
			if (getMapValue(x, y) == TILE_GRASS)
			{
				buildableTiles.push_back(std::make_pair(x, y));
			}
		}
	}

	// Shuffle and place objects
	if (!buildableTiles.empty())
	{
		// Seed random number generator
		srand((unsigned int)time(nullptr));

		// Randomly shuffle buildable tiles
		for (size_t i = buildableTiles.size() - 1; i > 0; i--)
		{
			size_t j = rand() % (i + 1);
			std::swap(buildableTiles[i], buildableTiles[j]);
		}

		// Place objects on the first 'count' tiles
		int objectsPlaced = 0;
		for (size_t i = 0; i < buildableTiles.size() && objectsPlaced < count; i++)
		{
			int x = buildableTiles[i].first;
			int y = buildableTiles[i].second;

			// Set tile to TILE_OBJECT
			setMapValue(x, y, TILE_OBJECT);

			// Store which object image to use (random 0-5)
			int objectIndex = rand() % 7;
			m_tileObjects[getTileKey(x, y)] = objectIndex;

			objectsPlaced++;
		}
	}
}