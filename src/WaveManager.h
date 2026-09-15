#pragma once

class MainEngine;

class WaveManager
{
public:
    WaveManager(MainEngine* pEngine);

    void update(int currentTime);
    void reset();
    
    int getWave() const { return m_iWave; }
    void setWave(int wave) { m_iWave = wave; }  // ADD THIS
    int getEnemiesPerWave() const { return m_iEnemiesPerWave; }

private:
    void startNextWave();
    void spawnEnemy();
    bool isWaveComplete() const;

    MainEngine* m_pEngine;
    int m_iWave;
    int m_iEnemiesSpawned;
    int m_iEnemiesPerWave;
    int m_iLastWaveSpawnTime;
    int m_iSpawnInterval;
    int m_iWaveCompleteTime;
};