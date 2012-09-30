//
// MainLoop.cpp
//

#include "MainLoop.hpp"
#include "Option.hpp"
#include "Dashboard.hpp"
#include <vector>
#include <algorithm>
#include "../ResourceManager.hpp"
#include "../WindowManager.hpp"
#include "../Core.hpp"
#include <shlwapi.h>
#include "ServerChange.hpp"
#include "../Music.hpp"

namespace scene {
MainLoop::MainLoop(const ManagerAccessorPtr& manager_accessor) :
      manager_accessor_(manager_accessor),
      player_manager_(std::make_shared<PlayerManager>(manager_accessor_)),
      card_manager_(manager_accessor->card_manager().lock()),
      command_manager_(manager_accessor->command_manager().lock()),
      world_manager_(std::make_shared<WorldManager>(manager_accessor_)),
      account_manager_(manager_accessor->account_manager().lock()),
      config_manager_(manager_accessor->config_manager().lock()),
      window_manager_(std::make_shared<WindowManager>(manager_accessor_)),
      inputbox_(std::make_shared<InputBox>(manager_accessor_)),
	  minimap_(std::make_shared<MiniMap>(manager_accessor)),
	  snapshot_number_(0),
	  snapshot_(false)
{
    manager_accessor_->set_player_manager(player_manager_);
    manager_accessor_->set_world_manager(world_manager_);
    manager_accessor_->set_window_manager(window_manager_);

    inputbox_->ReloadTabs();
	inputbox_->Activate();
	card_manager_->AddNativeCard("inputbox", inputbox_);

	minimap_->UIPlacement(config_manager_->screen_width() - MINIMAP_MINSIZE - 12, 12);
	card_manager_->AddNativeCard("radar", minimap_);

	window_manager_->RestorePosition();

    player_manager_->Init();
    world_manager_->Init();	

    world_manager_->myself()->Init(unicode::ToTString(account_manager_->model_name()));
}

MainLoop::~MainLoop()
{
	window_manager_->SavePosition();
    account_manager_->Save("./user/account.xml");
}

void MainLoop::Begin()
{

}

void MainLoop::Update()
{
	if (auto window_manager = manager_accessor_->window_manager().lock()) {
		window_manager->Update();
	}
    command_manager_->Update();
    player_manager_->Update();
    card_manager_->Update();
    world_manager_->Update();
	ResourceManager::music()->Update();

}

void MainLoop::ProcessInput(InputManager* input)
{
	if(world_manager_->stage()->host_change_flag())
	{
		//account_manager_->set_host(world_manager_->stage()->host_change_flag().second);
		next_scene_ = std::make_shared<scene::ServerChange>(manager_accessor_);
	} else if (input->GetKeyCount(KEY_INPUT_F1) == 1) {
		inputbox_->Inactivate();
		next_scene_ = std::make_shared<scene::Option>(manager_accessor_, shared_from_this());
	} else if (input->GetKeyCount(KEY_INPUT_F2) == 1) {
		inputbox_->Inactivate();
		next_scene_ = std::make_shared<scene::Dashboard>(manager_accessor_, shared_from_this());
	}

	if (auto window_manager = manager_accessor_->window_manager().lock()) {
		window_manager->ProcessInput(input);
	}
    player_manager_->ProcessInput(input);
    card_manager_->ProcessInput(input);
    world_manager_->ProcessInput(input);

	if(input->GetKeyCount(InputManager::KEYBIND_SCREEN_SHOT) == 1)
	{
		snapshot_ = true;
	}

}

void MainLoop::Draw()
{
    world_manager_->Draw();
    player_manager_->Draw();
    card_manager_->Draw();
	if (auto window_manager = manager_accessor_->window_manager().lock()) {
		window_manager->Draw();
	}

	if (snapshot_) {
		using namespace boost::filesystem;
		if (!exists("./screenshot")) {
			create_directory("./screenshot");
		}
		TCHAR tmp_str[MAX_PATH];
		_stprintf( tmp_str , _T(".\\screenshot\\ss%03d.png") , snapshot_number_ );
		if(PathFileExists(tmp_str))
		{
			while(1)
			{
				snapshot_number_++;
				_stprintf( tmp_str , _T(".\\screenshot\\ss%03d.png") , snapshot_number_ );
				if(!PathFileExists(tmp_str))break;
			}
		}
		SaveDrawScreenToPNG( 0, 0, config_manager_->screen_width(), config_manager_->screen_height(),tmp_str);
		snapshot_number_++;
		snapshot_ = false;
	}
}

void MainLoop::End()
{
	next_scene_ = BasePtr();
}

}

