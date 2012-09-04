#pragma once

#include <vector>
#include <array>
#include <string>
#include <DxLib.h>
#include "../../common/unicode.hpp"
#include "../InputManager.hpp"
#include "Character.hpp"
#include "CharacterManager.hpp"
#include "CharacterDataProvider.hpp"
#include "FieldPlayer.hpp"
#include "../ResourceManager.hpp"
#include "MotionPlayer.hpp"
#include "Timer.hpp"

class Stage;
typedef std::shared_ptr<Stage> StagePtr;

struct CameraStatus
{
    CameraStatus(float radius_ = 0, float target_height_ = 0,
        float theta_ = 0, float phi_ = 0, float manual_control_ = 0,
        const std::pair<int, int>& manual_control_startpos_ = std::pair<int, int>()) : 
    radius(radius_), target_height(target_height_), theta(theta_),
        phi(phi_), manual_control(manual_control_),
        manual_control_startpos(manual_control_startpos) {}

    //float height;           // カメラY座標のプレイヤー身長に対する比率
    float radius;          // プレイヤーを中心とするカメラ回転軌道の半径
    float target_height;    // 注視点Y座標のプレイヤー身長に対する比率
    float theta, phi;    // カメラの横方向の回転thetaと、縦方向の回転phi、単位はラジアン
    bool manual_control; // true:カメラの方向決定は手動モード
    std::pair<int, int> manual_control_startpos; // 手動モードになった瞬間のマウス座標
};

class KeyChecker
{
public:
    int Check();
    int GetKeyCount(size_t key_code) const;

private:
    std::array<int, 256> key_count_;
};

typedef std::shared_ptr<CharacterManager> CharacterManagerPtr;

class GameLoop
{
public:
    GameLoop(const StagePtr& stage);
    int Init(std::shared_ptr<CharacterManager> character_manager);
    int Logic(InputManager* input);
    int Draw();

    FieldPlayerPtr myself() const;
    void ResetCameraPosition();

private:
    // 自分自身
    CharacterManagerPtr charmgr_;
    FieldPlayerPtr myself_;

    StagePtr stage_;
    CameraStatus camera_default_stat, camera;

    void FixCameraPosition();
    void MoveCamera(InputManager* input);

    const static float CAMERA_MIN_RADIUS;
    const static float CAMERA_MAX_RADIUS;
};
