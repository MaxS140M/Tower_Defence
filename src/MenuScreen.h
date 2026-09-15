#pragma once

#include "ImageManager.h"

class BaseEngine;

class MenuScreen
{
public:
    MenuScreen(BaseEngine* pEngine);

    void draw();

private:
    void drawBackground();
    void drawTitle();
    void drawGameDescription();
    void drawInstructions();
    void drawControls();
    void drawFooter();

    BaseEngine* m_pEngine;
    SimpleImage m_menuBackground;
};