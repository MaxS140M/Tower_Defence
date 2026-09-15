#ifndef PSYMSENGINE_H
#define PSYMSENGINE_H

#include "BaseEngine.h"

// Forward declaration
class PsymsTileManager;

// Custom engine class with unique background design
class PsymsEngine : public BaseEngine
{
public:
	/**
	Constructor
	*/
	PsymsEngine()
		: m_iFrameCounter(0)
		, m_iLastSecond(0)
		, m_iFPS(0)
		, m_iKeyPressCounter(0)
		, m_iCollisionCounter(0)
		, m_pTileManager(nullptr)  // Initialize tile manager pointer
	{
	}

	/**
	Destructor - clean up tile manager
	*/
	virtual ~PsymsEngine();

	// Setup the background buffer with a unique design
	void virtSetupBackgroundBuffer() override;

	// Create any moving objects
	int virtInitialiseObjects() override;

	// Draw strings on top - AFTER objects are drawn
	void virtDrawStringsOnTop() override;

	// Handle key presses
	void virtKeyDown(int iKeyCode) override;

	// Handle key releases
	void virtKeyUp(int iKeyCode) override;

	// Handle mouse clicks
	void virtMouseDown(int iButton, int iX, int iY) override;

	// Called before objects update - used for frame counting
	void virtMainLoopDoBeforeUpdate() override;

	// Called after objects update - check for collisions
	void virtMainLoopDoAfterUpdate() override;

	// Get the collision counter (for objects to access)
	int getCollisionCounter() const { return m_iCollisionCounter; }
	void incrementCollisionCounter() { m_iCollisionCounter++; }

	// Get the tile manager (for objects to interact with tiles)
	PsymsTileManager* getTileManager() { return m_pTileManager; }

private:
	// Member variables for changing text
	int m_iFrameCounter;       // Counts frames for FPS calculation
	int m_iLastSecond;         // Tracks the last second for FPS updates
	int m_iFPS;                // Current frames per second
	int m_iKeyPressCounter;    // Counter incremented by 'C' key, reset by 'R' key
	int m_iCollisionCounter;   // Counts collisions between player and automated objects

	// Tile manager for displaying background tiles
	PsymsTileManager* m_pTileManager;

	// Helper function to check collisions
	void checkCollisions();
};

#endif