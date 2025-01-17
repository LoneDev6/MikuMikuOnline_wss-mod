//
// Title.cpp
//

#include "Connect.hpp"
#include "Title.hpp"
#include <vector>
#include <algorithm>
#include "../ResourceManager.hpp"
#include "../ConfigManager.hpp"
#include "../AccountManager.hpp"
#include "../../common/Logger.hpp"

namespace scene {
	
const int Title::BASE_BLOCK_SIZE = 12;

Title::Title(const ManagerAccessorPtr& manager_accessor) :
              manager_accessor_(manager_accessor),
              card_manager_(manager_accessor->card_manager().lock()),
              account_manager_(manager_accessor->account_manager().lock()),
              config_manager_(manager_accessor->config_manager().lock()),
              connect_flag_(false),
              screen_count_(0),
			  input_host_(manager_accessor->config_manager().lock())
{
    base_image_handle_ = ResourceManager::LoadCachedDivGraph<4>(
            _T("system/images/gui/gui_button_base.png"), 2, 2, 12, 12);
}

Title::~Title()
{
}

void Title::Begin()
{
    int width, height;
    GetScreenState(&width, &height, nullptr);
	input_host_.Init();
    input_host_.set_active(true);
    input_host_.set_text(unicode::ToTString(account_manager_->host()));
    input_host_.set_width(200);
	input_host_.set_multiline(false);
    input_host_.set_x((width - input_host_.width()) / 2);
    input_host_.set_y((height - input_host_.height()) / 2 - 30);
    input_host_.set_message(_LT("connect.enter_server_address"));

    input_host_.set_on_enter([this](const std::string& text) -> bool{
        connect_flag_ = true;
        return false;
    });

    label_.set_width(30);
    label_.set_text(_LT("connect.connect"));
    label_.set_bgcolor(UIBase::Color(0,0,0,0));
    label_.set_textcolor(UIBase::Color(0,0,0,255));
    label_.set_left((width - 26) / 2);
    label_.set_top((height - input_host_.height()) / 2 + 40);

    button_.set_height(32);
    button_.set_left((width - button_.width()) / 2);
    button_.set_top((height - button_.height()) / 2 + 40);

    button_.set_on_click([this](){
        connect_flag_ = true;
    });

	// ※ jsonに記載されたロビーサーバに接続するように修正
	auto lobby_servers_ = config_manager_->lobby_servers();
	auto lobby_connected_ = false;
	for(auto it=lobby_servers_.begin();it !=lobby_servers_.end(); ++it) {
		auto lobby_server_= *it;
		if(lobby_server_ != "m2op.net") { // m2op.netだった場合は除外
			lobby_connected_ = lobby_.Reload(lobby_server_);
			if (lobby_connected_){
				break;
			}
		}
	}
	if (lobby_connected_ == false) { // jsonに記載された全てのサーバに繋がらない場合は
		lobby_.Reload("z.moe.hm");   // デフォルトサーバ z.moe.hm に接続試行する
	}
	//	lobby_.Reload("m2op.net");
	// ここまで

}

void Title::Update()
{
    input_host_.Update();

    button_.Update();
    label_.Update();

    if (connect_flag_) {
        screen_count_++;
    }

    if (screen_count_ > 30) {
        account_manager_->set_host(unicode::ToString(input_host_.text()));
        next_scene_ = BasePtr(new scene::Connect(manager_accessor_));
    }
}

void Title::ProcessInput(InputManager* input)
{
    input_host_.ProcessInput(input);
    button_.ProcessInput(input);

	int button_top = 0;
	BOOST_FOREACH(const auto& server, lobby_.servers()) {
		if (!server->name().empty()) {

			int x = 20;
			int y = button_top + 20;
			int width = 260;
			int height = 65;

			bool hover = (x<= input->GetMouseX() && input->GetMouseX() <= x + width
					&& y <= input->GetMouseY() && input->GetMouseY() <= y + height);

			if (hover && input->GetMouseLeftCount() == 1) {
				input_host_.set_text(unicode::ToTString(server->host()));
				connect_flag_ = true;
			}

			button_top += height + 10;
		}
	}
}

void Title::Draw()
{

    int width, height;
    GetScreenState(&width, &height, nullptr);
    DrawBox(0, 0, width, height, GetColor(157, 212, 187), TRUE);

    button_.Draw();
    label_.Draw();
    input_host_.Draw();

	int button_top = 0;
	BOOST_FOREACH(const auto& server, lobby_.servers()) {
		if (!server->name().empty()) {

			int x = 20;
			int y = button_top + 20;
			int width = 260;
			int height = 65;

			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
			DrawGraph(x, y, *base_image_handle_[0], TRUE);
			DrawGraph(x + width - BASE_BLOCK_SIZE, y, *base_image_handle_[1], TRUE);
			DrawGraph(x, y + height - BASE_BLOCK_SIZE, *base_image_handle_[2], TRUE);
			DrawGraph(x + width - BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE, *base_image_handle_[3], TRUE);

			DrawRectExtendGraphF(x + BASE_BLOCK_SIZE, y,
								 x + width - BASE_BLOCK_SIZE, y + BASE_BLOCK_SIZE,
								 0, 0, 1, BASE_BLOCK_SIZE, *base_image_handle_[1], TRUE);

			DrawRectExtendGraphF(x + BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE,
								 x + width - BASE_BLOCK_SIZE, y + height,
								 0, 0, 1, BASE_BLOCK_SIZE, *base_image_handle_[3], TRUE);

			DrawRectExtendGraphF(x, y + BASE_BLOCK_SIZE,
								 x + BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE,
								 0, 0, BASE_BLOCK_SIZE, 1, *base_image_handle_[2], TRUE);

			DrawRectExtendGraphF(x + width - BASE_BLOCK_SIZE, y + BASE_BLOCK_SIZE,
								 x + width, y + height - BASE_BLOCK_SIZE,
								 0, 0, BASE_BLOCK_SIZE, 1, *base_image_handle_[3], TRUE);

			DrawRectExtendGraphF(x + BASE_BLOCK_SIZE, y + BASE_BLOCK_SIZE,
								 x + width - BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE,
								 0, 0, 1, 1, *base_image_handle_[3], TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

			SetDrawArea(x, y, x + width, y + height);
			DrawStringToHandle(x + 10, y + 10,
				unicode::ToTString(server->name()).c_str(),
				GetColor(100, 0, 0), ResourceManager::default_font_handle());

			DrawStringToHandle(x + 10, y + 10 + 15,
				unicode::ToTString(server->note()).c_str(),
				GetColor(90, 90, 90), ResourceManager::default_font_handle());

			auto num_and_stage = (tformat(_T("%d/%d  %s"))
				% server->player_num()
				% server->capacity() 
				% unicode::ToTString(server->stage())).str();

			DrawStringToHandle(x + 10, y + 10 + 30, num_and_stage.c_str(),
				GetColor(0, 0, 0), ResourceManager::default_font_handle());

			SetDrawAreaFull();

			button_top += height + 10;
		}
	}

	int alpha = std::min(255, screen_count_ * 10);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	DrawBox(0, 0, width, height, GetColor(157, 212, 187), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Title::End()
{
}

}
