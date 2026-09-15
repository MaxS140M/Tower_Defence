#include "header.h"
#include "WaveManager.h"
#include "MainEngine.h"
#include "EnemyObject.h"

WaveManager::WaveManager(MainEngine* pEngine)
    : m_pEngine(pEngine)
    , m_iWave(1)
    , m_iEnemiesSpawned(0)
    , m_iEnemiesPerWave(5)
    , m_iLastWaveSpawnTime(0)
    , m_iSpawnInterval(1000)
    , m_iWaveCompleteTime(0)
{
}

void WaveManager::update(int currentTime)
{
    // Spawn enemies for current wave
    if (m_iEnemiesSpawned < m_iEnemiesPerWave)
    {
        if (currentTime - m_iLastWaveSpawnTime >= m_iSpawnInterval)
        {
            spawnEnemy();
            m_iEnemiesSpawned++;
            m_iLastWaveSpawnTime = currentTime;
        }
    }

    // Check if wave is complete
    if (isWaveComplete())
    {
        if (m_iWaveCompleteTime == 0)
        {   
            m_iWaveCompleteTime = currentTime;
        }

        if (currentTime - m_iWaveCompleteTime >= 3000) // 4 second delay
        {
            startNextWave();
            m_iWaveCompleteTime = 0;
        }
    }
    // WIN STATE
    if (m_iWave > 19)  
    {
        m_pEngine->changeState(MainEngine::StateType::WIN);
    }
}

void WaveManager::startNextWave()
{
    m_iWave++;
    m_iEnemiesPerWave += 2;
    m_iEnemiesSpawned = 0;
    m_iLastWaveSpawnTime = m_pEngine->getModifiedTime();
}

bool WaveManager::isWaveComplete() const
{
    return m_iEnemiesSpawned >= m_iEnemiesPerWave && m_pEngine->getEnemyCount() == 0;
}

void WaveManager::reset()
{
    m_iWave = 1;
    m_iEnemiesSpawned = 0;
    m_iEnemiesPerWave = 5;
    m_iLastWaveSpawnTime = 0;
    m_iWaveCompleteTime = 0;
}


void WaveManager::spawnEnemy()
{

    EnemyObject* enemy = new EnemyObject(m_pEngine, m_iWave);
    m_pEngine->addEnemy(enemy);
}