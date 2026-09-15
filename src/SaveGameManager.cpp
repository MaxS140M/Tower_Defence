#include "header.h"
#include "SaveGameManager.h"
#include "MainEngine.h"
#include "MainTileManager.h"
#include "PlayerObject.h"
#include "TowerObject.h"
#include <fstream>
#include <sstream>
#include <direct.h>
#include <iomanip>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dirent.h>
#endif

SaveGameManager::SaveGameManager(MainEngine* pEngine)
    : m_pEngine(pEngine)
    , m_saveDirectory("saves/")
{
    // Create saves directory if it doesn't exist
    #ifdef _WIN32
        _mkdir(m_saveDirectory.c_str());
    #else
        mkdir(m_saveDirectory.c_str(), 0777);
    #endif
}

// a) Save high score
void SaveGameManager::saveHighScore(int wave)
{
    std::ofstream file(m_saveDirectory + "highscore.dat");
    if (file.is_open())
    {
        file << wave << std::endl;
        file.close();
    }
}

int SaveGameManager::loadHighScore()
{
    std::ifstream file(m_saveDirectory + "highscore.dat");
    int highScore = 0;
    if (file.is_open())
    {
        file >> highScore;
        file.close();
    }
    return highScore;
}

// b/c) Save map data (structured format)
void SaveGameManager::saveMapData(const std::string& filename, const std::vector<std::vector<int>>& mapData)
{
    std::ofstream file(m_saveDirectory + filename);
    if (file.is_open())
    {
        // Write header with metadata
        file << "# TOWER DEFENSE MAP FILE" << std::endl;
        file << "# Format: WIDTH HEIGHT" << std::endl;
        file << "# Tile types: 0=Grass, 1=Path, 2=Start, 3=End, 4=Tower, 5=Blocked, 6=Junction, 7=Object" << std::endl;
        
        int height = (int)mapData.size();
        int width = height > 0 ? (int)mapData[0].size() : 0;
        
        file << width << " " << height << std::endl;
        
        // Write map data
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                file << mapData[y][x];
                if (x < width - 1) file << " ";
            }
            file << std::endl;
        }
        file.close();
    }
}

bool SaveGameManager::loadMapData(const std::string& filename, std::vector<std::vector<int>>& mapData)
{
    std::ifstream file(m_saveDirectory + filename);
    if (!file.is_open())
        return false;

    mapData.clear();
    
    std::string line;
    int width = 0, height = 0;
    
    // Skip comment lines
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;
        
        // Read dimensions
        std::istringstream iss(line);
        iss >> width >> height;
        break;
    }
    
    // Read map data
    for (int y = 0; y < height; y++)
    {
        std::getline(file, line);
        std::istringstream iss(line);
        std::vector<int> row;
        
        int tile;
        for (int x = 0; x < width; x++)
        {
            iss >> tile;
            row.push_back(tile);
        }
        mapData.push_back(row);
    }
    
    file.close();
    return true;
}

// Generate unique save filename
std::string SaveGameManager::generateSaveFilename()
{
    int saveID = getNextSaveID();
    int wave = m_pEngine->getWave();
    
    std::ostringstream oss;
    oss << "save_" << std::setfill('0') << std::setw(3) << saveID 
        << "_wave_" << wave << ".sav";
    
    return oss.str();
}

// Get next available save ID
int SaveGameManager::getNextSaveID()
{
    std::vector<std::string> savedGames = listSavedGames();
    int maxID = 0;
    
    for (const auto& filename : savedGames)
    {
        // Parse filename format: save_XXX_wave_Y.sav
        size_t pos = filename.find("save_");
        if (pos != std::string::npos)
        {
            pos += 5; // Skip "save_"
            std::string idStr = filename.substr(pos, 3);
            try
            {
                int id = std::stoi(idStr);
                if (id > maxID)
                    maxID = id;
            }
            catch (...)
            {
                // Ignore invalid filenames
            }
        }
    }
    
    return maxID + 1;
}

// Auto-save with generated filename
std::string SaveGameManager::autoSaveGameState()
{
    std::string filename = generateSaveFilename();
    saveGameState(filename);
    return filename;
}

// Save COMPLETE game state
bool SaveGameManager::saveGameState(const std::string& filename)
{
    std::ofstream file(m_saveDirectory + filename);
    if (!file.is_open())
        return false;

    // Save game state metadata
    file << "# TOWER DEFENSE SAVE FILE" << std::endl;
    file << "VERSION 2.0" << std::endl;
    file << "USERNAME " << m_pEngine->getUsername() << std::endl;
    
    // Save core game state
    file << "MONEY " << m_pEngine->getMoney() << std::endl;
    file << "LIVES " << m_pEngine->getLives() << std::endl;
    file << "WAVE " << m_pEngine->getWave() << std::endl;
    
    // Save player position
    file << "PLAYER " << m_pEngine->getPlayer()->getWorldX() << " " 
         << m_pEngine->getPlayer()->getWorldY() << std::endl;
    
    // Save tower data (including levels and upgrades)
    const auto& towers = m_pEngine->getTowers();
    file << "TOWERS " << towers.size() << std::endl;
    for (const auto& tower : towers)
    {
        // Format: worldX worldY level
        file << tower->getWorldX() << " " 
             << tower->getWorldY() << " "
             << tower->getLevel() << std::endl;
    }
    
  
    // Save map state (including objects)
    MainTileManager* tm = m_pEngine->getTileManager();
    file << "MAP " << tm->getMapWidth() << " " << tm->getMapHeight() << std::endl;
    for (int y = 0; y < tm->getMapHeight(); y++)
    {
        for (int x = 0; x < tm->getMapWidth(); x++)
        {
            file << tm->getMapValue(x, y);
            if (x < tm->getMapWidth() - 1) file << " ";
        }
        file << std::endl;
    }
    
    file.close();
    return true;
}

bool SaveGameManager::loadGameState(const std::string& filename)
{
    std::ifstream file(m_saveDirectory + filename);
    if (!file.is_open())
        return false;

    std::string line, keyword;

    // Clear existing game state
    m_pEngine->clearTowers();

    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream iss(line);
        iss >> keyword;

        if (keyword == "VERSION")
        {
            std::string version;
            iss >> version;
        }
        else if (keyword == "USERNAME")
        {
            std::string username;
            iss >> username;
            m_pEngine->setUsername(username);
        }
        else if (keyword == "MONEY")
        {
            int money;
            iss >> money;
            m_pEngine->setMoney(money);
        }
        else if (keyword == "LIVES")
        {
            int lives;
            iss >> lives;
            m_pEngine->setLives(lives);
        }
        else if (keyword == "WAVE")
        {
            int wave;
            iss >> wave;
            m_pEngine->setWave(wave);
        }
        else if (keyword == "PLAYER")
        {
            int x, y;
            iss >> x >> y;
            m_pEngine->getPlayer()->setWorldPosition(x, y);
        }
        else if (keyword == "TOWERS")
        {
            int count;
            iss >> count;

            // Load towers with their upgrade levels
            for (int i = 0; i < count; i++)
            {
                std::getline(file, line);
                std::istringstream towerStream(line);
                int x, y, level;
                towerStream >> x >> y >> level;

                // Load tower at position
                m_pEngine->loadTower(x, y);

                // Upgrade tower to saved level
                const auto& towers = m_pEngine->getTowers();
                if (!towers.empty())
                {
                    TowerObject* tower = towers.back();
                    for (int lvl = 1; lvl < level; lvl++)
                    {
                        if (tower->canUpgrade())
                        {
                            tower->upgrade();
                        }
                    }
                }
            }
        }
        else if (keyword == "ENEMIES")
        {
            int count;
            iss >> count;

            // Note: Enemies are dynamic and respawn based on waves
            // Skip loading enemies and let wave manager handle spawning
            for (int i = 0; i < count; i++)
            {
                std::getline(file, line); // Skip enemy data
            }
        }
        else if (keyword == "MAP")
        {
            int width, height;
            iss >> width >> height;

            std::vector<std::vector<int>> mapData;
            for (int y = 0; y < height; y++)
            {
                std::getline(file, line);
                std::istringstream mapStream(line);
                std::vector<int> row;
                int tile;
                for (int x = 0; x < width; x++)
                {
                    mapStream >> tile;
                    row.push_back(tile);
                }
                mapData.push_back(row);
            }
            m_pEngine->loadMapData(mapData);
        }
    }

    file.close();

    // Refresh display after loading
    m_pEngine->lockAndSetupBackground();
    m_pEngine->redrawDisplay();

    return true;
}

// h) Username management
void SaveGameManager::saveUsername(const std::string& username)
{
    std::ofstream file(m_saveDirectory + "username.dat");
    if (file.is_open())
    {
        file << username << std::endl;
        file.close();
    }
}

std::string SaveGameManager::loadUsername()
{
    std::ifstream file(m_saveDirectory + "username.dat");
    std::string username;
    if (file.is_open())
    {
        std::getline(file, username);
        file.close();
    }
    return username.empty() ? "Player" : username;
}

// List all saved game files
std::vector<std::string> SaveGameManager::listSavedGames()
{
    std::vector<std::string> savedGames;
    
#ifdef _WIN32
    WIN32_FIND_DATAA findData;
    std::string searchPath = m_saveDirectory + "*.sav";
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                savedGames.push_back(findData.cFileName);
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(m_saveDirectory.c_str());
    if (dir != nullptr)
    {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr)
        {
            std::string filename = entry->d_name;
            if (filename.length() > 4 && 
                filename.substr(filename.length() - 4) == ".sav")
            {
                savedGames.push_back(filename);
            }
        }
        closedir(dir);
    }
#endif
    
    return savedGames;
}

// Delete a saved game file
bool SaveGameManager::deleteSavedGame(const std::string& filename)
{
    std::string fullPath = m_saveDirectory + filename;

#ifdef _WIN32
    return DeleteFileA(fullPath.c_str()) != 0;
#else
    return remove(fullPath.c_str()) == 0;
#endif
}