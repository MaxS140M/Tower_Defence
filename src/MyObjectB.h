#pragma once

#include "DisplayableObject.h"

class MyObjectB : public DisplayableObject
{
public:
	// Constructor - requires pointer to the engine
	MyObjectB(BaseEngine* pEngine);

	// Override virtual functions from DisplayableObject
	void virtDraw() override;
	void virtDoUpdate(int iCurrentTime) override;
};
