#include "header.h"
#include "MainEngine.h"
#include "MainTileManager.h"
#include "PlayerObject.h"
#include "EnemyObject.h"
#include "TowerObject.h"
#include "BulletObject.h"
#include "WaveManager.h"
#include "GameState.h"
#include "MenuState.h"
#include "RunningState.h"
#include "PausedState.h"
#include "GameOverState.h"
#include "WinState.h"
#include <cmath>
#include <queue>

// Constructor - initializes default values for game state and resources
MainEngine::MainEngine()
    : m_currentStateType(StateType::MENU)
    , m_pCurrentState(nullptr)
    , m_pTileManager(nullptr)
    , m_pPlayer(nullptr)
    , m_pWaveManager(nullptr)
    , m_pSaveManager(nullptr)
    , m_iMoney(20)
    , m_iLives(3)
    , m_iBaseTowerCost(10)
    , m_iBaseTowerLimit(5)
    , m_iMouseWorldX(0)
    , m_iMouseWorldY(0)
    , m_pHoveredTower(nullptr)
    , m_username("Player")
{
}

// Destructor - cleans up all dynamically allocated resources
MainEngine::~MainEngine()
{
    for (auto& pair : m_states)
    {
        delete pair.second;
    }
    m_states.clear();

    if (m_pSaveManager != nullptr)
    {
        delete m_pSaveManager;
        m_pSaveManager = nullptr;
    }

    for (auto enemy : m_enemies)
    {
        removeDisplayableObject(enemy);
        delete enemy;
    }
    m_enemies.clear();

    for (auto tower : m_towers)
    {
        removeDisplayableObject(tower);
        delete tower;
    }
    m_towers.clear();

    for (auto bullet : m_bullets)
    {
        removeDisplayableObject(bullet);
        delete bullet;
    }
    m_bullets.clear();

    if (m_pTileManager != nullptr)
    {
        delete m_pTileManager;
        m_pTileManager = nullptr;
    }

    if (m_pWaveManager != nullptr)
    {
        delete m_pWaveManager;
        m_pWaveManager = nullptr;
    }

    if (m_pPlayer != nullptr)
    {
        removeDisplayableObject(m_pPlayer);
        delete m_pPlayer;
        m_pPlayer = nullptr;
    }
}

// Creates and initializes all game states (menu, running, paused, etc.)
void MainEngine::initializeStates()
{
    m_states[StateType::MENU] = new MenuState(this);
    m_states[StateType::RUNNING] = new RunningState(this);
    m_states[StateType::PAUSED] = new PausedState(this);
    m_states[StateType::GAME_OVER] = new GameOverState(this);
    m_states[StateType::WIN] = new WinState(this);

    m_pCurrentState = m_states[StateType::MENU];
    m_currentStateType = StateType::MENU;
}

// Transitions from current state to a new state, calling exit/enter handlers
void MainEngine::changeState(StateType newState)
{
    if (m_pCurrentState != nullptr)
    {
        m_pCurrentState->onExit();
    }

    m_currentStateType = newState;
    m_pCurrentState = m_states[newState];

    if (m_pCurrentState != nullptr)
    {
        m_pCurrentState->onEnter();
    }

    redrawDisplay();
}

// Sets up the static background layer with base color and tile rendering
void MainEngine::virtSetupBackgroundBuffer()
{
    for (int iY = 0; iY < getWindowHeight(); iY++)
    {
        for (int iX = 0; iX < getWindowWidth(); iX++)
        {
            setBackgroundPixel(iX, iY, 0x222222);
        }
    }

    if (m_pTileManager != nullptr)
    {
        m_pTileManager->drawAllTiles(this, getBackgroundSurface());
    }
}

// Initializes game objects including states, managers, player, and path finding
int MainEngine::virtInitialiseObjects()
{
    initializeStates();

    m_pSaveManager = new SaveGameManager(this);
    m_username = m_pSaveManager->loadUsername();

    m_pWaveManager = new WaveManager(this);

    m_pTileManager = new MainTileManager(50, 50, 26, 16);

    buildEnemyPath();
    buildPathRoutes();

    m_pPlayer = new PlayerObject(this);
    m_pPlayer->setWorldPosition(800, 300);

    drawableObjectsChanged();
    appendObjectToArray(m_pPlayer);

    return 0;
}

// Draws UI elements and tooltips on top of game objects
void MainEngine::virtDrawStringsOnTop()
{
    if (m_pCurrentState != nullptr)
    {
        m_pCurrentState->draw();
    }
    
    // Draw tower tooltip if hovering
    if (m_pHoveredTower != nullptr && m_currentStateType == StateType::RUNNING)
    {
        drawTowerTooltip(m_pHoveredTower);
    }
}

// Handles keyboard key press events, including tower upgrade with 'U' key
void MainEngine::virtKeyDown(int iKeyCode)
{
    if (m_pCurrentState != nullptr)
    {
        m_pCurrentState->handleKeyDown(iKeyCode);
    }

    // Handle tower upgrade with U key
    if (iKeyCode == SDLK_u && m_currentStateType == StateType::RUNNING)
    {
        if (m_pHoveredTower != nullptr && m_pHoveredTower->canUpgrade())
        {
            int upgradeCost = m_pHoveredTower->getUpgradeCost();
            if (m_iMoney >= upgradeCost)
            {
                m_pHoveredTower->upgrade();
                subtractMoney(upgradeCost);
                redrawDisplay();
            }
        }
    }
}

// Handles keyboard key release events
void MainEngine::virtKeyUp(int iKeyCode)
{
    if (m_pCurrentState != nullptr)
    {
        m_pCurrentState->handleKeyUp(iKeyCode);
    }
}

// Handles mouse button press events
void MainEngine::virtMouseDown(int iButton, int iX, int iY)
{
    if (m_pCurrentState != nullptr)
    {
        m_pCurrentState->handleMouseDown(iButton, iX, iY);
    }
}

// Updates mouse world coordinates and determines which tower is being hovered
void MainEngine::virtMouseMoved(int iX, int iY)
{
    m_iMouseWorldX = screenToWorldX(iX);
    m_iMouseWorldY = screenToWorldY(iY);
    
    // Update hovered tower
    m_pHoveredTower = getTowerAtPosition(m_iMouseWorldX, m_iMouseWorldY);
}

// Called before each update cycle to process current state logic
void MainEngine::virtMainLoopDoBeforeUpdate()
{
    if (m_pCurrentState != nullptr)
    {
        m_pCurrentState->update(getModifiedTime());
    }

    redrawDisplay();
}

// Updates all game logic including enemies, towers, waves, and animations
void MainEngine::updateGameLogic(int iCurrentTime)
{
    updateEnemies();
    updateTowers();
    cleanupDeadEnemies();
    cleanupBullets();

    if (m_pWaveManager != nullptr)
    {
        m_pWaveManager->update(iCurrentTime);
    }

    if (m_pTileManager != nullptr)
    {
        m_pTileManager->updateTileAnimations(this, iCurrentTime);
    }
}

// Decrements player lives and triggers game over if lives reach zero
void MainEngine::loseLife()
{
    m_iLives--;
    if (m_iLives <= 0)
    {
        changeState(StateType::GAME_OVER);
    }
}

// Handles tower placement and removal based on mouse clicks
void MainEngine::handleBuildingPlacement(int iButton, int iX, int iY)
{
    if (m_pTileManager == nullptr || m_pPlayer == nullptr)
        return;

    int worldX = screenToWorldX(iX);
    int worldY = screenToWorldY(iY);

    if (!m_pTileManager->isValidTilePosition(worldX, worldY))
        return;

    int tileX = m_pTileManager->getMapXForScreenX(worldX);
    int tileY = m_pTileManager->getMapYForScreenY(worldY);

    if (tileX < 0 || tileX >= m_pTileManager->getMapWidth() ||
        tileY < 0 || tileY >= m_pTileManager->getMapHeight())
        return;

    int tileCenterWorldX, tileCenterWorldY;
    m_pTileManager->getTileCenterPosition(tileX, tileY, tileCenterWorldX, tileCenterWorldY);

    if (!canBuildAtWorldPosition(tileCenterWorldX, tileCenterWorldY))
        return;

    if (iButton == SDL_BUTTON_LEFT)
    {
        int towerCost = getTowerCost();

        // Check tower limit - use getMaxTowers() instead of m_iBaseTowerLimit
        if (m_pTileManager->canPlaceTower(tileX, tileY) &&
            m_iMoney >= towerCost &&
            getTowerCount() < getMaxTowers())
        {
            placeTowerAtPosition(tileX, tileY);
            subtractMoney(towerCost);
            lockAndSetupBackground();
            redrawDisplay();
        }
    }
    else if (iButton == SDL_BUTTON_RIGHT)
    {
        if (m_pTileManager->getMapValue(tileX, tileY) == MainTileManager::TILE_TOWER)
        {
            int worldCenterX, worldCenterY;
            m_pTileManager->getTileCenterPosition(tileX, tileY, worldCenterX, worldCenterY);

            for (size_t i = 0; i < m_towers.size(); i++)
            {
                int towerCenterX = m_towers[i]->getWorldX() + 20;
                int towerCenterY = m_towers[i]->getWorldY() + 20;

                if (abs(towerCenterX - worldCenterX) < 5 && abs(towerCenterY - worldCenterY) < 5)
                {
                    // Use sell value instead of half tower cost
                    int sellValue = m_towers[i]->getSellValue();

                    // Clear hovered tower if it's the one being deleted
                    if (m_pHoveredTower == m_towers[i])
                    {
                        m_pHoveredTower = nullptr;
                    }

                    removeDisplayableObject(m_towers[i]);
                    delete m_towers[i];
                    m_towers.erase(m_towers.begin() + i);
                    drawableObjectsChanged();

                    addMoney(sellValue);
                    break;
                }
            }

            m_pTileManager->removeTower(tileX, tileY, this);
            lockAndSetupBackground();
            redrawDisplay();
        }
    }
}

// Returns the current wave number from the wave manager
int MainEngine::getWave() const
{
    return m_pWaveManager->getWave();
}

// Returns the number of enemies per wave
int MainEngine::getEnemiesPerWave() const
{
    return m_pWaveManager->getEnemiesPerWave();
}

// Adds an enemy to the game world and display list
void MainEngine::addEnemy(EnemyObject* enemy)
{
    m_enemies.push_back(enemy);
    appendObjectToArray(enemy);
    drawableObjectsChanged();
}

// Adds a bullet to the game world and display list
void MainEngine::addBullet(BulletObject* bullet)
{
    m_bullets.push_back(bullet);
    appendObjectToArray(bullet);
    drawableObjectsChanged();
}

// Calculates tower cost based on current wave using exponential scaling
int MainEngine::getTowerCost() const
{
    int wave = getWave();
    double cost = m_iBaseTowerCost * pow(1.5, wave - 1);
    return (int)cost;
}

// Checks if a tower can be built at the given world position based on player range
bool MainEngine::canBuildAtWorldPosition(int worldX, int worldY)
{
    if (m_pPlayer == nullptr)
        return false;

    int playerCenterX = m_pPlayer->getWorldX() + m_pPlayer->getWidth() / 2;
    int playerCenterY = m_pPlayer->getWorldY() + m_pPlayer->getHeight() / 2;

    int dx = worldX - playerCenterX;
    int dy = worldY - playerCenterY;
    int distanceSquared = dx * dx + dy * dy;
    int radiusSquared = m_pPlayer->getBuildRadius() * m_pPlayer->getBuildRadius();

    return distanceSquared <= radiusSquared;
}

// Sets the current wave number in the wave manager
void MainEngine::setWave(int wave)
{
    if (m_pWaveManager)
        m_pWaveManager->setWave(wave);
}

// Removes and deletes all towers from the game
void MainEngine::clearTowers()
{
    for (auto tower : m_towers)
    {
        removeDisplayableObject(tower);
        delete tower;
    }
    m_towers.clear();
    drawableObjectsChanged();
}

// Loads a tower at the specified coordinates (used for save game loading)
void MainEngine::loadTower(int x, int y)
{
    TowerObject* tower = new TowerObject(this, x, y);
    m_towers.push_back(tower);
    appendObjectToArray(tower);
    drawableObjectsChanged();
}

// Loads map data and rebuilds paths and background
void MainEngine::loadMapData(const std::vector<std::vector<int>>& mapData)
{
    if (m_pTileManager)
    {
        m_pTileManager->loadMap(mapData);
        buildEnemyPath();
        buildPathRoutes();
        lockAndSetupBackground();
        redrawDisplay();
    }
}

// Builds the main enemy path using BFS from start to end position
void MainEngine::buildEnemyPath()
{
    m_enemyPath.clear();

    int startX, startY, endX, endY;
    if (!m_pTileManager->getStartPosition(startX, startY) ||
        !m_pTileManager->getEndPosition(endX, endY))
        return;

    int w = m_pTileManager->getMapWidth();
    int h = m_pTileManager->getMapHeight();

    std::vector<int> parent(w * h, -1);
    std::vector<char> visited(w * h, 0);
    std::queue<std::pair<int, int>> pathQueue;

    auto idx = [&](int x, int y) { return y * w + x; };

    visited[idx(startX, startY)] = 1;
    pathQueue.push(std::make_pair(startX, startY));

    const int dx[4] = { 1, -1, 0, 0 };
    const int dy[4] = { 0, 0, 1, -1 };

    bool found = false;

    while (!pathQueue.empty())
    {
        std::pair<int, int> cur = pathQueue.front();
        pathQueue.pop();

        if (cur.first == endX && cur.second == endY)
        {
            found = true;
            break;
        }

        for (int i = 0; i < 4; ++i)
        {
            int nx = cur.first + dx[i];
            int ny = cur.second + dy[i];

            if (nx >= 0 && nx < w && ny >= 0 && ny < h && !visited[idx(nx, ny)])
            {
                int tileType = m_pTileManager->getMapValue(nx, ny);
                if (tileType == MainTileManager::TILE_PATH ||
                    tileType == MainTileManager::TILE_START ||
                    tileType == MainTileManager::TILE_END ||
                    tileType == MainTileManager::TILE_JUNCTION)
                {
                    visited[idx(nx, ny)] = 1;
                    parent[idx(nx, ny)] = idx(cur.first, cur.second);
                    pathQueue.push(std::make_pair(nx, ny));
                }
            }
        }
    }

    if (found)
    {
        int cur = idx(endX, endY);
        std::vector<std::pair<int, int>> reversePath;

        while (cur != -1)
        {
            reversePath.push_back(std::make_pair(cur % w, cur / w));
            cur = parent[cur];
        }

        for (int i = (int)reversePath.size() - 1; i >= 0; --i)
        {
            m_enemyPath.push_back(reversePath[i]);
        }
    }
}

// Uses BFS to find a path between two points on the tile map
void MainEngine::BFSPath(int startX, int startY, int endX, int endY, std::vector<std::pair<int, int>>& outPath)
{
    outPath.clear();

    int w = m_pTileManager->getMapWidth();
    int h = m_pTileManager->getMapHeight();

    auto idx = [&](int x, int y) { return y * w + x; };

    std::vector<int> parent(w * h, -1);
    std::vector<char> visited(w * h, 0);
    std::queue<std::pair<int, int>> q;

    visited[idx(startX, startY)] = 1;
    q.push(std::make_pair(startX, startY));

    const int dx[4] = { 1, -1, 0, 0 };
    const int dy[4] = { 0, 0, 1, -1 };

    bool found = false;

    while (!q.empty())
    {
        auto cur = q.front();
        q.pop();

        if (cur.first == endX && cur.second == endY)
        {
            found = true;
            break;
        }

        for (int i = 0; i < 4; ++i)
        {
            int nx = cur.first + dx[i];
            int ny = cur.second + dy[i];

            if (nx >= 0 && nx < w && ny >= 0 && ny < h && !visited[idx(nx, ny)])
            {
                int tileType = m_pTileManager->getMapValue(nx, ny);
                if (tileType == MainTileManager::TILE_PATH ||
                    tileType == MainTileManager::TILE_START ||
                    tileType == MainTileManager::TILE_END ||
                    tileType == MainTileManager::TILE_JUNCTION)
                {
                    visited[idx(nx, ny)] = 1;
                    parent[idx(nx, ny)] = idx(cur.first, cur.second);
                    q.push(std::make_pair(nx, ny));
                }
            }
        }
    }

    if (found)
    {
        int cur = idx(endX, endY);
        std::vector<std::pair<int, int>> reversePath;

        while (cur != -1)
        {
            reversePath.push_back(std::make_pair(cur % w, cur / w));
            cur = parent[cur];
        }

        for (int i = (int)reversePath.size() - 1; i >= 0; --i)
        {
            outPath.push_back(reversePath[i]);
        }
    }
}

// Builds multiple path routes for enemies including junction splits and merges
void MainEngine::buildPathRoutes()
{
    m_mainPath.clear();
    m_pathRoutes.clear();
    m_finalPath.clear();

    if (!m_pTileManager)
        return;

    int startX, startY, endX, endY;
    if (!m_pTileManager->getStartPosition(startX, startY) ||
        !m_pTileManager->getEndPosition(endX, endY))
        return;

    std::vector<std::pair<int, int>> junctions = m_pTileManager->getJunctions();

    if (junctions.empty())
    {
        BFSPath(startX, startY, endX, endY, m_enemyPath);
        m_mainPath = m_enemyPath;
        return;
    }

    std::pair<int, int> splitJunction = std::make_pair(5, 7);
    std::pair<int, int> mergeJunction = std::make_pair(19, 7);

    BFSPath(startX, startY, splitJunction.first, splitJunction.second, m_mainPath);

    PathRoute upperRoute;
    upperRoute.junctionPoint = splitJunction;
    upperRoute.mergePoint = mergeJunction;

    for (int y = 7; y >= 3; y--)
    {
        upperRoute.waypoints.push_back(std::make_pair(5, y));
    }

    for (int x = 6; x <= 19; x++)
    {
        upperRoute.waypoints.push_back(std::make_pair(x, 3));
    }

    for (int y = 4; y <= 7; y++)
    {
        upperRoute.waypoints.push_back(std::make_pair(19, y));
    }

    m_pathRoutes.push_back(upperRoute);

    PathRoute lowerRoute;
    lowerRoute.junctionPoint = splitJunction;
    lowerRoute.mergePoint = mergeJunction;

    for (int y = 7; y <= 11; y++)
    {
        lowerRoute.waypoints.push_back(std::make_pair(5, y));
    }

    for (int x = 6; x <= 19; x++)
    {
        lowerRoute.waypoints.push_back(std::make_pair(x, 11));
    }

    for (int y = 10; y >= 7; y--)
    {
        lowerRoute.waypoints.push_back(std::make_pair(19, y));
    }

    m_pathRoutes.push_back(lowerRoute);

    BFSPath(mergeJunction.first, mergeJunction.second, endX, endY, m_finalPath);

    m_enemyPath = m_mainPath;
    m_enemyPath.insert(m_enemyPath.end(), upperRoute.waypoints.begin(), upperRoute.waypoints.end());
    m_enemyPath.insert(m_enemyPath.end(), m_finalPath.begin(), m_finalPath.end());
}

// BFS pathfinding with directional preference for routing decisions
void MainEngine::BFSPathPreferDirection(int startX, int startY, int endX, int endY,
    std::vector<std::pair<int, int>>& outPath,
    int direction)
{
    outPath.clear();

    int w = m_pTileManager->getMapWidth();
    int h = m_pTileManager->getMapHeight();

    auto idx = [&](int x, int y) { return y * w + x; };
    //parent array store prev tile
    std::vector<int> parent(w * h, -1);
    //visted array tracks tiles already explored
    std::vector<char> visited(w * h, 0);
    //stores tile pairs to explaore 
    std::queue<std::pair<int, int>> q;
    //mark started as visteset and add to queue
    visited[idx(startX, startY)] = 1;
    q.push(std::make_pair(startX, startY));

    // direction array
    int dx[4], dy[4];
    if (direction == 0)
    {
        dx[0] = 1;  dy[0] = 0;
        dx[1] = 0;  dy[1] = -1;
        dx[2] = 0;  dy[2] = 1;
        dx[3] = -1; dy[3] = 0;
    }
    else
    {
        dx[0] = 0;  dy[0] = 1;
        dx[1] = 1;  dy[1] = 0;
        dx[2] = -1; dy[2] = 0;
        dx[3] = 0;  dy[3] = -1;
    }

    bool found = false;

    while (!q.empty())
    {
        auto cur = q.front();
        q.pop();

        if (cur.first == endX && cur.second == endY)
        {
            found = true;
            break;
        }
        //explore 4 neighbours
        for (int i = 0; i < 4; ++i)
        {
            int nx = cur.first + dx[i];
            int ny = cur.second + dy[i];
            //check if valid
            if (nx >= 0 && nx < w && ny >= 0 && ny < h && !visited[idx(nx, ny)])
            {
                int tileType = m_pTileManager->getMapValue(nx, ny);
                if (tileType == MainTileManager::TILE_PATH ||
                    tileType == MainTileManager::TILE_START ||
                    tileType == MainTileManager::TILE_END ||
                    tileType == MainTileManager::TILE_JUNCTION)
                {
                    //mark visited
                    visited[idx(nx, ny)] = 1;
                    parent[idx(nx, ny)] = idx(cur.first, cur.second);
                    q.push(std::make_pair(nx, ny));
                }
            }
        }
    }
    // If path found, reconstruct it by following parent links backwards
    if (found)
    {
        int cur = idx(endX, endY);
        std::vector<std::pair<int, int>> reversePath;

        while (cur != -1)
        {
            reversePath.push_back(std::make_pair(cur % w, cur / w));
            cur = parent[cur];
        }

        for (int i = (int)reversePath.size() - 1; i >= 0; --i)
        {
            outPath.push_back(reversePath[i]);
        }
    }
}

// Updates enemy logic and handles life loss when enemies reach the base
void MainEngine::updateEnemies()
{
    for (auto enemy : m_enemies)
    {
        if (enemy->reachedBase() && !enemy->isDead())
        {
            loseLife();
            enemy->takeDamage(9999);
        }
    }
}

// Updates tower logic (currently empty placeholder)
void MainEngine::updateTowers()
{
}

// Removes dead enemies from the game and awards money for kills
void MainEngine::cleanupDeadEnemies()
{
    for (int i = (int)m_enemies.size() - 1; i >= 0; --i)
    {
        // Remove enemies immediately when dead or marked for deletion
        if (m_enemies[i]->isDead() || m_enemies[i]->isMarkedForDeletion())
        {
            if (!m_enemies[i]->reachedBase() && m_enemies[i]->isDead())
            {
                addMoney(m_enemies[i]->getMoneyReward());
            }

            removeDisplayableObject(m_enemies[i]);
            delete m_enemies[i];
            m_enemies.erase(m_enemies.begin() + i);
            drawableObjectsChanged();
        }
    }
}

// Removes bullets that should be deleted from the game
void MainEngine::cleanupBullets()
{
    for (int i = (int)m_bullets.size() - 1; i >= 0; --i)
    {
        if (m_bullets[i]->shouldBeRemoved())
        {
            removeDisplayableObject(m_bullets[i]);
            delete m_bullets[i];
            m_bullets.erase(m_bullets.begin() + i);
            drawableObjectsChanged();
        }
    }
}

// Creates and places a tower at the specified tile coordinates
void MainEngine::placeTowerAtPosition(int tileX, int tileY)
{
    if (m_pTileManager->canPlaceTower(tileX, tileY))
    {
        m_pTileManager->placeTower(tileX, tileY, this);

        int worldX, worldY;
        m_pTileManager->getTileCenterPosition(tileX, tileY, worldX, worldY);
        worldX -= 20;
        worldY -= 20;

        TowerObject* tower = new TowerObject(this, worldX, worldY);
        m_towers.push_back(tower);
        appendObjectToArray(tower);
        drawableObjectsChanged();
    }
}

// Returns the tower at the given world position, or nullptr if none exists
TowerObject* MainEngine::getTowerAtPosition(int worldX, int worldY)
{
    for (auto tower : m_towers)
    {
        if (tower->isPointInside(worldX, worldY))
        {
            return tower;
        }
    }
    return nullptr;
}

// Draws a tooltip displaying tower stats and upgrade information
void MainEngine::drawTowerTooltip(TowerObject* tower)
{
    if (tower == nullptr)
        return;
    
    const int tooltipWidth = 180;
    const int tooltipHeight = 110;
    const int padding = 8;
    const int lineHeight = 18;
    
    int tooltipX = m_iMouseWorldX + 15;
    int tooltipY = m_iMouseWorldY + 15;
    
    if (tooltipX + tooltipWidth > getWindowWidth())
        tooltipX = m_iMouseWorldX - tooltipWidth - 15;
    if (tooltipY + tooltipHeight > getWindowHeight())
        tooltipY = m_iMouseWorldY - tooltipHeight - 15;
    
    drawForegroundRectangle(tooltipX, tooltipY, 
                           tooltipX + tooltipWidth, 
                           tooltipY + tooltipHeight, 
                           0x2A2A2A);
    
    drawForegroundRectangle(tooltipX, tooltipY, 
                           tooltipX + tooltipWidth, 
                           tooltipY + 2, 
                           0xFFD700);
    drawForegroundRectangle(tooltipX, tooltipY + tooltipHeight - 2, 
                           tooltipX + tooltipWidth, 
                           tooltipY + tooltipHeight, 
                           0xFFD700);
    drawForegroundRectangle(tooltipX, tooltipY, 
                           tooltipX + 2, 
                           tooltipY + tooltipHeight, 
                           0xFFD700);
    drawForegroundRectangle(tooltipX + tooltipWidth - 2, tooltipY, 
                           tooltipX + tooltipWidth, 
                           tooltipY + tooltipHeight, 
                           0xFFD700);
    
    char buffer[64];
    int textY = tooltipY + padding;
    
    // Show current level and max level
    sprintf(buffer, "Tower Level: %d/%d", tower->getLevel(), tower->getMaxLevel());
    drawForegroundString(tooltipX + padding, textY, buffer, 0xFFFFFF, getFont("Cornerstone Regular.ttf", 14));
    textY += lineHeight;
    
    sprintf(buffer, "Damage: %d", tower->getDamage());
    drawForegroundString(tooltipX + padding, textY, buffer, 0xFF6666, getFont("Cornerstone Regular.ttf", 14));
    textY += lineHeight;
    
    sprintf(buffer, "Range: %d", tower->getRange());
    drawForegroundString(tooltipX + padding, textY, buffer, 0x66CCFF, getFont("Cornerstone Regular.ttf", 14));
    textY += lineHeight;
    
    sprintf(buffer, "Sell Value: $%d", tower->getSellValue());
    drawForegroundString(tooltipX + padding, textY, buffer, 0x66FF66, getFont("Cornerstone Regular.ttf", 14));
    textY += lineHeight;
    
    if (tower->canUpgrade())
    {
        sprintf(buffer, "Upgrade Cost: $%d", tower->getUpgradeCost());
        drawForegroundString(tooltipX + padding, textY, buffer, 0xFFD700, getFont("Cornerstone Regular.ttf", 14));
        textY += lineHeight;
        
        drawForegroundString(tooltipX + padding, textY, "(Press U to upgrade)", 0xAAAAAA, getFont("Cornerstone Regular.ttf", 11));
    }
    else
    {
        drawForegroundString(tooltipX + padding, textY, "MAX LEVEL", 0xFF00FF, getFont("Cornerstone Regular.ttf", 14));
    }
}

// Resets all game state to initial values and clears all game objects
void MainEngine::resetGame()
{
    drawableObjectsChanged();

    for (int i = (int)m_enemies.size() - 1; i >= 0; --i)
    {
        if (m_enemies[i] != nullptr)
        {
            removeDisplayableObject(m_enemies[i]);
            delete m_enemies[i];
            m_enemies[i] = nullptr;
        }
    }
    m_enemies.clear();

    for (int i = (int)m_towers.size() - 1; i >= 0; --i)
    {
        if (m_towers[i] != nullptr)
        {
            removeDisplayableObject(m_towers[i]);
            delete m_towers[i];
            m_towers[i] = nullptr;
        }
    }
    m_towers.clear();

    for (int i = (int)m_bullets.size() - 1; i >= 0; --i)
    {
        if (m_bullets[i] != nullptr)
        {
            removeDisplayableObject(m_bullets[i]);
            delete m_bullets[i];
            m_bullets[i] = nullptr;
        }
    }
    m_bullets.clear();

    if (m_pPlayer != nullptr)
    {
        m_pPlayer->setWorldPosition(625, 875);
        m_pPlayer->setPosition(m_pPlayer->getWorldX(), m_pPlayer->getWorldY());
        m_pPlayer->setVelocity(0, 0);
    }

    if (m_pTileManager != nullptr)
    {
        m_pTileManager->resetMap();
    }

    m_iMoney = 20;
    m_iLives = 3;

    if (m_pWaveManager != nullptr)
    {
        m_pWaveManager->reset();
    }

    m_enemyPath.clear();
    buildEnemyPath();
    buildPathRoutes();

    lockAndSetupBackground();
    redrawDisplay();
}

// Calculates maximum allowed towers based on current wave progression
int MainEngine::getMaxTowers() const
{
    int currentWave = getWave();
    
    if (currentWave < 6)
    {
        return m_iBaseTowerLimit; // 5 towers
    }
    else
    {
        // Every 2 waves after wave 6, add 1 tower
        int wavesAfterSix = currentWave - 6;
        int bonusTowers = (wavesAfterSix / 2) + 1;
        return m_iBaseTowerLimit + bonusTowers;
    }
}

// Handles mouse scroll wheel events and delegates to menu state if active
void MainEngine::virtMouseWheel(int x, int y, int which, int timestamp) {

    if (m_currentStateType == StateType::MENU)
    {
        MenuState* menuState = dynamic_cast<MenuState*>(m_pCurrentState);
        if (menuState != nullptr)
        {
            menuState->handleMouseWheel(y);
        }
    }
}