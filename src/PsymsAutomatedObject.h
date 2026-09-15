#ifndef PSYMSAUTOMATEDOBJECT_H
#define PSYMSAUTOMATEDOBJECT_H

#include "DisplayableObject.h"
#include "BaseEngine.h"

/*
Automated object that moves in a sine wave pattern and bounces off walls.
Different from demos by combining horizontal movement with vertical sine wave motion.
Different from PsymsPlayerObject by being autonomous and having circular appearance.
*/
class PsymsAutomatedObject : public DisplayableObject
{
public:
	// Constructor
	PsymsAutomatedObject(BaseEngine* pEngine, int iStartX, int iStartY, double fSpeedX, double fAmplitude, double fFrequency);

	// Destructor
	virtual ~PsymsAutomatedObject() {}

	// Draw the object
	void virtDraw() override;

	// Update the object's position based on time
	void virtDoUpdate(int iCurrentTime) override;

	// Get center position and radius for circle collision detection
	int getCenterX() const { return m_iCurrentScreenX + OBJECT_RADIUS; }
	int getCenterY() const { return m_iCurrentScreenY + OBJECT_RADIUS; }
	int getRadius() const { return OBJECT_RADIUS; }

	// Handle collision
	void handleCollision();

	// Check tile interaction - call this only for objects that should interact with tiles
	void checkTileInteraction(int iCurrentTime);

private:
	// Object size
	static const int OBJECT_RADIUS = 25;

	// Movement parameters
	double m_fX;           // Precise X position 
	double m_fY;           // Precise Y position 
	double m_fSpeedX;      // Horizontal speed (pixels per update)
	double m_fAmplitude;   // Amplitude of sine wave (vertical range)
	double m_fFrequency;   // Frequency of sine wave
	double m_fPhase;       // Current phase in the sine wave

	// Base Y position (center line of sine wave)
	double m_fBaseY;

	// Color that pulses over time
	int m_iColorPhase;

	// Track last update time for smooth animation
	int m_iLastUpdateTime;

	// Collision flash effect
	int m_iCollisionFlashTimer;

	// Track which tile we're currently over to avoid excessive updates
	int m_iLastTileX;
	int m_iLastTileY;

	// Keep track of the engine to access drawing functions
	BaseEngine* m_pEngine;
};

#endif