#pragma once

#include "GameState.h"
#include "ImageManager.h"
#include <vector>

class MenuState : public GameState
{
public:
    MenuState(MainEngine* pEngine);
    
    void draw() override;
    void handleKeyDown(int iKeyCode) override;
    void handleKeyUp(int iKeyCode) override;
    void handleMouseDown(int iButton, int iX, int iY) override;
    void update(int iCurrentTime) override;
    void onEnter() override;
    void onExit() override;
    void handleMouseWheel(int wheelDelta);
private:
    enum class MenuMode
    {
        MAIN,           // Show main menu with Start/Load buttons and username box
        LOAD_GAME       // Show list of saved games
    };

    void drawBackground();
    void drawTitle();
    void drawGameDescription();
    void drawInstructions();
    void drawControls();
    void drawMainMenu();
    void drawUsernameBox();
    void drawLoadGameMenu();

    int transformX(int x) const;
    int transformY(int y) const;
    void updateScroll();
    
    SimpleImage m_background;
    bool m_bFirstEnter;
    MenuMode m_menuMode;
    std::string m_usernameInput;
    bool m_bEditingUsername;
    
    // Load game selection
    std::vector<std::string> m_savedGames;
    int m_selectedGameIndex;
    
    // Username box bounds for click detection
    int m_usernameBoxX;
    int m_usernameBoxY;
    int m_usernameBoxWidth;
    int m_usernameBoxHeight;

    // NEW: Scrolling and zooming
    float m_fScrollY;          // Vertical scroll offset
    float m_fScrollVelocity;   // Scroll momentum
    float m_fZoomLevel;        // Zoom factor (1.0 = normal, 2.0 = 2x zoom)
    bool m_bScrollingUp;       // Key-based scrolling
    bool m_bScrollingDown;

    // Content bounds
    static const int CONTENT_HEIGHT = 1200;  // Total virtual height o
};