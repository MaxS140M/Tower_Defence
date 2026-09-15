#ifndef RUNNING_STATE_H
#define RUNNING_STATE_H

#include "GameState.h"
#include "ImageManager.h"

class TowerObject;

class RunningState : public GameState
{
public:
    RunningState(MainEngine* pEngine);

    void draw() override;
    void handleKeyDown(int iKeyCode) override;
    void handleKeyUp(int iKeyCode) override;
    void handleMouseDown(int iButton, int iX, int iY) override;
    void update(int iCurrentTime) override;
    void onEnter() override;  // ADD THIS

private:
    void drawTextWithOutline(int x, int y, const char* text, unsigned int textColor, unsigned int outlineColor);


    bool m_bMovingUp;
    bool m_bMovingDown;
    bool m_bMovingLeft;
    bool m_bMovingRight;

    // FPS tracking
    int m_iLastFPSTime;
    int m_iFrameCount;
    int m_iCurrentFPS;

    // Tower selection
    TowerObject* m_pSelectedTower;
};

#endif