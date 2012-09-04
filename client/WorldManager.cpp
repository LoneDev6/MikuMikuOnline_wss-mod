//
// WorldManager.cpp
//

#include "WorldManager.hpp"
#include "PlayerManager.hpp"
#include "3d/Stage.hpp"

WorldManager::WorldManager(const ManagerAccessorPtr& manager_accessor) :
manager_accessor_(manager_accessor),
stage_(std::make_shared<Stage>(_T("stage:ケロリン町"))),
game_loop_(stage_)
{

}

void WorldManager::Init()
{
    auto player_manager = manager_accessor_->player_manager().lock();
    game_loop_.Init(player_manager->charmgr());

}

void WorldManager::ProcessInput(InputManager* input)
{
    game_loop_.Logic(input);
}

void WorldManager::Update()
{

}

void WorldManager::Draw()
{
    game_loop_.Draw();
}

bool WorldManager::IsVisiblePoint(const VECTOR& point)
{
    return stage_->IsVisiblePoint(point);
}

StagePtr WorldManager::stage()
{
    return stage_;
}

FieldPlayerPtr WorldManager::myself()
{
    return game_loop_.myself();
}

void WorldManager::ResetCameraPosition()
{
    game_loop_.ResetCameraPosition();
}