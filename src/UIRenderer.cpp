#include "UIRenderer.h"
#include "MainEngine.h"

UIRenderer::UIRenderer(MainEngine* pEngine)
    : m_pEngine(pEngine)
{
    // Load background images
    m_menuBackground = ImageManager::loadImage("resources/menuBackground.png", true);
    m_winBackground = ImageManager::loadImage("resources/winbackground.png", true);
}

void UIRenderer::drawBackground(SimpleImage& background)
{
    if (background.exists())
    {
        DrawingSurface* pSurface = m_pEngine->getForegroundSurface();
        int imgWidth = background.getWidth();
        int imgHeight = background.getHeight();
        int screenWidth = m_pEngine->getWindowWidth();
        int screenHeight = m_pEngine->getWindowHeight();

        // Scale image to fit screen
        for (int y = 0; y < screenHeight; y++)
        {
            for (int x = 0; x < screenWidth; x++)
            {
                int srcX = x * imgWidth / screenWidth;
                int srcY = y * imgHeight / screenHeight;

                if (srcX >= imgWidth) srcX = imgWidth - 1;
                if (srcY >= imgHeight) srcY = imgHeight - 1;

                int pixelColor = background.getPixelColour(srcX, srcY);
                pSurface->setPixel(x, y, pixelColor);
            }
        }
    }
}

void UIRenderer::drawUI()
{
    switch (m_pEngine->getState())
    {
    case MainEngine::State::MENU:
        drawMenuState();
        break;
    case MainEngine::State::RUNNING:
        drawRunningState();
        break;
    case MainEngine::State::PAUSED:
        drawPausedState();
        break;
    case MainEngine::State::GAME_OVER:
        drawGameOverState();
        break;
    case MainEngine::State::WIN:
        drawWinState();
        break;
    }
}

void UIRenderer::drawMenuState()
{
    // Draw background image
    drawBackground(m_menuBackground);

    int centerX = m_pEngine->getWindowWidth() / 2;
    int centerY = m_pEngine->getWindowHeight() / 2;

    m_pEngine->drawForegroundString(centerX - 120, centerY - 50, "TOWER DEFENSE", 0xFFFFFF, NULL);
    m_pEngine->drawForegroundString(centerX - 120, centerY, "Press SPACE to Start", 0xFFFFFF, NULL);
    m_pEngine->drawForegroundString(centerX - 100, centerY + 30, "Press ESC to Exit", 0xFFFFFF, NULL);
}

// Helper function to draw text with outline
void UIRenderer::drawTextWithOutline(int x, int y, const char* text, unsigned int textColor, unsigned int outlineColor)
{
    // Draw outline (white) by drawing text offset in all directions
    m_pEngine->drawForegroundString(x - 1, y - 1, text, outlineColor, NULL);
    m_pEngine->drawForegroundString(x + 1, y - 1, text, outlineColor, NULL);
    m_pEngine->drawForegroundString(x - 1, y + 1, text, outlineColor, NULL);
    m_pEngine->drawForegroundString(x + 1, y + 1, text, outlineColor, NULL);
    m_pEngine->drawForegroundString(x - 1, y, text, outlineColor, NULL);
    m_pEngine->drawForegroundString(x + 1, y, text, outlineColor, NULL);
    m_pEngine->drawForegroundString(x, y - 1, text, outlineColor, NULL);
    m_pEngine->drawForegroundString(x, y + 1, text, outlineColor, NULL);

    // Draw main text (blue) on top
    m_pEngine->drawForegroundString(x, y, text, textColor, NULL);
}

void UIRenderer::drawRunningState()
{
    char buffer[128];

    // Position in top right corner
    int rightX = m_pEngine->getWindowWidth() - 260;

    // Blue text with white outline
    unsigned int blueColor = 0x0066FF;
    unsigned int whiteColor = 0xFFFFFF;

    snprintf(buffer, sizeof(buffer), "Money: $%d", m_pEngine->getMoney());
    drawTextWithOutline(rightX, 10, buffer, blueColor, whiteColor);

    snprintf(buffer, sizeof(buffer), "Lives: %d", m_pEngine->getLives());
    drawTextWithOutline(rightX, 30, buffer, blueColor, whiteColor);

    snprintf(buffer, sizeof(buffer), "Wave: %d", m_pEngine->getWave());
    drawTextWithOutline(rightX, 50, buffer, blueColor, whiteColor);

    snprintf(buffer, sizeof(buffer), "Enemies: %d/%d", m_pEngine->getEnemyCount(), m_pEngine->getEnemiesPerWave());
    drawTextWithOutline(rightX, 70, buffer, blueColor, whiteColor);

    snprintf(buffer, sizeof(buffer), "Tower Cost: $%d", m_pEngine->getTowerCost());
    drawTextWithOutline(rightX, 90, buffer, blueColor, whiteColor);

    m_pEngine->drawForegroundString(10, m_pEngine->getWindowHeight() - 60, "WASD/Arrows: Move", 0xCCCCCC, NULL);
    snprintf(buffer, sizeof(buffer), "Left Click: Place Tower ($%d)", m_pEngine->getTowerCost());
    m_pEngine->drawForegroundString(10, m_pEngine->getWindowHeight() - 40, buffer, 0xCCCCCC, NULL);
    m_pEngine->drawForegroundString(10, m_pEngine->getWindowHeight() - 20, "Right Click: Remove Tower", 0xCCCCCC, NULL);
}

void UIRenderer::drawPausedState()
{
    // Draw background image
    drawBackground(m_menuBackground);

    int centerX = m_pEngine->getWindowWidth() / 2;
    int centerY = m_pEngine->getWindowHeight() / 2;

    m_pEngine->drawForegroundString(centerX - 50, centerY - 20, "PAUSED", 0xFFFF00, NULL);
    m_pEngine->drawForegroundString(centerX - 120, centerY + 10, "Press P to Resume", 0xFFFFFF, NULL);
    m_pEngine->drawForegroundString(centerX - 120, centerY + 40, "Press ESC for Menu", 0xFFFFFF, NULL);
}

void UIRenderer::drawGameOverState()
{
    // Draw background image
    drawBackground(m_menuBackground);

    int centerX = m_pEngine->getWindowWidth() / 2;
    int centerY = m_pEngine->getWindowHeight() / 2;

    m_pEngine->drawForegroundString(centerX - 70, centerY - 30, "GAME OVER", 0xFF0000, NULL);

    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Wave Reached: %d", m_pEngine->getWave());
    m_pEngine->drawForegroundString(centerX - 80, centerY, buffer, 0xFFFFFF, NULL);

    m_pEngine->drawForegroundString(centerX - 130, centerY + 30, "Press SPACE for Menu", 0xFFFFFF, NULL);
}

void UIRenderer::drawWinState()
{
    // Draw win background image
    drawBackground(m_winBackground);

    int centerX = m_pEngine->getWindowWidth() / 2;
    int centerY = m_pEngine->getWindowHeight() / 2;

    m_pEngine->drawForegroundString(centerX - 70, centerY - 30, "YOU WIN!", 0x00FF00, NULL);

    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Waves Completed: %d", m_pEngine->getWave());
    m_pEngine->drawForegroundString(centerX - 100, centerY, buffer, 0xFFFFFF, NULL);

    snprintf(buffer, sizeof(buffer), "Money Remaining: $%d", m_pEngine->getMoney());
    m_pEngine->drawForegroundString(centerX - 100, centerY + 25, buffer, 0xFFD700, NULL);

    m_pEngine->drawForegroundString(centerX - 130, centerY + 60, "Press SPACE for Menu", 0xFFFFFF, NULL);
}