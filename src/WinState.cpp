#include "header.h"
#include "WinState.h"
#include "MainEngine.h"

WinState::WinState(MainEngine* pEngine)
    : GameState(pEngine)
{
    m_background = ImageManager::loadImage("resources/WinBackground.png", true);
}

void WinState::drawBackground()
{
    if (m_background.exists())
    {
        DrawingSurface* pSurface = m_pEngine->getForegroundSurface();
        int imgWidth = m_background.getWidth();
        int imgHeight = m_background.getHeight();
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

                int pixelColor = m_background.getPixelColour(srcX, srcY);
                pSurface->setPixel(x, y, pixelColor);
            }
        }
    }
}

void WinState::draw()
{
    drawBackground();

    int centerX = m_pEngine->getWindowWidth() / 2;
    int centerY = m_pEngine->getWindowHeight() / 2;

    m_pEngine->drawForegroundString(centerX - 70, centerY - 30, "YOU WIN!", 0xFFFFFF, NULL);

    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Waves Completed: %d", m_pEngine->getWave());
    m_pEngine->drawForegroundString(centerX - 100, centerY, buffer, 0xFFFFFF, NULL);

    snprintf(buffer, sizeof(buffer), "You defeated all monster waves");
    m_pEngine->drawForegroundString(centerX - 100, centerY + 25, buffer, 0xFFD700, NULL);

    m_pEngine->drawForegroundString(centerX - 130, centerY + 60, "Press SPACE for Menu", 0xFFFFFF, NULL);
}

void WinState::handleKeyDown(int iKeyCode)
{
    switch (iKeyCode)
    {
    case SDLK_SPACE:
    case SDLK_RETURN:
        m_pEngine->changeState(MainEngine::StateType::MENU);
        break;
    case SDLK_ESCAPE:
        m_pEngine->setExitWithCode(0);
        break;
    }
}

void WinState::handleKeyUp(int iKeyCode) {}
void WinState::handleMouseDown(int iButton, int iX, int iY) {}
void WinState::update(int iCurrentTime) {}