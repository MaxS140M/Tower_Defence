#pragma once

#include "GameState.h"
#include "ImageManager.h"

class WinState : public GameState
{
public:
    WinState(MainEngine* pEngine);

    void draw() override;
    void handleKeyDown(int iKeyCode) override;
    void handleKeyUp(int iKeyCode) override;
    void handleMouseDown(int iButton, int iX, int iY) override;
    void update(int iCurrentTime) override;

private:
    void drawBackground();
    SimpleImage m_background;
};