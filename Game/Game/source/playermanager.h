/*********************************************************************/
// * \file   playermanager.h
// * \brief  プレイヤー状態管理クラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#pragma once
#include "playerbase.h"

namespace PlayerConstant
{
	static constexpr auto TRANSORM_TIME_LIMIT = 17.0f;		// 変身の時間制限（秒）

	static constexpr auto BLINK_INTERVAL = 0.1f;			// 点滅の基本間隔（秒）
	static constexpr auto BLINK_SPEED_THRESHOLD_1 = 3.0f;	// 3秒以下で2倍速
	static constexpr auto BLINK_SPEED_THRESHOLD_2 = 1.0f;	// 1秒以下で4倍速
	static constexpr auto BLINK_SPEED_2X = 0.5f;			// 2倍速の間隔（秒）
	static constexpr auto BLINK_SPEED_4X = 0.25f;			// 4倍速の間隔（秒）
}

class PlayerManager
{
public:
	enum class PlayerState
	{
		TANUKI,
		HUMAN,
		MONO
	};

	static PlayerManager* GetInstance(); // シングルトンインスタンスの取得

	bool Initialize(); // 初期化
	bool Terminate();  // 終了処理

	// 状態遷移関数
	void TransformToTanuki(); // タヌキに変身する関数
	void TransformToHuman(); // 人間に変身する関数
	void TransformToMono(); // モノに変身する関数

	// 状態の取得関数
	PlayerState GetPlayerState();
	bool IsShowTanuki();
	bool IsShowHuman();
	bool IsShowMono();
	bool IsTransforming();
	bool IsTransformRequest();

	// 変身時間制限の管理関数
	void SetTransformTimeLimit(float second);// 変身時間制限を設定する関数
	float GetTransformTimeLimit(); // 変身時間制限を取得する関数
	void UpdateTransformTimer(float dt); // 変身時間制限のタイマーを更新する関数
	bool IsTransformTimeLimitActive(); // 変身時間制限がアクティブかどうかを取得する関数

	// 変身の要求
	void RequestTransformToTanuki(); // タヌキへの変身を要求する関数
	void RequestTransformToHuman();// 人間への変身を要求する関数
	void RequestTransformToMono(); // モノへの変身を要求する関数
	void CancelTransformRequest(); // 変身要求をキャンセルする関数

	// 点滅制御
	bool GetBlinkVisible(); // 点滅が現在表示状態かどうかを取得する関数
	void UpdateBlinkTimer(float dt); // 点滅タイマーを更新する関数

	// 初期化
	void SetInitialPlayerState(PlayerState state); // 初期プレイヤー状態を設定する関数

	// アニメーション制御
	void SetTransformAnimation(int animId); // 変身アニメーションを設定する関数
	int GetTransformAnimation(); // 変身アニメーションを取得する関数
	bool IsTransformAnimationPlaying(); // 変身アニメーションが再生中かどうかを取得する関数

private:
	PlayerManager() = default;
	~PlayerManager() = default;

	// コピー・ムーブ禁止
	PlayerManager(const PlayerManager&) = delete;
	PlayerManager& operator=(const PlayerManager&) = delete;
	PlayerManager(PlayerManager&&) = delete;
	PlayerManager& operator=(PlayerManager&&) = delete;

	PlayerState _playerState; // 現在のプレイヤー状態

	// 変身要求のフラグ
	bool _requestedTransformToHuman;
	bool _requestedTransformToMono;
	bool _requestedTransformToTanuki;

	// 変身時間制限の管理
	bool _transformTimeActive;
	float _transformTimeLimit;
	float _transformTimer;

	// 点滅制御の管理
	bool _blinkVisible;
	float _blinkTimer;
	float _blinkInterval;

	// 変身アニメーションの管理
	int _transformAnimId;

	void ApplyTransition(PlayerState newState);// 状態遷移を適用する関数
};

