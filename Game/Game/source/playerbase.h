/*********************************************************************/
// * \file   playerbase.h
// * \brief  プレイヤーベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#pragma once
#include "charabase.h"
#include "camera.h"
#include "appframe.h"

class PlayerBase : public CharaBase
{
	typedef CharaBase base;
public:

	enum class PlayerType
	{
		TANUKI,
		HUMAN,
		MONO,
	};

	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	void CopyStateFrom(PlayerBase* player); // 別のプレイヤーから状態をコピーする関数

	void SetCamera(Camera* cam)  override { _cam = cam; }

	int GetMakimonoCount() const { return _makimonoCount; } // まきものの所持数を取得する
	void SetMakimonoCount(int count) { _makimonoCount = count; } // まきものの所持数を設定する
	void AddMakimono(int addCount);                         // まきものの所持数を増やす
	void SubMakimono(int subCount);                         // まきものの所持数を減らす	

	virtual bool IsDash() const { return false; } // ダッシュ中かどうかを返す（デフォルトはfalse）

	int GetModelHandle() const { return _handle; }

	void SetInputDisable(bool disable) { _inputDisabled = disable; } // 入力を無効にする
	bool IsInputDisabled() const { return _inputDisabled; }		   // 入力が無効かどうかを返す

	// 入力が有効かどうかのゲッターとセッター
	bool GetInputEnabled() const { return _inputEnabled; }
	void SetInputEnabled(bool enabled) { _inputEnabled = enabled; }
protected:

	//アナログスティック関係
	float _fAnalogDeadZone; // アナログスティックの無効範囲
	float fLx, fLz, fRx, fRy;
	DINPUT_JOYSTATE di;
	//左スティックの値
	float lStickX, lStickZ;
	Camera* _cam;

	// 移動方向を決める
	vec3::Vec3 _v;

	// メモ読み込み
	std::string _memoFileName;

	int _makimonoCount; // まきものの所持数	
	int _padHorizontalSign; // パッドの水平入力の符号（-1, 0, 1）
	bool _applyYFlip; // Y軸反転を適用するかどうか
	float _yaw; // プレイヤーの向き（Y軸回転角）
	float _targetY; // プレイヤーの向きの目標値（Y軸回転角）
	float _ySpeed; // プレイヤーの向きの回転速度
	float _yRotation; // プレイヤーのY軸回転角

	bool _inputDisabled; // 入力を受け付けるかどうか
	bool _inputEnabled; // 入力が有効かどうか
};

