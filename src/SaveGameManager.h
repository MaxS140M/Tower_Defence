#pragma once

#include <string>
#include <vector>

class MainEngine;

class SaveGameManager
{
public:
    SaveGameManager(MainEngine* pEngine);

    // a) Save/load high score (highest wave reached)
    void saveHighScore(int wave);
    int loadHighScore();

    // b/c) Save/load map data
    void saveMapData(const std::string& filename, const std::vector<std::vector<int>>& mapData);
    bool loadMapData(const std::string& filename, std::vector<std::vector<int>>& mapData);

    // d/e/f/g) Save/load complete game state
    bool saveGameState(const std::string& filename);
    bool loadGameState(const std::string& filename);
    
    // Auto-save with generated filename
    std::string autoSaveGameState();

    // h) Save/load username
    void saveUsername(const std::string& username);
    std::string loadUsername();

    // List all saved game files
    std::vector<std::string> listSavedGames();
    
    // Delete a saved game file
    bool deleteSavedGame(const std::string& filename);

private:
    MainEngine* m_pEngine;
    std::string m_saveDirectory;
    
    // Helper to generate save filename
    std::string generateSaveFilename();
    int getNextSaveID();
};