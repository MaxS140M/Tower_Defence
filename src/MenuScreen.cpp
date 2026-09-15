#include "header.h"
#include "MenuScreen.h"
#include "BaseEngine.h"

MenuScreen::MenuScreen(BaseEngine* pEngine)
    : m_pEngine(pEngine)
{
    m_menuBackground = ImageManager::loadImage("resources/menuBackground.png", true);
}

void MenuScreen::drawBackground()
{
    if (m_menuBackground.exists())
    {
        DrawingSurface* pSurface = m_pEngine->getForegroundSurface();
        int imgWidth = m_menuBackground.getWidth();
        int imgHeight = m_menuBackground.getHeight();
        int screenWidth = m_pEngine->getWindowWidth();
        int screenHeight = m_pEngine->getWindowHeight();

        for (int y = 0; y < screenHeight; y++)
        {
            for (int x = 0; x < screenWidth; x++)
            {
                int srcX = x * imgWidth / screenWidth;
                int srcY = y * imgHeight / screenHeight;
                if (srcX >= imgWidth) srcX = imgWidth - 1;
                if (srcY >= imgHeight) srcY = imgHeight - 1;

                int pixelColor = m_menuBackground.getPixelColour(srcX, srcY);
                pSurface->setPixel(x, y, pixelColor);
            }
        }
    }
    else
    {
        // Fallback gradient
        for (int y = 0; y < m_pEngine->getWindowHeight(); y++)
        {
            for (int x = 0; x < m_pEngine->getWindowWidth(); x++)
            {
                int distFromCenter = abs(x - m_pEngine->getWindowWidth() / 2) +
                    abs(y - m_pEngine->getWindowHeight() / 2);
                int brightness = 0x30 - (distFromCenter / 20);
                if (brightness < 0x10) brightness = 0x10;

                unsigned int color = (brightness << 16) | (brightness << 8) | brightness;
                m_pEngine->setForegroundPixel(x, y, color);
            }
        }
    }
}

void MenuScreen::draw()
{
    drawBackground();
    drawTitle();
    drawGameDescription();
    drawInstructions();
    drawControls();
    drawFooter();
}

void MenuScreen::drawTitle()
{
    int centerX = m_pEngine->getWindowWidth() / 2;

    // Main title - large and centered
    m_pEngine->drawForegroundString(centerX - 180, 80, "TOWER DEFENSE", 0xFFD700, NULL);

    // Subtitle
    m_pEngine->drawForegroundString(centerX - 100, 110, "Defend Your Base!", 0xFFFFFF, NULL);

    // Decorative line
    m_pEngine->drawForegroundLine(centerX - 200, 130, centerX + 200, 130, 0xFFD700);
}

void MenuScreen::drawGameDescription()
{
    int leftMargin = 150;
    int startY = 160;
    int lineHeight = 25;

    m_pEngine->drawForegroundString(leftMargin, startY, "ABOUT THE GAME:", 0x00FF00, NULL);

    m_pEngine->drawForegroundString(leftMargin, startY + lineHeight,
        "You are the last builder and you are prepared to defend your base", 0xCCCCCC, NULL);

    m_pEngine->drawForegroundString(leftMargin, startY + lineHeight * 2,
        "from monsters that have come to destroy it!", 0xCCCCCC, NULL);

    m_pEngine->drawForegroundString(leftMargin, startY + lineHeight * 3,
        "Each monster that reaches your base cost you a life", 0xCCCCCC, NULL);

    m_pEngine->drawForegroundString(leftMargin, startY + lineHeight * 4,
        "The waves of monsters dont stop so coming so be prepared", 0xCCCCCC, NULL);
}

void MenuScreen::drawInstructions()
{
    int leftMargin = 150;
    int startY = 310;
    int lineHeight = 25;

    m_pEngine->drawForegroundString(leftMargin, startY, "HOW TO PLAY:", 0x00FF00, NULL);

    m_pEngine->drawForegroundString(leftMargin, startY + lineHeight,
        "- Move the Builder with WASD or Arrow Keys", 0xCCCCCC, NULL);

    m_pEngine->drawForegroundString(leftMargin, startY + lineHeight * 2,
        "- You can only build towers within your build radius", 0xCCCCCC, NULL);

    m_pEngine->drawForegroundString(leftMargin, startY + lineHeight * 3,
        "- Left Click: Place tower (they cost money) ", 0xFFD700, NULL);

    m_pEngine->drawForegroundString(leftMargin, startY + lineHeight * 4,
        "- Right Click: Remove tower (half of building cost is refunded)", 0xFF8888, NULL);

    m_pEngine->drawForegroundString(leftMargin, startY + lineHeight * 5,
        "- Kill enemies to earn money for more towers", 0xCCCCCC, NULL);
}

void MenuScreen::drawControls()
{
    int centerX = m_pEngine->getWindowWidth() / 2;
    int startY = 500;
    int lineHeight = 25;

    m_pEngine->drawForegroundString(centerX - 100, startY, "GAME CONTROLS:", 0x00FF00, NULL);

    m_pEngine->drawForegroundString(centerX - 80, startY + lineHeight,
        "P or ESC - Pause Game", 0xCCCCCC, NULL);

    m_pEngine->drawForegroundString(centerX - 80, startY + lineHeight * 2,
        "ESC (in pause) - Return to Menu", 0xCCCCCC, NULL);
}

void MenuScreen::drawFooter()
{
    int centerX = m_pEngine->getWindowWidth() / 2;
    int bottomY = m_pEngine->getWindowHeight() - 100;

    // Decorative line
    m_pEngine->drawForegroundLine(centerX - 200, bottomY - 20, centerX + 200, bottomY - 20, 0xFFD700);

    // Start prompt - make it flash by checking time
    int time = m_pEngine->getRawTime();
    if ((time / 500) % 2 == 0)  // Flash every 500ms
    {
        m_pEngine->drawForegroundString(centerX - 140, bottomY,
            "PRESS SPACE TO START", 0x00FFFF, NULL);
    }

    m_pEngine->drawForegroundString(centerX - 80, bottomY + 30,
        "Good Luck", 0x888888, NULL);
}