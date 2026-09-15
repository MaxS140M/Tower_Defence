#include "header.h"
#include "RunningState.h"
#include "MainEngine.h"
#include "PlayerObject.h"
#include "TowerObject.h"

RunningState::RunningState(MainEngine* pEngine)
    : GameState(pEngine)
    , m_bMovingUp(false)
    , m_bMovingDown(false)
    , m_bMovingLeft(false)
    , m_bMovingRight(false)
    , m_iLastFPSTime(0)
    , m_iFrameCount(0)
    , m_iCurrentFPS(0)
    , m_pSelectedTower(nullptr)
{
}

void RunningState::drawTextWithOutline(int x, int y, const char* text, unsigned int textColor, unsigned int outlineColor)
{
    m_pEngine->drawForegroundString(x - 1, y - 1, text, outlineColor, NULL);
    m_pEngine->drawForegroundString(x + 1, y - 1, text, outlineColor, NULL);
    m_pEngine->drawForegroundString(x - 1, y + 1, text, outlineColor, NULL);
    m_pEngine->drawForegroundString(x + 1, y + 1, text, outlineColor, NULL);
    m_pEngine->drawForegroundString(x - 1, y, text, outlineColor, NULL);
    m_pEngine->drawForegroundString(x + 1, y, text, outlineColor, NULL);
    m_pEngine->drawForegroundString(x, y - 1, text, outlineColor, NULL);
    m_pEngine->drawForegroundString(x, y + 1, text, outlineColor, NULL);
    m_pEngine->drawForegroundString(x, y, text, textColor, NULL);
}

void RunningState::draw()
{
    char buffer[256];

    // top- Money and Tower Cost 
    snprintf(buffer, sizeof(buffer), "Money: $%d   Tower Cost: $%d ",
        m_pEngine->getMoney(),
        m_pEngine->getTowerCost());
    m_pEngine->drawForegroundString(450, 20, buffer, 0x4800FF, NULL);

    // bottom- Wave, Enemies, and Towers info 
    snprintf(buffer, sizeof(buffer), "Wave: %d   Enemies: %d/%d   Towers: %d/%d",
        m_pEngine->getWave(),
        m_pEngine->getEnemyCount(),
        m_pEngine->getEnemiesPerWave(),
        m_pEngine->getTowerCount(),
        m_pEngine->getMaxTowers());
    m_pEngine->drawForegroundString(450, m_pEngine->getWindowHeight() - 30, buffer, 0xFF2800, NULL);

    // Display heart icons for lives
    static SimpleImage heartImage = ImageManager::loadImage("resources/heart.png", true);

    if (heartImage.exists())
    {
        const int HEART_SIZE = 32;
        const int HEART_SPACING = 48;
        const int START_X = 30;
        const int START_Y = 20;

        int lives = m_pEngine->getLives();

        for (int i = 0; i < lives && i < 3; i++)
        {
            int drawX = START_X + (i * HEART_SPACING);
            int drawY = START_Y;

            heartImage.renderImageWithMask(
                m_pEngine->getForegroundSurface(),
                0, 0,
                drawX, drawY,
                HEART_SIZE, HEART_SIZE
            );
        }
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "Lives: %d", m_pEngine->getLives());
        m_pEngine->drawForegroundString(10, 40, buffer, 0xFF0000, NULL);
    }

    // Display username at top right of screen
    snprintf(buffer, sizeof(buffer), "%s's Base", m_pEngine->getUsername().c_str());

    int textWidth = (int)strlen(buffer) * 20;
    int textX = m_pEngine->getWindowWidth() - textWidth - 10;
    int textY = 10;

    m_pEngine->drawForegroundString(textX, textY, buffer, 0x4800FF, NULL);
}

void RunningState::handleKeyDown(int iKeyCode)
{
    switch (iKeyCode)
    {
    case SDLK_p:
    case SDLK_ESCAPE:
        // Deselect tower if selected, otherwise pause
        if (m_pSelectedTower != nullptr)
        {
            m_pSelectedTower = nullptr;
        }
        else
        {
            m_pEngine->changeState(MainEngine::StateType::PAUSED);
        }
        break;
    case SDLK_w:
    case SDLK_UP:
        m_bMovingUp = true;
        break;
    case SDLK_s:
    case SDLK_DOWN:
        m_bMovingDown = true;
        break;
    case SDLK_a:
    case SDLK_LEFT:
        m_bMovingLeft = true;
        break;
    case SDLK_d:
    case SDLK_RIGHT:
        m_bMovingRight = true;
        break;
    case SDLK_u:
        // Upgrade selected tower
        if (m_pSelectedTower != nullptr && m_pSelectedTower->canUpgrade())
        {
            int upgradeCost = m_pSelectedTower->getUpgradeCost();
            if (m_pEngine->getMoney() >= upgradeCost)
            {
                m_pSelectedTower->upgrade();
                m_pEngine->subtractMoney(upgradeCost);
            }
        }
        break;
    }
}

void RunningState::handleKeyUp(int iKeyCode)
{
    switch (iKeyCode)
    {
    case SDLK_w:
    case SDLK_UP:
        m_bMovingUp = false;
        break;
    case SDLK_s:
    case SDLK_DOWN:
        m_bMovingDown = false;
        break;
    case SDLK_a:
    case SDLK_LEFT:
        m_bMovingLeft = false;
        break;
    case SDLK_d:
    case SDLK_RIGHT:
        m_bMovingRight = false;
        break;
    }
}

void RunningState::handleMouseDown(int iButton, int iX, int iY)
{
    // Handle building placement (left-click to place, right-click to remove)
    m_pEngine->handleBuildingPlacement(iButton, iX, iY);
}

void RunningState::update(int iCurrentTime)
{
    // Update FPS counter
    m_iFrameCount++;
    if (m_iLastFPSTime == 0)
    {
        m_iLastFPSTime = iCurrentTime;
    }

    int elapsed = iCurrentTime - m_iLastFPSTime;
    if (elapsed >= 1000)
    {
        m_iCurrentFPS = (m_iFrameCount * 1000) / elapsed;
        m_iFrameCount = 0;
        m_iLastFPSTime = iCurrentTime;
    }

    // Update player movement
    PlayerObject* player = m_pEngine->getPlayer();
    if (player != nullptr)
    {
        int vx = 0, vy = 0;
        int speed = 2;

        if (m_bMovingUp) vy -= speed;
        if (m_bMovingDown) vy += speed;
        if (m_bMovingLeft) vx -= speed;
        if (m_bMovingRight) vx += speed;

        if (vx != 0 && vy != 0)
        {
            float length = sqrt((float)(vx * vx + vy * vy));
            vx = (int)(vx / length * speed);
            vy = (int)(vy / length * speed);
        }

        player->setVelocity(vx, vy);
        player->setPosition(player->getWorldX(), player->getWorldY());
    }

    // Update game logic
    m_pEngine->updateGameLogic(iCurrentTime);
}

void RunningState::onEnter()
{
    m_pEngine->lockAndSetupBackground();
    m_pEngine->redrawDisplay();

    m_iLastFPSTime = 0;
    m_iFrameCount = 0;
    m_iCurrentFPS = 0;

    m_pSelectedTower = nullptr; // Clear selection when entering state
}