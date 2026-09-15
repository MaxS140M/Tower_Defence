#include "header.h"
#include "EnemyObject.h"
#include "MainEngine.h"
#include "MainTileManager.h"
#include "TowerObject.h"
#include <cmath>


//Enemy Object Constructor
EnemyObject::EnemyObject(MainEngine* pEngine, int wave)
	: GameObject(pEngine)
	, m_iMoneyReward(1)
	, m_bReachedBase(false)
	, m_iCurrentPathIndex(0)
	, m_iCurrentFrame(0)
	, m_iLastAnimTime(0)
	, m_iAnimDelay(150)
	, m_bDeathAnimComplete(false)
	, m_iChosenRouteIndex(-1)
	, m_fWorldX(0.0f)
	, m_fWorldY(0.0f)
{
	// Load sprite sheets
	m_walkSprite = ImageManager::loadImage("sprites/Enemy_walk.png", true);
	m_deathSprite = ImageManager::loadImage("sprites/Enemy_death.png", true);

	m_iMaxHealth = 25 + (2 * wave * 25);

	if (wave >= 7)
	{
		m_iMaxHealth *= 2;
	}

	m_iHealth = m_iMaxHealth;

	if (wave <= 3)
		m_iSpeed = 1;  // medium on waves 2-3
	else if (wave <= 6)
		m_iSpeed = (int)1.5;  // fast on waves 4-6
	else
		m_iSpeed = (int)1.6;  // very fast on wave 7+


	m_iMoneyReward = (int)pow(2, wave);

	m_iWidth = 32;
	m_iHeight = 32;
	setSize(32, 32);
	setVisible(true);

	buildCompletePath();

	if (!m_completePath.empty())
	{
		MainTileManager* tm = m_pMainEngine->getTileManager();
		int centerX, centerY;
		tm->getTileCenterPosition(m_completePath[0].first, m_completePath[0].second, centerX, centerY);
		m_fWorldX = (float)(centerX - getWidth() / 2);
		m_fWorldY = (float)(centerY - getHeight() / 2);
		m_iWorldX = (int)m_fWorldX;
		m_iWorldY = (int)m_fWorldY;

		if (m_completePath.size() > 1)
		{
			tm->getTileCenterPosition(m_completePath[1].first, m_completePath[1].second, m_iTargetX, m_iTargetY);
			m_iCurrentPathIndex = 1;
		}
	}
}

void EnemyObject::buildCompletePath()
{
	m_completePath.clear();

	const auto& mainPath = m_pMainEngine->getMainPath();
	const auto& routes = m_pMainEngine->getAllRoutes();
	const auto& finalPath = m_pMainEngine->getFinalPath();

	if (routes.empty())
	{
		m_completePath = m_pMainEngine->getEnemyPath();
		return;
	}

	// Add main path
	m_completePath.insert(m_completePath.end(), mainPath.begin(), mainPath.end());

	// AI DECISION - Choose best route
	int bestRouteIndex = 0;
	int lowestDanger = 999999;

	// DEBUG: Print evaluation for each route
	for (size_t i = 0; i < routes.size(); i++)
	{
		int danger = evaluateRoute(routes[i].waypoints);
		//Ai Decision making
		std::cout << "Route " << i << " danger score: " << danger << std::endl;

		if (danger < lowestDanger)
		{
			lowestDanger = danger;
			bestRouteIndex = (int)i;
		}
	}

	m_iChosenRouteIndex = bestRouteIndex;

	// Add chosen route
	const auto& chosenRoute = routes[bestRouteIndex].waypoints;
	m_completePath.insert(m_completePath.end(), chosenRoute.begin(), chosenRoute.end());

	// Add final path
	m_completePath.insert(m_completePath.end(), finalPath.begin(), finalPath.end());
}

//Helper method:
int EnemyObject::evaluateRoute(const std::vector<std::pair<int, int>>& route)
{
	// Count towers within range of path
	int dangerScore = countTowersNearPath(route, 225);

	return dangerScore;
}

//Helper Method: Count Towers Near path method
int EnemyObject::countTowersNearPath(const std::vector<std::pair<int, int>>& pathSegment, int range)
{
	const auto& towers = m_pMainEngine->getTowers();
	MainTileManager* tm = m_pMainEngine->getTileManager();

	int towerCount = 0;
	std::vector<bool> towerCounted(towers.size(), false);

	for (const auto& waypoint : pathSegment)
	{
		int waypointWorldX, waypointWorldY;
		tm->getTileCenterPosition(waypoint.first, waypoint.second, waypointWorldX, waypointWorldY);

		for (size_t towerIdx = 0; towerIdx < towers.size(); towerIdx++)
		{
			if (towerCounted[towerIdx])
				continue;  // Already counted this tower

			const auto& tower = towers[towerIdx];
			// Use actual center based on tower's size
			int towerCenterX = tower->getWorldX() + tower->getWidth() / 2;
			int towerCenterY = tower->getWorldY() + tower->getHeight() / 2;

			int dx = towerCenterX - waypointWorldX;
			int dy = towerCenterY - waypointWorldY;
			int distSquared = dx * dx + dy * dy;

			if (distSquared <= range * range)
			{
				towerCount++;
				towerCounted[towerIdx] = true;  // Mark counted
			}
		}
	}

	return towerCount;
}

//Method to draw enemy sprites
void EnemyObject::virtDraw()
{
	if (!isVisible())
		return;

	DrawingSurface* pSurface = getEngine()->getForegroundSurface();

	int centerX = getXCentre();
	int centerY = getYCentre();

	const int SPRITE_WIDTH = 16;
	const int SPRITE_HEIGHT = 25;
	const float SCALE = 2.0f;  
	int drawWidth = (int)(SPRITE_WIDTH * SCALE);
	int drawHeight = (int)(SPRITE_HEIGHT * SCALE);

	int drawX = centerX - drawWidth / 2;
	int drawY = centerY - drawHeight / 2;

	// Choose which sprite sheet to use
	SimpleImage* currentSprite = nullptr;
	int maxFrames = 0;

	if (isDead())
	{
		currentSprite = &m_deathSprite;
		maxFrames = 7;  // 7 death frames
	}
	else
	{
		currentSprite = &m_walkSprite;
		maxFrames = 8;  // 8 walk frames
	}

	if (currentSprite->exists())
	{
		// Calculate source X position (frames are horizontal)
		int srcX = m_iCurrentFrame * SPRITE_WIDTH;
		int srcY = 0;  // All frames are on the same row

		// Draw the sprite
		for (int py = 0; py < drawHeight; py++)
		{
			for (int px = 0; px < drawWidth; px++)
			{
				int srcPX = (int)(px / SCALE);
				int srcPY = (int)(py / SCALE);

				if (srcPX >= SPRITE_WIDTH) srcPX = SPRITE_WIDTH - 1;
				if (srcPY >= SPRITE_HEIGHT) srcPY = SPRITE_HEIGHT - 1;

				int pixelColor = currentSprite->getPixelColour(srcX + srcPX, srcY + srcPY);

				// Skip transparent pixels (assuming black is transparent)
				if (pixelColor == 0x000000)
					continue;

				int targetX = drawX + px;
				int targetY = drawY + py;

				if (targetX >= 0 && targetX < getEngine()->getWindowWidth() &&
					targetY >= 0 && targetY < getEngine()->getWindowHeight())
				{
					pSurface->setPixel(targetX, targetY, pixelColor);
				}
			}
		}
	}
	//Fallback if spritesheet didnt load
	else
	{
		// Fallback to colored circle
		unsigned int color = isDead() ? 0x808080 : 0xFF0000;

		int radius = 10;
		for (int y = -radius; y <= radius; y++)
		{
			int width = (int)sqrt(radius * radius - y * y);
			for (int x = -width; x <= width; x++)
			{
				int px = centerX + x;
				int py = centerY + y;
				if (px >= 0 && px < getEngine()->getWindowWidth() &&
					py >= 0 && py < getEngine()->getWindowHeight())
				{
					pSurface->setPixel(px, py, color);
				}
			}
		}
	}

	// Draw health bar
	if (!isDead())
	{
		int barWidth = 40;
		int barHeight = 4;
		int barX = centerX - barWidth / 2;
		int barY = centerY - drawHeight / 2 - 8;

		// Background (black)
		for (int y = barY; y < barY + barHeight; y++)
		{
			for (int x = barX; x < barX + barWidth; x++)
			{
				if (x >= 0 && x < getEngine()->getWindowWidth() &&
					y >= 0 && y < getEngine()->getWindowHeight())
				{
					pSurface->setPixel(x, y, 0x000000);
				}
			}
		}

		// Health (green)
		int healthWidth = (int)((float)m_iHealth / m_iMaxHealth * barWidth);
		for (int y = barY; y < barY + barHeight; y++)
		{
			for (int x = barX; x < barX + healthWidth; x++)
			{
				if (x >= 0 && x < getEngine()->getWindowWidth() &&
					y >= 0 && y < getEngine()->getWindowHeight())
				{
					pSurface->setPixel(x, y, 0x00FF00);
				}
			}
		}
	}
}
//Update display 
void EnemyObject::virtDoUpdate(int iCurrentTime)
{
	if (isDead())
	{
		setVisible(false);
		m_bDeathAnimComplete = true;
		return;
	}

	if (m_bReachedBase)
		return;

	updatePathFollowing();

	// Walk animation - 8 frames
	if (iCurrentTime - m_iLastAnimTime > m_iAnimDelay)
	{
		m_iCurrentFrame++;
		if (m_iCurrentFrame >= 8)
		{
			m_iCurrentFrame = 0; 
		}
		m_iLastAnimTime = iCurrentTime;
	}

	// Update screen position using int values
	setPosition(m_iWorldX, m_iWorldY);
	redrawDisplay();
}

//Update Path following
void EnemyObject::updatePathFollowing()
{
	if (m_iCurrentPathIndex >= m_completePath.size())
	{
		m_bReachedBase = true;
		return;
	}
	float enemyCenterX = m_fWorldX + getWidth() / 2.0f;
	float enemyCenterY = m_fWorldY + getHeight() / 2.0f;

	float dx = (float)m_iTargetX - enemyCenterX;
	float dy = (float)m_iTargetY - enemyCenterY;
	float distance = sqrt(dx * dx + dy * dy);

	if (distance < m_iSpeed + 2)
	{
		m_iCurrentPathIndex++;
		if (m_iCurrentPathIndex < m_completePath.size())
		{
			MainTileManager* tm = m_pMainEngine->getTileManager();
			tm->getTileCenterPosition(m_completePath[m_iCurrentPathIndex].first,
				m_completePath[m_iCurrentPathIndex].second,
				m_iTargetX, m_iTargetY);
		}
		else
		{
			m_bReachedBase = true;
		}
	}
	else
	{
		float vx = (dx / distance) * m_iSpeed;
		float vy = (dy / distance) * m_iSpeed;
		m_fWorldX += vx; 
		m_fWorldY += vy;
		m_iWorldX = (int)m_fWorldX;
		m_iWorldY = (int)m_fWorldY;
	}
}

//Enemy Take Damage Method
void EnemyObject::takeDamage(int damage)
{
	m_iHealth -= damage;
	if (m_iHealth < 0)
		m_iHealth = 0;
}

// Checks if a world coordinate intersects with a non-transparent pixel in the enemy sprite
bool EnemyObject::checkPixelCollision(int worldX, int worldY) const
{
	// First do bounding box check for optimization
	if (worldX < m_iWorldX || worldX >= m_iWorldX + m_iWidth ||
		worldY < m_iWorldY || worldY >= m_iWorldY + m_iHeight)
	{
		return false;
	}

	// If dead, don't collide
	if (isDead())
		return false;

	// Get the current sprite we're using
	const SimpleImage* currentSprite = &m_walkSprite;
	if (isDead())
		currentSprite = &m_deathSprite;

	if (!currentSprite->exists())
		return true; // Fallback to bounding box if no sprite

	// Constants matching the draw method
	const int SPRITE_WIDTH = 16;
	const int SPRITE_HEIGHT = 25;
	const float SCALE = 2.0f;
	int drawWidth = (int)(SPRITE_WIDTH * SCALE);
	int drawHeight = (int)(SPRITE_HEIGHT * SCALE);

	// Calculate enemy's center and draw position (must match virtDraw)
	int centerX = m_iWorldX + m_iWidth / 2;
	int centerY = m_iWorldY + m_iHeight / 2;
	int drawX = centerX - drawWidth / 2;
	int drawY = centerY - drawHeight / 2;

	// Convert world coordinate to sprite-local coordinate
	int localX = worldX - drawX;
	int localY = worldY - drawY;

	// Check if within sprite bounds
	if (localX < 0 || localX >= drawWidth ||
		localY < 0 || localY >= drawHeight)
	{
		return false;
	}

	// Map scaled coordinate back to source sprite coordinate
	int srcX = (int)(localX / SCALE);
	int srcY = (int)(localY / SCALE);

	// Add frame offset for animation
	int maxFrames = isDead() ? 7 : 8;
	srcX += m_iCurrentFrame * SPRITE_WIDTH;

	// Bounds check on source image
	if (srcX < 0 || srcX >= currentSprite->getWidth() ||
		srcY < 0 || srcY >= currentSprite->getHeight())
	{
		return false;
	}

	// Get the pixel color at this location
	int pixelColor = currentSprite->getPixelColour(srcX, srcY);

	// If pixel is transparent (black in our sprites), no collision
	if (pixelColor == 0x000000)
		return false;

	// Non-transparent pixel = collision!
	return true;
}