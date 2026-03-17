/*********************************************************************/
// * \file   playertanuki.h
// * \brief  狸状態クラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
//				vec::Vec3を使用するように修正　鈴木裕稀　2026 / 01 / 17
/*********************************************************************/

#pragma once
#include "playerbase.h"
#include "camera.h"

namespace dash
{
	static constexpr auto DASH_MAX = 5.0f; // 最大ダッシュ回数
	static constexpr auto DASH_COOL_DOWN_DURATION = 5.0f;// ダッシュクールダウンの持続時間（秒）
	static constexpr auto DASH_RECOVER_INTERVAL = 5.0f;// ダッシュ回復のインターバル（秒）
	static constexpr auto DASH_SPEED = 5.0f; // ダッシュ中の移動速度
	static constexpr auto DASH_DURATION = 0.3f; // ダッシュ状態の持続時間
	static constexpr auto DASH_COOLDOWN_SPEED = 0.8f; // クールダウン中の移動量
}

class PlayerTanuki : public PlayerBase
{
	typedef PlayerBase base;
public:

	bool Initialize() override;
	bool Terminate() override;
	bool Process() override;
	bool Render() override;

	void SetCamera(Camera* cam)  override { _cam = cam; if(_cam) { _camOffset = vec3::VSub(_cam->GetPos(), _vPos); _camTargetOffset = vec3::VSub(_cam->GetTarget(), _vPos); } }
	bool SoundWalk();// 歩行音の再生

	virtual bool IsDash() const override { return _dash; } // ダッシュ中かどうかを返す（狸は常にダッシュ状態）
	float GetDashCoolDownTime() const { return _dashCoolDownTime; }

	auto GetDashCount() const { return _dashCount; } // ダッシュ回数を取得するゲッター

	// 入力が有効かどうかのゲッターとセッター
	bool GetInputEnabled() const { return _inputEnabled; }
	void SetInputEnabled(bool enabled) { _inputEnabled = enabled; }

	// クリア演出用のモデルへ切り替え
	bool SetClearHandle(const std::string& animName, bool loop);
	int  PlayGameClearAnimation(std::string name, bool loop);

	void ResetDash();// ダッシュ状態をリセットする関数

protected:
	Camera* _cam;

	// カメラ追従用オフセット
	vec::Vec3 _camOffset;
	vec::Vec3 _camTargetOffset;

	bool _dash; // ダッシュ状態かどうか
	float _dashTimer; // ダッシュ状態の経過時間
	

	float _normalSpeed; // 元の移動速度を保持
	int _dashCount; // ダッシュ回数

	float _dashCoolDownTime; // ダッシュクールダウンの持続時間

	float _dashRecoverTime; // ダッシュ回復のタイマー
	bool _dashRecoverActive; // ダッシュ回復がアクティブかどうか

	bool _inputEnabled; // 入力が有効かどうか

	bool _transformPlayerButtonDown; // 変身ボタンが押されているかどうか
	bool _transformPlayerMove; // 変身中に移動しているかどうか
	bool _transformPlayerCandidate; // 変身可能な状態かどうか
	bool _transformPlayerMonoButtonDown; // 変身モードのボタンが押されているかどうか
	bool _transformPlayerMonoMove; // 変身モード中に移動しているかどうか
	bool _transformPlayerMonoCandidate; // 変身モード可能な状態かどうか

	float _target_Rotation_Y;
private:
	int _clearModelHandle; // クリア後のモデルハンドル
};

