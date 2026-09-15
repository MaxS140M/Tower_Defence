#pragma once

#include "GameObject.h"
#include "ImageManager.h"

class MainEngine;
class EnemyObject;

class TowerObject : public GameObject
{
public:
	TowerObject(MainEngine* pEngine, int worldX, int worldY);
	virtual ~TowerObject() {}

	void virtDraw() override;
	void virtDoUpdate(int iCurrentTime) override;

	int getRange() const { return m_iRange; }
	int getDamage() const { return m_iDamage; }
	
	// Upgrade system
	int getLevel() const { return m_iLevel; }
	int getUpgradeCost() const;
	int getSellValue() const;
	int getMaxLevel() const;  // NEW: Dynamic max level based on wave
	bool canUpgrade() const;  // Updated to use dynamic max level
	void upgrade();
	
	// Hover detection for tooltip
	bool isPointInside(int worldX, int worldY) const;

private:
	EnemyObject* findTargetEnemy();
	void shootAtEnemy(EnemyObject* target);
	void loadTowerSprite();

	int m_iRange;
	int m_iDamage;
	int m_iFireRate;
	int m_iLastFireTime;
	int m_iUpdateOffset;

	// Upgrade system
	int m_iLevel;
	int m_iBaseDamage;
	int m_iBaseRange;

	SimpleImage m_towerSprite;
};