#include "header.h"
#include "MenuState.h"
#include "MainEngine.h"
#include <algorithm>

MenuState::MenuState(MainEngine* pEngine)
    : GameState(pEngine)
    , m_bFirstEnter(true)
    , m_menuMode(MenuMode::MAIN)
    , m_selectedGameIndex(0)
    , m_bEditingUsername(false)
    , m_usernameBoxX(0)
    , m_usernameBoxY(0)
    , m_usernameBoxWidth(0)
    , m_usernameBoxHeight(0)
    , m_fScrollY(0.0f)
    , m_fScrollVelocity(0.0f)
    , m_fZoomLevel(1.0f)
    , m_bScrollingUp(false)
    , m_bScrollingDown(false)
{
    m_background = ImageManager::loadImage("resources/menuBackground.png", true);
}

void MenuState::onEnter()
{
    if (m_bFirstEnter)
    {
        std::string savedUsername = m_pEngine->getSaveManager()->loadUsername();
        if (!savedUsername.empty() && savedUsername != "Player")
        {
            m_usernameInput = savedUsername;
            m_pEngine->setUsername(savedUsername);
        }
    }
    
    if (!m_bFirstEnter)
    {
        m_pEngine->resetGame();
    }
    m_bFirstEnter = false;
    m_menuMode = MenuMode::MAIN;
    m_bEditingUsername = false;
    
    m_fScrollY = 0.0f;
    m_fScrollVelocity = 0.0f;
    m_fZoomLevel = 1.0f;
}

void MenuState::onExit()
{
    m_bScrollingUp = false;
    m_bScrollingDown = false;
}

int MenuState::transformX(int x) const
{
    int screenCenterX = m_pEngine->getWindowWidth() / 2;
    return (int)(screenCenterX + (x - screenCenterX) * m_fZoomLevel);
}

int MenuState::transformY(int y) const
{
    return (int)((y - m_fScrollY) * m_fZoomLevel);
}

void MenuState::updateScroll()
{
    const float SCROLL_SPEED = 8.0f;
    const float FRICTION = 0.92f;
    
    if (m_bScrollingUp)
    {
        m_fScrollVelocity -= SCROLL_SPEED;
    }
    if (m_bScrollingDown)
    {
        m_fScrollVelocity += SCROLL_SPEED;
    }
    
    m_fScrollY += m_fScrollVelocity;
    m_fScrollVelocity *= FRICTION;
    
    float maxScroll = CONTENT_HEIGHT - (m_pEngine->getWindowHeight() / m_fZoomLevel);
    if (maxScroll < 0) maxScroll = 0;
    
    if (m_fScrollY < 0)
    {
        m_fScrollY = 0;
        m_fScrollVelocity = 0;
    }
    if (m_fScrollY > maxScroll)
    {
        m_fScrollY = maxScroll;
        m_fScrollVelocity = 0;
    }
    
    if (abs(m_fScrollVelocity) < 0.1f)
    {
        m_fScrollVelocity = 0.0f;
    }
}

void MenuState::handleMouseWheel(int wheelDelta)
{
    const float ZOOM_STEP = 0.15f;
    const float MIN_ZOOM = 0.75f;
    const float MAX_ZOOM = 2.5f;

    float oldZoom = m_fZoomLevel;

    if (wheelDelta > 0)
    {
        m_fZoomLevel += ZOOM_STEP;
    }
    else if (wheelDelta < 0)
    {
        m_fZoomLevel -= ZOOM_STEP;
    }

    if (m_fZoomLevel < MIN_ZOOM) m_fZoomLevel = MIN_ZOOM;
    if (m_fZoomLevel > MAX_ZOOM) m_fZoomLevel = MAX_ZOOM;

    // Adjust scroll to keep the CENTER of the current view in the same place
    if (m_fZoomLevel != oldZoom && oldZoom > 0)
    {
        // Calculate the center point of the current view in content space
        float viewCenterY = m_fScrollY + (m_pEngine->getWindowHeight() / (2.0f * oldZoom));

        // After zoom, adjust scroll so this center point stays in the center
        m_fScrollY = viewCenterY - (m_pEngine->getWindowHeight() / (2.0f * m_fZoomLevel));

        // Clamp scroll
        float maxScroll = CONTENT_HEIGHT - (m_pEngine->getWindowHeight() / m_fZoomLevel);
        if (maxScroll < 0) maxScroll = 0;
        if (m_fScrollY < 0) m_fScrollY = 0;
        if (m_fScrollY > maxScroll) m_fScrollY = maxScroll;
    }

    m_pEngine->redrawDisplay();
}

void MenuState::drawBackground()
{
    DrawingSurface* pSurface = m_pEngine->getForegroundSurface();
    int screenWidth = m_pEngine->getWindowWidth();
    int screenHeight = m_pEngine->getWindowHeight();
    
    if (m_background.exists())
    {
        int imgWidth = m_background.getWidth();
        int imgHeight = m_background.getHeight();
        
        // Calculate virtual dimensions based on zoom
        int virtualWidth = (int)(screenWidth / m_fZoomLevel);
        int virtualHeight = (int)(screenHeight / m_fZoomLevel);
        int scrollOffset = (int)m_fScrollY;
        
        for (int y = 0; y < screenHeight; y++)
        {
            for (int x = 0; x < screenWidth; x++)
            {
                // Calculate source position with zoom and scroll
                int virtualX = (int)(x / m_fZoomLevel);
                int virtualY = (int)(y / m_fZoomLevel) + scrollOffset;
                
                int srcX = (virtualX * imgWidth) / (screenWidth / m_fZoomLevel);
                int srcY = ((virtualY * imgHeight) / (int)(screenHeight / m_fZoomLevel)) % imgHeight;
                
                if (srcX >= imgWidth) srcX = imgWidth - 1;
                if (srcY >= imgHeight) srcY = imgHeight - 1;
                if (srcX < 0) srcX = 0;
                if (srcY < 0) srcY = 0;

                int pixelColor = m_background.getPixelColour(srcX, srcY);
                pSurface->setPixel(x, y, pixelColor);
            }
        }
    }
    else
    {
        // Fallback gradient
        int scrollOffset = (int)m_fScrollY;
        for (int y = 0; y < screenHeight; y++)
        {
            for (int x = 0; x < screenWidth; x++)
            {
                int virtualY = (int)(y / m_fZoomLevel) + scrollOffset;
                int distFromCenter = abs(x - screenWidth / 2) + abs(virtualY - 400);
                int brightness = 0x30 - (distFromCenter / 20);
                if (brightness < 0x10) brightness = 0x10;

                unsigned int color = (brightness << 16) | (brightness << 8) | brightness;
                pSurface->setPixel(x, y, color);
            }
        }
    }
}

void MenuState::draw()
{
    drawBackground();
    
    if (m_menuMode == MenuMode::LOAD_GAME)
    {
        drawLoadGameMenu();
    }
    else
    {
        drawTitle();
        drawGameDescription();
        drawInstructions();
        drawControls();
        drawUsernameBox();
        drawMainMenu();
    }
    
    // Draw zoom and scroll indicators
    char buffer[64];
    sprintf(buffer, "Zoom: %.2fx (Mouse Wheel)", m_fZoomLevel);
    m_pEngine->drawForegroundString(10, 10, buffer, 0xFFFFFF, NULL);
   
    sprintf(buffer, "Scroll: %.0f (UP/DOWN Keys)", m_fScrollY);
    m_pEngine->drawForegroundString(10, 30, buffer, 0xFFFFFF, NULL);
}

void MenuState::drawTitle()
{
    int centerX = m_pEngine->getWindowWidth() / 2;
    
    // Calculate font size based on zoom
    int baseFontSize = 24;
    int zoomedFontSize = (int)(baseFontSize * m_fZoomLevel);
    if (zoomedFontSize < 10) zoomedFontSize = 10;
    if (zoomedFontSize > 72) zoomedFontSize = 72;
    
    int y = transformY(60);
    if (y < -100 || y > m_pEngine->getWindowHeight() + 100) return;

    // Scale text width approximation
    int titleWidth = (int)(100 * m_fZoomLevel);
    int x = transformX(centerX - 100);
    
    m_pEngine->drawForegroundString(x, y, "BuildATower", 0xFFD700, NULL);
    
    y = transformY(95);
    x = transformX(centerX - 100);
    m_pEngine->drawForegroundString(x, y, "Defend Your Home!", 0xFFFFFF, NULL);

    // Decorative lines
    int lineY1 = transformY(50);
    int lineY2 = transformY(125);
    int lineX1 = transformX(centerX - 220);
    int lineX2 = transformX(centerX + 220);
    
    if (lineY1 >= 0 && lineY1 < m_pEngine->getWindowHeight())
        m_pEngine->drawForegroundLine(lineX1, lineY1, lineX2, lineY1, 0xFFD700);
    if (lineY2 >= 0 && lineY2 < m_pEngine->getWindowHeight())
        m_pEngine->drawForegroundLine(lineX1, lineY2, lineX2, lineY2, 0xFFD700);
}

void MenuState::drawGameDescription()
{
    int leftMargin = 400;
    int startY = 155;
    int lineHeight = (int)(22 * m_fZoomLevel);

    int y = transformY(startY);
    if (y < -200 || y > m_pEngine->getWindowHeight() + 100) return;
    
    int x = transformX(leftMargin);
    m_pEngine->drawForegroundString(x + 175, y, "ABOUT", 0x00FF00, NULL);

    const char* lines[] = {
        "Defend your home",
        "Build Defence Towers",
        "Dont let the monsters destroy your base",
        "Monsters give gold",
        "Upgrade your Towers"
     
    };
    
    for (int i = 0; i < 5; i++)
    {
        y = transformY(startY + (int)(22 * (i + 1)));
        if (y >= -50 && y < m_pEngine->getWindowHeight() + 50)
        {
            x = transformX(leftMargin + 10);
            m_pEngine->drawForegroundString(x, y, lines[i], 0x101010, NULL);
        }
    }
}

void MenuState::drawInstructions()
{
    int leftMargin = 400;
    int startY = 295;

    int y = transformY(startY);
    if (y < -200 || y > m_pEngine->getWindowHeight() + 100) return;
    
    int x = transformX(leftMargin);
    m_pEngine->drawForegroundString(x + 175, y, "HOW TO PLAY:", 0x00FF00, NULL);

    const char* lines[] = {
        "Move Builder (WASD or Arrow Keys)",
        "You can only build towers",
        "within your build radius",
        "Left Click: Place tower",
        "Hover tower: See info",
        "Kill enemies to earn money",
        "Survive unti wave 20"
    };
    
    unsigned int colors[] = { 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010 };
    
    for (int i = 0; i < 6; i++)
    {
        y = transformY(startY + (int)(22 * (i + 1)));
        if (y >= -50 && y < m_pEngine->getWindowHeight() + 50)
        {
            x = transformX(leftMargin + 10);
            m_pEngine->drawForegroundString(x, y, lines[i], colors[i], NULL);
        }
    }
}

void MenuState::drawControls()
{
    int leftMargin = 400;
    int startY = 600;

    int y = transformY(startY);
    if (y < -100 || y > m_pEngine->getWindowHeight() + 100) return;
    
    int x = transformX(leftMargin);
    m_pEngine->drawForegroundString(x + 175, y, "CONTROLS:", 0x00FF00, NULL);

    const char* lines[] = {
        "WASD / Arrow Keys - Move Builder",
        "P / ESC - Pause Game",
        "U - Upgrade hovered tower",
        "S - Sell hovered tower"
    };
    
    for (int i = 0; i < 4; i++)
    {
        y = transformY(startY + (int)(22 * (i + 1)));
        if (y >= -50 && y < m_pEngine->getWindowHeight() + 50)
        {
            x = transformX(leftMargin + 10);
            m_pEngine->drawForegroundString(x, y, lines[i], 0x101010, NULL);
        }
    }
}

void MenuState::drawUsernameBox()
{
    int centerX = m_pEngine->getWindowWidth() / 2;
    int bottomY = 800;

    int y = transformY(bottomY - 68);
    if (y < -100 || y > m_pEngine->getWindowHeight() + 100) return;
    
    int x = transformX(centerX - 175);
    m_pEngine->drawForegroundString(x, y, "Player Name (Optional)", 0x101010, NULL);

    // Calculate transformed box bounds
    int baseX = centerX - 175;
    int baseY = bottomY + 10;
    int baseWidth = 350;
    int baseHeight = 60;
    
    m_usernameBoxX = transformX(baseX);
    m_usernameBoxY = transformY(baseY);
    int boxRight = transformX(baseX + baseWidth);
    int boxBottom = transformY(baseY + baseHeight);
    m_usernameBoxWidth = boxRight - m_usernameBoxX;
    m_usernameBoxHeight = boxBottom - m_usernameBoxY;

    // Ensure positive dimensions
    if (m_usernameBoxWidth < 0)
    {
        m_usernameBoxX = boxRight;
        m_usernameBoxWidth = -m_usernameBoxWidth;
    }
    if (m_usernameBoxHeight < 0)
    {
        m_usernameBoxY = boxBottom;
        m_usernameBoxHeight = -m_usernameBoxHeight;
    }

    unsigned int boxColor = m_bEditingUsername ? 0xFFD700 : 0x888888;
    m_pEngine->drawForegroundRectangle(m_usernameBoxX, m_usernameBoxY,
        m_usernameBoxX + m_usernameBoxWidth, m_usernameBoxY + m_usernameBoxHeight, boxColor);

    // Draw inner background
    for (int py = m_usernameBoxY + 2; py < m_usernameBoxY + m_usernameBoxHeight - 2 && py < m_pEngine->getWindowHeight(); py++)
    {
        if (py < 0) continue;
        for (int px = m_usernameBoxX + 2; px < m_usernameBoxX + m_usernameBoxWidth - 2 && px < m_pEngine->getWindowWidth(); px++)
        {
            if (px < 0) continue;
            m_pEngine->setForegroundPixel(px, py, 0x1A1A1A);
        }
    }

    // Draw username text
    std::string displayText = m_usernameInput.empty() ? "Click here to enter name" : m_usernameInput;
    unsigned int textColor = m_usernameInput.empty() ? 0x666666 : 0xFFFFFF;

    if (!displayText.empty())
    {
        m_pEngine->drawForegroundString(m_usernameBoxX + 12, m_usernameBoxY + 12,
            displayText.c_str(), textColor, NULL);
    }

    if (m_bEditingUsername)
    {
        int cursorX = m_usernameBoxX + 12;
        if (!m_usernameInput.empty())
        {
            cursorX += (int)(m_usernameInput.length() * 8 * m_fZoomLevel);
        }
        m_pEngine->drawForegroundLine(cursorX, m_usernameBoxY + 10,
            cursorX, m_usernameBoxY + m_usernameBoxHeight - 10, 0xFFFFFF);
    }
}

void MenuState::drawMainMenu()
{
    int centerX = m_pEngine->getWindowWidth() / 2;
    int bottomY = 950;

    int y = transformY(bottomY - 10);
    if (y >= 0 && y < m_pEngine->getWindowHeight())
    {
        int x1 = transformX(centerX - 220);
        int x2 = transformX(centerX + 220);
        m_pEngine->drawForegroundLine(x1, y, x2, y, 0xFFD700);
    }

    const char* options[] = {
        "Press 1 - Start NewGame",
        "Press 2 - Load Game",
        "Press ESC to Exit"
    };
    int yOffsets[] = {15, 42, 75};
    unsigned int colors[] = { 0x101010, 0x101010, 0x101010 };
    
    for (int i = 0; i < 3; i++)
    {
        y = transformY(bottomY + yOffsets[i]);
        if (y >= -50 && y < m_pEngine->getWindowHeight() + 50)
        {
            int x = transformX(centerX - 160);
            m_pEngine->drawForegroundString(x, y, options[i], colors[i], NULL);
        }
    }
}

void MenuState::drawLoadGameMenu()
{
    int centerX = m_pEngine->getWindowWidth() / 2;
    int startY = 150;
    int lineHeight = (int)(30 * m_fZoomLevel);

    m_pEngine->drawForegroundString(transformX(centerX - 100), transformY(80), "LOAD SAVED GAME", 0x101010, NULL);
    
    int lineY = transformY(110);
    int lineX1 = transformX(centerX - 200);
    int lineX2 = transformX(centerX + 200);
    m_pEngine->drawForegroundLine(lineX1, lineY, lineX2, lineY, 0xFFD700);

    if (m_savedGames.empty())
    {
        m_pEngine->drawForegroundString(transformX(centerX - 100), transformY(startY + 50), 
            "No saved games found", 0xFF8888, NULL);
        m_pEngine->drawForegroundString(transformX(centerX - 120), transformY(startY + 100), 
            "Press ESC to go back", 0xCCCCCC, NULL);
    }
    else
    {
        m_pEngine->drawForegroundString(transformX(centerX - 150), transformY(startY), 
            "Use UP/DOWN arrows to select", 0xCCCCCC, NULL);
        m_pEngine->drawForegroundString(transformX(centerX - 150), transformY(startY + 25), 
            "Press ENTER to load", 0xCCCCCC, NULL);
        m_pEngine->drawForegroundString(transformX(centerX - 150), transformY(startY + 50), 
            "Press DELETE to remove", 0xFF8888, NULL);

        for (size_t i = 0; i < m_savedGames.size() && i < 10; i++)
        {
            unsigned int color = (i == (size_t)m_selectedGameIndex) ? 0x00FF00 : 0xFFFFFF;
            std::string prefix = (i == (size_t)m_selectedGameIndex) ? "> " : "  ";
            
            int itemY = transformY(startY + 95 + (int)(i * 30));
            int itemX = transformX(centerX - 150);
            m_pEngine->drawForegroundString(itemX, itemY, 
                (prefix + m_savedGames[i]).c_str(), color, NULL);
        }

        m_pEngine->drawForegroundString(transformX(centerX - 100), transformY(m_pEngine->getWindowHeight() - 80), 
            "Press ESC to go back", 0x888888, NULL);
    }
}

void MenuState::handleKeyDown(int iKeyCode)
{
    if (m_menuMode == MenuMode::LOAD_GAME)
    {
        if (iKeyCode == SDLK_UP)
        {
            if (m_selectedGameIndex > 0)
            {
                m_selectedGameIndex--;
                m_pEngine->redrawDisplay();
            }
        }
        else if (iKeyCode == SDLK_DOWN)
        {
            if (m_selectedGameIndex < (int)m_savedGames.size() - 1)
            {
                m_selectedGameIndex++;
                m_pEngine->redrawDisplay();
            }
        }
        else if (iKeyCode == SDLK_RETURN && !m_savedGames.empty())
        {
            if (m_pEngine->getSaveManager()->loadGameState(m_savedGames[m_selectedGameIndex]))
            {
                m_pEngine->changeState(MainEngine::StateType::RUNNING);
            }
        }
        else if (iKeyCode == SDLK_DELETE && !m_savedGames.empty())
        {
            if (m_pEngine->getSaveManager()->deleteSavedGame(m_savedGames[m_selectedGameIndex]))
            {
                m_savedGames.erase(m_savedGames.begin() + m_selectedGameIndex);
                
                if (m_selectedGameIndex >= (int)m_savedGames.size() && m_selectedGameIndex > 0)
                {
                    m_selectedGameIndex--;
                }
                
                m_pEngine->redrawDisplay();
            }
        }
        else if (iKeyCode == SDLK_ESCAPE)
        {
            m_menuMode = MenuMode::MAIN;
            m_pEngine->redrawDisplay();
        }
    }
    else // MAIN menu
    {
        if (m_bEditingUsername)
        {
            if (iKeyCode == SDLK_UP)
            {
                m_bScrollingUp = true;
            }
            else if (iKeyCode == SDLK_DOWN)
            {
                m_bScrollingDown = true;
            }
            else if (iKeyCode == SDLK_RETURN || iKeyCode == SDLK_ESCAPE)
            {
                m_bEditingUsername = false;
                if (!m_usernameInput.empty())
                {
                    m_pEngine->setUsername(m_usernameInput);
                    m_pEngine->getSaveManager()->saveUsername(m_usernameInput);
                }
                m_pEngine->redrawDisplay();
            }
            else if (iKeyCode == SDLK_BACKSPACE && !m_usernameInput.empty())
            {
                m_usernameInput.pop_back();
                m_pEngine->redrawDisplay();
            }
            else if (iKeyCode >= SDLK_a && iKeyCode <= SDLK_z)
            {
                if (m_usernameInput.length() < 20)
                {
                    char c = (char)iKeyCode;
                    if (SDL_GetModState() & KMOD_SHIFT)
                        c = toupper(c);
                    m_usernameInput += c;
                    m_pEngine->redrawDisplay();
                }
            }
            else if (iKeyCode >= SDLK_0 && iKeyCode <= SDLK_9)
            {
                if (m_usernameInput.length() < 20)
                {
                    m_usernameInput += (char)iKeyCode;
                    m_pEngine->redrawDisplay();
                }
            }
            else if (iKeyCode == SDLK_SPACE && m_usernameInput.length() < 20)
            {
                m_usernameInput += ' ';
                m_pEngine->redrawDisplay();
            }
        }
        else
        {
            if (iKeyCode == SDLK_UP || iKeyCode == SDLK_w)
            {
                m_bScrollingUp = true;
            }
            else if (iKeyCode == SDLK_DOWN || iKeyCode == SDLK_s)
            {
                m_bScrollingDown = true;
            }
            else if (iKeyCode == SDLK_1)
            {
                m_pEngine->resetGame();
                m_pEngine->changeState(MainEngine::StateType::RUNNING);
            }
            else if (iKeyCode == SDLK_2)
            {
                m_savedGames = m_pEngine->getSaveManager()->listSavedGames();
                m_selectedGameIndex = 0;
                m_menuMode = MenuMode::LOAD_GAME;
                m_pEngine->redrawDisplay();
            }
            else if (iKeyCode == SDLK_ESCAPE)
            {
                exit(0);
            }
        }
    }
}

void MenuState::handleKeyUp(int iKeyCode)
{
    if (iKeyCode == SDLK_UP || iKeyCode == SDLK_w)
    {
        m_bScrollingUp = false;
    }
    else if (iKeyCode == SDLK_DOWN || iKeyCode == SDLK_s)
    {
        m_bScrollingDown = false;
    }
}

void MenuState::handleMouseDown(int iButton, int iX, int iY)
{
    if (m_menuMode == MenuMode::MAIN)
    {
        if (iX >= m_usernameBoxX && iX <= m_usernameBoxX + m_usernameBoxWidth &&
            iY >= m_usernameBoxY && iY <= m_usernameBoxY + m_usernameBoxHeight)
        {
            m_bEditingUsername = true;
            m_pEngine->redrawDisplay();
        }
        else
        {
            m_bEditingUsername = false;
            m_pEngine->redrawDisplay();
        }
    }
}

void MenuState::update(int iCurrentTime)
{
    updateScroll();
    
    if (m_fScrollVelocity != 0.0f || m_bScrollingUp || m_bScrollingDown)
    {
        m_pEngine->redrawDisplay();
    }
}