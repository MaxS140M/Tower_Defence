#pragma once

#include "GameObject.h"

class PlayerObject : public GameObject
{
public:
	PlayerObject(MainEngine* pEngine);
	virtual ~PlayerObject() {}

	void virtDraw() override;
	void virtDoUpdate(int iCurrentTime) override;

	void setVelocity(int vx, int vy);
	int getBuildRadius() const { return m_iBuildRadius; }

private:
	int m_iVelocityX;
	int m_iVelocityY;
	int m_iSpeed;
	int m_iBuildRadius;

	// Sprite animation members
	SimpleImage m_spriteSheet;
	int m_iCurrentFrame;
	int m_iLastAnimTime;
	int m_iAnimDelay;
	int m_iAnimRow;
	bool m_bFacingLeft;
	bool m_bIsMoving;
};