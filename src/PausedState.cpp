#include "header.h"
#include "PausedState.h"
#include "MainEngine.h"

PausedState::PausedState(MainEngine* pEngine)
    : GameState(pEngine)
{
    m_background = ImageManager::loadImage("resources/menuBackground.png", true);
}

void PausedState::drawBackground()
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

void PausedState::draw()
{
    drawBackground();

    int centerX = m_pEngine->getWindowWidth() / 2;
    int centerY = m_pEngine->getWindowHeight() / 2;

    m_pEngine->drawForegroundString(centerX - 50, centerY - 60, "PAUSED", 0xFFFF00, NULL);
    m_pEngine->drawForegroundString(centerX - 120, centerY - 20, "Press P to Resume", 0xFFFFFF, NULL);
    m_pEngine->drawForegroundString(centerX - 120, centerY + 10, "Press S to Save Game", 0x00FF00, NULL);
    m_pEngine->drawForegroundString(centerX - 120, centerY + 40, "Press ESC for Menu", 0xFFFFFF, NULL);
}

void PausedState::handleKeyDown(int iKeyCode)
{
    switch (iKeyCode)
    {
    case SDLK_p:
    case SDLK_SPACE:
        m_pEngine->changeState(MainEngine::StateType::RUNNING);
        break;
    case SDLK_s:
        // Save game with auto-generated unique filename
        {
            std::string savedFilename = m_pEngine->getSaveManager()->autoSaveGameState();
        }
        break;
    case SDLK_ESCAPE:
        m_pEngine->changeState(MainEngine::StateType::MENU);
        break;
    }
}

void PausedState::handleKeyUp(int iKeyCode) {}
void PausedState::handleMouseDown(int iButton, int iX, int iY) {}
void PausedState::update(int iCurrentTime) {}
