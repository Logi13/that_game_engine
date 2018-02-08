#include "Game.hpp"

Game::Game() : window("that platform game")
{
    vikingTexture.loadFromFile(workingDir.Get() + "viking.png");
    vikingSprite.setTexture(vikingTexture);
    
    deltaTime = clock.restart().asSeconds();
}

void Game::Update()
{
    window.Update();
    
    const sf::Vector2f& spritePos = vikingSprite.getPosition();
    int pixelsToMovePerSec = 100;
    float frameMovement = pixelsToMovePerSec * deltaTime;
    vikingSprite.setPosition(spritePos.x + frameMovement, spritePos.y);
}

void Game::LateUpdate()
{
    deltaTime = clock.restart().asSeconds();
}

void Game::Draw()
{
    window.BeginDraw();
    
    window.Draw(vikingSprite);
    
    window.EndDraw();
}

bool Game::IsRunning() const
{
    return window.IsOpen();
}
