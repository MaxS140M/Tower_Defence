#pragma once

class MainEngine;

class InputHandler
{
public:
    InputHandler(MainEngine* pEngine);

    void handleKeyDown(int iKeyCode);
    void handleKeyUp(int iKeyCode);
    void handleMouseDown(int iButton, int iX, int iY);

    // Movement state accessors
    bool isMovingUp() const { return m_bMovingUp; }
    bool isMovingDown() const { return m_bMovingDown; }
    bool isMovingLeft() const { return m_bMovingLeft; }
    bool isMovingRight() const { return m_bMovingRight; }

private:
    void handleMenuInput(int iKeyCode);
    void handleRunningInput(int iKeyCode);
    void handlePausedInput(int iKeyCode);
    void handleWinInput(int iKeyCode);
    void handleGameOverInput(int iKeyCode);

    MainEngine* m_pEngine;
    bool m_bMovingUp;
    bool m_bMovingDown;
    bool m_bMovingLeft;
    bool m_bMovingRight;
};