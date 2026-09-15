#pragma once

class MainEngine;

// Abstract base class for all game states (State Pattern)
class GameState
{
public:
    GameState(MainEngine* pEngine) : m_pEngine(pEngine) {}
    virtual ~GameState() {}

    // Pure virtual functions - each state must implement these
    virtual void draw() = 0;
    virtual void handleKeyDown(int iKeyCode) = 0;
    virtual void handleKeyUp(int iKeyCode) = 0;
    virtual void handleMouseDown(int iButton, int iX, int iY) = 0;
    virtual void update(int iCurrentTime) = 0;

    // Optional: called when entering/exiting a state
    virtual void onEnter() {}
    virtual void onExit() {}

protected:
    MainEngine* m_pEngine;
};