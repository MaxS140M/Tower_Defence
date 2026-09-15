#pragma once

#include "BaseEngine.h"
#include "MainTileManager.h"
#include "SaveGameManager.h"
#include <vector>
#include <map>
#include <queue>

class PlayerObject;
class EnemyObject;
class TowerObject;
class BulletObject;
class WaveManager;
class GameState;

class MainEngine : public BaseEngine
{
public:
    enum class StateType
    {
        MENU,
        RUNNING,
        PAUSED,
        GAME_OVER,
        WIN
    };

    struct PathRoute
    {
        std::vector<std::pair<int, int>> waypoints;
        std::pair<int, int> junctionPoint;
        std::pair<int, int> mergePoint;
    };

    MainEngine();
    virtual ~MainEngine();

    void virtSetupBackgroundBuffer() override;
    int virtInitialiseObjects() override;
    void virtDrawStringsOnTop() override;
    void virtKeyDown(int iKeyCode) override;
    void virtKeyUp(int iKeyCode) override;
    void virtMouseDown(int iButton, int iX, int iY) override;
    void virtMouseMoved(int iX, int iY) override;  // NEW: Track mouse for hover
    void virtMainLoopDoBeforeUpdate() override;

    void changeState(StateType newState);
    StateType getCurrentStateType() const { return m_currentStateType; }

    void handleBuildingPlacement(int iButton, int iX, int iY);
    void updateGameLogic(int iCurrentTime);

    MainTileManager* getTileManager() const { return m_pTileManager; }
    PlayerObject* getPlayer() const { return m_pPlayer; }
    int getMoney() const { return m_iMoney; }
    int getLives() const { return m_iLives; }
    int getWave() const;
    int getEnemyCount() const { return (int)m_enemies.size(); }
    int getEnemiesPerWave() const;
    const std::vector<std::pair<int, int>>& getEnemyPath() const { return m_enemyPath; }
    std::vector<EnemyObject*> getEnemies() const { return m_enemies; }
    int getTowerCost() const;

    // NEW: Tower limit
    int getTowerCount() const { return (int)m_towers.size(); }
    int getMaxTowers() const { return m_iMaxTowers; }

    const std::vector<PathRoute>& getAllRoutes() const { return m_pathRoutes; }
    const std::vector<std::pair<int, int>>& getMainPath() const { return m_mainPath; }
    const std::vector<std::pair<int, int>>& getFinalPath() const { return m_finalPath; }

    void addMoney(int amount) { m_iMoney += amount; }
    void subtractMoney(int amount) { m_iMoney -= amount; }
    void loseLife();

    void addEnemy(EnemyObject* enemy);
    void addBullet(BulletObject* bullet);

    int screenToWorldX(int screenX) const { return screenX; }
    int screenToWorldY(int screenY) const { return screenY; }

    void resetGame();

    void setMoney(int money) { m_iMoney = money; }
    void setLives(int lives) { m_iLives = lives; }
    void setWave(int wave);
    void clearTowers();
    void loadTower(int x, int y);
    void loadMapData(const std::vector<std::vector<int>>& mapData);
    const std::vector<TowerObject*>& getTowers() const { return m_towers; }

    SaveGameManager* getSaveManager() const { return m_pSaveManager; }

    void setUsername(const std::string& username) { m_username = username; }
    const std::string& getUsername() const { return m_username; }

private:
    void initializeStates();
    void buildEnemyPath();
    void buildPathRoutes();
    void BFSPath(int startX, int startY, int endX, int endY, std::vector<std::pair<int, int>>& outPath);
    void BFSPathPreferDirection(int startX, int startY, int endX, int endY, std::vector<std::pair<int, int>>& outPath, int direction);
    void updateEnemies();
    void updateTowers();
    void cleanupDeadEnemies();
    void cleanupBullets();
    void placeTowerAtPosition(int tileX, int tileY);
    bool canBuildAtWorldPosition(int worldX, int worldY);

    // NEW: Tower hover tooltip methods
    void drawTowerTooltip(TowerObject* tower);
    TowerObject* getTowerAtPosition(int worldX, int worldY);

    StateType m_currentStateType;
    GameState* m_pCurrentState;
    std::map<StateType, GameState*> m_states;

    MainTileManager* m_pTileManager;
    PlayerObject* m_pPlayer;
    WaveManager* m_pWaveManager;

    int m_iMoney;
    int m_iLives;
    int m_iBaseTowerCost;
    int m_iMaxTowers;  // NEW: Tower limit

    std::vector<EnemyObject*> m_enemies;
    std::vector<TowerObject*> m_towers;
    std::vector<BulletObject*> m_bullets;
    std::vector<std::pair<int, int>> m_enemyPath;

    std::vector<std::pair<int, int>> m_mainPath;
    std::vector<PathRoute> m_pathRoutes;
    std::vector<std::pair<int, int>> m_finalPath;

    SaveGameManager* m_pSaveManager;
    std::string m_username;

    // NEW: Hover tracking
    int m_iMouseWorldX;
    int m_iMouseWorldY;
    TowerObject* m_pHoveredTower;
};