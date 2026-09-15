#include "header.h"
#include "GameOverState.h"
#include "MainEngine.h"

GameOverState::GameOverState(MainEngine* pEngine)
    : GameState(pEngine)
{
    m_background = ImageManager::loadImage("resources/LoseBackground.png", true);
}

void GameOverState::drawBackground()
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

void GameOverState::draw()
{
    drawBackground();

    int centerX = m_pEngine->getWindowWidth() / 2;
    int centerY = m_pEngine->getWindowHeight() / 2;

    m_pEngine->drawForegroundString(centerX - 70, centerY - 30, "GAME OVER", 0xFF0000, NULL);

    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Wave Reached: %d", m_pEngine->getWave());
    m_pEngine->drawForegroundString(centerX - 80, centerY, buffer, 0xFFFFFF, NULL);

    m_pEngine->drawForegroundString(centerX - 130, centerY + 30, "Press SPACE for Menu", 0xFFFFFF, NULL);
}

void GameOverState::handleKeyDown(int iKeyCode)
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

void GameOverState::handleKeyUp(int iKeyCode) {}
void GameOverState::handleMouseDown(int iButton, int iX, int iY) {}
void GameOverState::update(int iCurrentTime) {}