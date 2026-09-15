#include "TowerObject.h"
#include "MainEngine.h"
#include "EnemyObject.h"
#include "BulletObject.h"
#include <cmath>


//Tower Constrcuctor method subclass
TowerObject::TowerObject(MainEngine* pEngine, int worldX, int worldY)
	: GameObject(pEngine)
	, m_iRange(225)
	, m_iDamage(40)
	, m_iFireRate(1000)
	, m_iLastFireTime(0)
	, m_iLevel(1)
	, m_iBaseDamage(40)
	, m_iBaseRange(225)
{
	setWorldPosition(worldX, worldY);
	loadTowerSprite();  // Load the initial sprite based on level
	setSize(32, 50);
	setVisible(true);
	setPosition(worldX, worldY);
}
void TowerObject::loadTowerSprite()
{
	// Load the appropriate sprite based on current level (1-7)
	int spriteLevel = m_iLevel;
	if (spriteLevel > 7) spriteLevel = 7;

	std::string spritePath = "resources/Tower" + std::to_string(spriteLevel) + ".png";
	m_towerSprite = ImageManager::loadImage(spritePath, true);
}

//draw tower method
void TowerObject::virtDraw()
{
	if (!isVisible())
		return;

	DrawingSurface* pSurface = getEngine()->getForegroundSurface();

	int centerX = getXCentre();
	int centerY = getYCentre();

	if (m_towerSprite.exists())
	{
		const int SPRITE_WIDTH = 70;
		const int SPRITE_HEIGHT = 150;
		const float SCALE = 0.65f;
		int drawWidth = (int)(SPRITE_WIDTH * SCALE);
		int drawHeight = (int)(SPRITE_HEIGHT * SCALE);

		int drawX = centerX - drawWidth / 2 + 5;
		int drawY = centerY - drawHeight / 2 - 12;

		for (int py = 0; py < drawHeight; py++)
		{
			for (int px = 0; px < drawWidth; px++)
			{
				int srcX = (int)(px / SCALE);
				int srcY = (int)(py / SCALE);

				if (srcX >= SPRITE_WIDTH) srcX = SPRITE_WIDTH - 1;
				if (srcY >= SPRITE_HEIGHT) srcY = SPRITE_HEIGHT - 1;

				int pixelColor = m_towerSprite.getPixelColour(srcX, srcY);

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

		// Draw level indicator (small colored circles on tower)
		if (m_iLevel > 1)
		{
			unsigned int levelColors[] = { 0xFFD700, 0x00FF00, 0x00FFFF, 0xFF00FF };
			unsigned int color = levelColors[(m_iLevel - 2) % 4];

			int indicatorX = drawX + drawWidth - 8;
			int indicatorY = drawY + 5;

			for (int dy = -2; dy <= 2; dy++)
			{
				for (int dx = -2; dx <= 2; dx++)
				{
					if (dx * dx + dy * dy <= 4)
					{
						int px = indicatorX + dx;
						int py = indicatorY + dy;
						if (px >= 0 && px < getEngine()->getWindowWidth() &&
							py >= 0 && py < getEngine()->getWindowHeight())
						{
							pSurface->setPixel(px, py, color);
						}
					}
				}
			}
		}
	}
	else
	{
		int size = 10;
		for (int y = centerY - size; y <= centerY + size; y++)
		{
			for (int x = centerX - size; x <= centerX + size; x++)
			{
				if (x >= 0 && x < getEngine()->getWindowWidth() &&
					y >= 0 && y < getEngine()->getWindowHeight())
				{
					pSurface->setPixel(x, y, 0xFFD700);
				}
			}
		}
	}
}

//update tower method
void TowerObject::virtDoUpdate(int iCurrentTime)
{
	if (iCurrentTime - m_iLastFireTime < m_iFireRate)
		return;

	EnemyObject* target = findTargetEnemy();
	if (target != nullptr)
	{
		shootAtEnemy(target);
		m_iLastFireTime = iCurrentTime;
	}
}

//find enemy method
EnemyObject* TowerObject::findTargetEnemy()
{
	std::vector<EnemyObject*> enemies = m_pMainEngine->getEnemies();

	EnemyObject* closestEnemy = nullptr;
	float closestDistance = (float)(m_iRange * m_iRange);
	int towerCenterX = m_iWorldX + getWidth() / 2;
	int towerCenterY = m_iWorldY + getHeight() / 2;

	if (enemies.empty())
		return nullptr;
	//each enemy, find closest one to tower
	for (auto enemy : enemies)
	{
		if (enemy->isDead() || enemy->reachedBase())
			continue;

		int enemyCenterX = enemy->getWorldX() + enemy->getWidth() / 2;
		int enemyCenterY = enemy->getWorldY() + enemy->getHeight() / 2;

		int dx = enemyCenterX - towerCenterX;
		int dy = enemyCenterY - towerCenterY;

		if (abs(dx) > m_iRange || abs(dy) > m_iRange)
			continue;

		float distSq = (float)(dx * dx + dy * dy);

		if (distSq < closestDistance)
		{
			closestDistance = distSq;
			closestEnemy = enemy;
		}
	}

	return closestEnemy;
}

//shoot enemy method
void TowerObject::shootAtEnemy(EnemyObject* target)
{
	if (target != nullptr)
	{
		// shoot from center
		int towerCenterX = m_iWorldX + getWidth() / 2;
		int towerCenterY = m_iWorldY + getHeight() / 2;

		BulletObject* bullet = new BulletObject(m_pMainEngine, towerCenterX, towerCenterY, target, m_iDamage);
		m_pMainEngine->addBullet(bullet);
	}
}

//Increase Tower upgrade cost
int TowerObject::getUpgradeCost() const
{
	return (int)(50 * pow(1.6, m_iLevel - 1));
}

//sell tower value method
int TowerObject::getSellValue() const
{
	// Returns 50%
	int totalCost = m_pMainEngine->getTowerCost();
	for (int level = 1; level < m_iLevel; level++)
	{
		totalCost += (int)(15 * pow(1.6, level - 1));
	}
	return totalCost / 2;
}

//Method to set max upgrade level per wave 
int TowerObject::getMaxLevel() const
{
	int currentWave = m_pMainEngine->getWave();
	
	if (currentWave < 6)
	{
		return 5;
	} else {
		// Every 3 waves after wave 6, add 1 level
		int wavesAfterSix = currentWave - 6;
		int bonusLevels = (wavesAfterSix / 3) + 1;
		return 5 + bonusLevels;
	}
}

bool TowerObject::canUpgrade() const
{
	return m_iLevel < getMaxLevel();
}

//upgrade method
void TowerObject::upgrade()
{
	if (canUpgrade())
	{
		m_iLevel++;
		// Increase damage by 30% per level
		m_iDamage = (int)(m_iBaseDamage * pow(1.3, m_iLevel - 1));
		// Increase range by 5% per level
		m_iRange = (int)(m_iBaseRange * pow(1.05, m_iLevel - 1));
		// Decrease fire rate by 2.5% per level
		m_iFireRate = (int)(1000 / pow(1.025, m_iLevel - 1));

		loadTowerSprite();
	}
}

bool TowerObject::isPointInside(int worldX, int worldY) const
{
	int towerCenterX = m_iWorldX + 20;
	int towerCenterY = m_iWorldY + 20;

	return (worldX >= towerCenterX - 20 && worldX <= towerCenterX + 20 &&
		    worldY >= towerCenterY - 25 && worldY <= towerCenterY + 25);
}