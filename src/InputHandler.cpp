#include "header.h"
#include "InputHandler.h"
#include "MainEngine.h"

InputHandler::InputHandler(MainEngine* pEngine)
    : m_pEngine(pEngine)
    , m_bMovingUp(false)
    , m_bMovingDown(false)
    , m_bMovingLeft(false)
    , m_bMovingRight(false)
{
}

void InputHandler::handleKeyDown(int iKeyCode)
{
    switch (m_pEngine->getState())
    {
    case MainEngine::State::MENU:
        handleMenuInput(iKeyCode);
        break;
    case MainEngine::State::RUNNING:
        handleRunningInput(iKeyCode);
        break;
    case MainEngine::State::PAUSED:
        handlePausedInput(iKeyCode);
        break;
    case MainEngine::State::GAME_OVER:
        handleGameOverInput(iKeyCode);
        break;
    case MainEngine::State::WIN:
        handleWinInput(iKeyCode);
        break;
    }
}

void InputHandler::handleKeyUp(int iKeyCode)
{
    if (m_pEngine->getState() == MainEngine::State::RUNNING)
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
}

void InputHandler::handleMouseDown(int iButton, int iX, int iY)
{
    if (m_pEngine->getState() == MainEngine::State::MENU)
    {
        m_pEngine->setState(MainEngine::State::RUNNING);
    }
    else if (m_pEngine->getState() == MainEngine::State::RUNNING)
    {
        m_pEngine->handleBuildingPlacement(iButton, iX, iY);
    }
}

void InputHandler::handleMenuInput(int iKeyCode)
{
    switch (iKeyCode)
    {
    case SDLK_SPACE:
    case SDLK_RETURN:
        m_pEngine->setState(MainEngine::State::RUNNING);
        break;
    case SDLK_ESCAPE:
        m_pEngine->setExitWithCode(0);
        break;
    }
}

void InputHandler::handleRunningInput(int iKeyCode)
{
    switch (iKeyCode)
    {
    case SDLK_p:
    case SDLK_ESCAPE:
        m_pEngine->setState(MainEngine::State::PAUSED);
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
    }
}

void InputHandler::handlePausedInput(int iKeyCode)
{
    switch (iKeyCode)
    {
    case SDLK_p:
    case SDLK_SPACE:
        m_pEngine->setState(MainEngine::State::RUNNING);
        break;
    case SDLK_ESCAPE:
        m_pEngine->setState(MainEngine::State::MENU);
        break;
    }
}

void InputHandler::handleGameOverInput(int iKeyCode)
{
    switch (iKeyCode)
    {
    case SDLK_SPACE:
    case SDLK_RETURN:
        m_pEngine->setState(MainEngine::State::MENU);
        break;
    case SDLK_ESCAPE:
        m_pEngine->setExitWithCode(0);
        break;
    }
}

void InputHandler::handleWinInput(int iKeyCode)
{
    switch (iKeyCode)
    {
    case SDLK_SPACE:
    case SDLK_RETURN:
        m_pEngine->setState(MainEngine::State::MENU);
        break;
    case SDLK_ESCAPE:
        m_pEngine->setExitWithCode(0);
        break;
    }
}