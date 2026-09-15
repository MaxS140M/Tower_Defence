#pragma once

#include "BaseEngine.h"
#include "MyTileManager.h"

class MyDemoA : public BaseEngine
{
public:
	// Virtual function overrides
	void virtSetupBackgroundBuffer() override;
	void virtMouseDown(int iButton, int iX, int iY) override;
	void virtKeyDown(int iKeyCode) override;

	// Initialize moving objects
	int virtInitialiseObjects() override;

protected:
	MyTileManager tm; // Tile manager instance
};