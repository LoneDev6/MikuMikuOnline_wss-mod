﻿//
// MainLoop.cpp
//

#include "MainLoop.hpp"
#include "Option.hpp"
#include "Dashboard.hpp"
#include "ChannelChange.hpp"
#include <vector>
#include <algorithm>
#include "../PlayerManager.hpp"
#include "../CardManager.hpp"
#include "../CommandManager.hpp"
#include "../WorldManager.hpp"
#include "../AccountManager.hpp"
#include "../ConfigManager.hpp"
#include "../ResourceManager.hpp"
#include "../WindowManager.hpp"
#include "../SocketServerManager.hpp"
#include "../Core.hpp"
#include <shlwapi.h>
#include "ServerChange.hpp"
#include "../Music.hpp"
#include "../../common/unicode.hpp" // ※ スクリーンショット採取時にメッセージを出すために追加

namespace scene {
MainLoop::MainLoop(const ManagerAccessorPtr& manager_accessor) :
      manager_accessor_(manager_accessor),
      player_manager_(manager_accessor->player_manager().lock()),
      card_manager_(manager_accessor->card_manager().lock()),
      command_manager_(manager_accessor->command_manager().lock()),
      world_manager_(manager_accessor->world_manager().lock()),
      account_manager_(manager_accessor->account_manager().lock()),
      config_manager_(manager_accessor->config_manager().lock()),
      window_manager_(std::make_shared<WindowManager>(manager_accessor_)),
	  socket_server_manager_(std::make_shared<SocketServerManager>(manager_accessor_)),
      inputbox_(std::make_shared<InputBox>(manager_accessor_)),
	  minimap_(std::make_shared<MiniMap>(manager_accessor)),
	  snapshot_number_(0),
	  snapshot_(false),
	  fade_counter_(0)
{
    manager_accessor_->set_window_manager(window_manager_);

    inputbox_->ReloadTabs();
	inputbox_->Activate();
	card_manager_->AddNativeCard("inputbox", inputbox_);

	minimap_->UIPlacement(config_manager_->screen_width() - MINIMAP_MINSIZE - 12, 12);
	card_manager_->AddNativeCard("radar", minimap_);

	window_manager_->RestorePosition();

	socket_server_manager_->Start();

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

	static int cleanup_counter = 0;
	if (cleanup_counter % (60 * 30) == 0) {
		ResourceManager::ClearModelHandle();
	}
	cleanup_counter++;

	if (fade_counter_ < 120) {
		fade_counter_++;
	}
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

// ※ ここから  ゲームパッド対応にするため修正
//	if(input->GetKeyCount(InputManager::KEYBIND_SCREEN_SHOT) == 1)
	if((input->GetKeyCount(InputManager::KEYBIND_SCREEN_SHOT) == 1 && GetActiveFlag() != 0)||  // ※ 非アクティブ時はキーは効かない様に修正
		input->GetGamepadCount(InputManager::PADBIND_SCREEN_SHOT) == 1)
// ※ ここまで
	{
		snapshot_ = true;
	}

	if (const auto& channel = command_manager_->current_channel()) {
		BOOST_FOREACH(const auto& warp_point, channel->warp_points) {
			auto point = warp_point.position;
			point.y += 30;
			const auto& pos = player_manager_->GetMyself()->position();

			auto distance = VSize(warp_point.position - VGet(pos.x, pos.y, pos.z));
// ※ ここから  ゲームパッド対応にするため修正
//			if (distance < 50 && input->GetKeyCount(KEY_INPUT_M) == 1) {
			if (distance < 50 && ((input->GetKeyCount(KEY_INPUT_M) == 1 && GetActiveFlag() != 0)||  // ※ 非アクティブ時はキーは効かない様に修正
				input->GetGamepadCount(InputManager::PADBIND_WARP) == 1)) {

				// 同一チャンネルの場合は移動するだけ
				if (player_manager_->GetMyself()->channel() == warp_point.channel) {
					if (warp_point.destination) {
						world_manager_->myself()->ResetPosition(warp_point.destination);
					}
				} else {
					next_scene_ = std::make_shared<scene::ChannelChange>(
						warp_point.channel,
						warp_point.destination,
						manager_accessor_);
				}
			}
		}
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
		// ※ スクリンショット採取時に擬似メッセージが出るように修正　ここから
		boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
        auto time_string = to_iso_extended_string(now);
        std::string info_json;
		auto id = player_manager_->GetMyself()->id();
        info_json += "{";
        info_json += (boost::format("\"id\":\"%d\",") % id).str();
        info_json += (boost::format("\"time\":\"%s\"") % time_string).str();
        info_json += "}";
//		_stprintf( tmp_str , _T("{\"type\":\"chat\",\"body\":\"スクリーンショットを保存しました:ss%03d.png\"}") , snapshot_number_ );
        _stprintf( tmp_str , _T("{\"private\":[%d,%d],\"body\":\"スクリーンショットを保存しました:ss%03d.png\"}") , id,id,snapshot_number_ );
		card_manager_->OnReceiveJSON(info_json, unicode::ToString(tmp_str));
		// ※ ここまで
		snapshot_number_++;
		snapshot_ = false;
	}
	
	if (const auto& channel = command_manager_->current_channel()) {
		BOOST_FOREACH(const auto& warp_point, channel->warp_points) {
			auto point = warp_point.position;
			point.y += 30;
			const auto& pos = player_manager_->GetMyself()->position();
			
			auto distance = VSize(warp_point.position - VGet(pos.x, pos.y, pos.z));

			if (world_manager_->stage()->IsVisiblePoint(point)) {
				auto screen_pos = ConvWorldPosToScreenPos(point);
				int x = static_cast<int>(screen_pos.x / 2) * 2;
				int y = static_cast<int>(screen_pos.y / 2) * 2 - 16;

				UILabel label_;
				label_.set_width(160);
				auto text = _T("【") + unicode::ToTString(warp_point.name) + _T("】");
				if (distance < 50) {
					text += _LT("game.teleport_with_m_key");
					label_.set_bgcolor(UIBase::Color(255,0,0,150));
				} else {
					label_.set_bgcolor(UIBase::Color(0,0,0,150));
				}
				label_.set_text(text);
				label_.set_textcolor(UIBase::Color(255,255,255,255));

				label_.set_left(x - 60);
				label_.set_top(y + 10);

				label_.Update();
				label_.Draw();
			}
		}
	}

	if (command_manager_->status() == CommandManager::STATUS_ERROR){
		SetDrawBlendMode(DX_BLENDMODE_MUL, 120);
		DrawBox(0,0,config_manager_->screen_width(),config_manager_->screen_height(),GetColor(0,0,0),1);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		auto str = unicode::ToTString(_LT("connection.connection_lost"));
		auto width = GetDrawStringWidthToHandle(str.c_str(),str.length(),ResourceManager::default_font_handle());
		DrawStringToHandle(
			config_manager_->screen_width() / 2 - width,
			config_manager_->screen_height() / 2 - ResourceManager::default_font_size(),
			str.c_str(),GetColor(255,255,255),ResourceManager::default_font_handle());
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	if (fade_counter_ < 120) {
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255 * (120 - fade_counter_) / 120);
		int width, height;
		GetScreenState(&width, &height, nullptr);
		DrawBox(0, 0, width, height, GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

void MainLoop::End()
{
	next_scene_ = BasePtr();
}

}

