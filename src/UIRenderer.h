#pragma once

#include "ImageManager.h"

class MainEngine;

class UIRenderer
{
public:
    UIRenderer(MainEngine* pEngine);

    void drawUI();

private:
    void drawMenuState();
    void drawRunningState();
    void drawPausedState();
    void drawGameOverState();
    void drawWinState();
    void drawTextWithOutline(int x, int y, const char* text, unsigned int textColor, unsigned int outlineColor);
    void drawBackground(SimpleImage& background);

    MainEngine* m_pEngine;
    SimpleImage m_menuBackground;
    SimpleImage m_winBackground;
};