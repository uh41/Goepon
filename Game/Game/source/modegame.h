/*********************************************************************/
// * \file   modegame.h
// * \brief  モードゲームクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#pragma once
#include "appframe.h"
#include "modemenu.h"
#include "charabase.h"
#include "objectbase.h"
#include "camera.h"
#include "playerbase.h"
#include "player.h"
#include "playertanuki.h"
#include "enemy.h"
#include "treasure.h"
#include "map.h"
#include "cube.h"
#include "enemysensor.h"
#include "uibase.h"
#include "uihp.h"
#include "charashadow.h"



class ModeGame : public ModeBase
{
	typedef ModeBase base;
public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	bool GetDebugViewCollision() const { return _d_view_collision; }
	bool GetDebugUseCollision() const { return _d_use_collision; }
	bool GetDebugViewCameraInfo() const { return _d_view_camera_info; }
	bool GetDebugViewShadowMap() const { return _d_view_shadow_map; }
	void SetDebugViewCollsion(bool d) { this->_d_view_collision = d; }
	void SetDebugUseCollision(bool d) { this->_d_use_collision = d; }
	void SetDebugViewCameraInfo(bool d) { this->_d_view_camera_info = d; }
	void SetDebugViewShadowMap(bool d) { this->_d_view_shadow_map = d; }
	void SetCameraControlMode(bool b) {_bCameraControlMode = b;}
	bool GetCameraControlMode() const { return _bCameraControlMode; }

	// Effekseer 再生フラグ管理
	bool GetEffekseerLaunched() const { return _effekseerLaunched; }
	void SetEffekseerLaunched(bool b) { _effekseerLaunched = b; }


	bool PushChara(CharaBase* move, CharaBase* stop);
	
	bool IsHitCircle(CharaBase* c1, CharaBase* c2);
	bool IsHitCircle(CharaBase* target)
	{
		return IsHitCircle(_player.get(), target);
	}

	// 当たり判定処理
	bool EscapeCollision(CharaBase* chara, ObjectBase* obj);// キャラの回避処理
	bool CharaToCharaCollision(CharaBase* c1, CharaBase* c2);// キャラ同士の当たり判定処理

	// 索敵範囲の当たり判定
	bool IsPlayerAttack(PlayerBase* player, at::vec<Enemy*> enemy);
	
	// デバック関数
	bool DebugInitialize();
	bool DebugProcess();
	bool DebugRender();

	// カメラ情報
	bool PlayerCameraInfo(PlayerBase* player);// プレイヤーのカメラ情報表示

	// オブジェクト関数
	bool ObjectInitialize();
	bool ObjectProcess();

	// プレイヤー変身関数
	bool PlayerTransform();

	// 影関数
	bool ShadowInitialize();

	// カメラ操作公開API（メニューから呼び出すため）
	void CameraMoveBy(const vec::Vec3& delta);
	void CameraZoomTowardsTarget(float amount);
	bool DebugCameraControl();

	// メニューから開始/終了されるカメラ編集（現在のカメラ状態を保存・復元）
	void StartCameraControlAndSave();
	void EndCameraControlAndRestore();

	// BGMチェンジ
	bool ChangeBGM();

	bool LoadStageData();


protected:
	Camera* _camera;
    // メニュー開始前のカメラ状態を保存するためのメンバ
    vec::Vec3 _savedCamPos;
    vec::Vec3 _savedCamTarget;
    bool _hasSavedCameraState;

	// キャラクタ管理
	at::vspc<CharaBase> _chara;
	at::vspc<ObjectBase> _object;
	at::vspc<PlayerBase> _playerBase;
	at::spc<Player> _player;
	at::spc<PlayerTanuki> _playerTanuki;
	// 宝箱(オブジェクト)
	at::vspc<Treasure> _treasure;
	// マップ
	at::spc<Map> _map;
	// キューブ
	at::spc<Cube> _cube;
	// 敵
	at::vspc<Enemy> _enemy;
	// UI
	at::vspc<UiBase> _uiBase;
	at::spc<UiHp> _uiHp;
	// シャドウ
	at::vspc<CharaShadow> _charaShadow;
	// デバッグ用
	bool _d_view_collision;
	bool _d_use_collision;
	bool _d_view_camera_info;
	bool _d_view_shadow_map;

	// メニューから切り替える「カメラ操作モード」フラグ
	bool _bCameraControlMode;
	

	bool _bResolveOnY;// Y方向のコリジョン解決を行うかどうか
	bool _bLandedOnUp;// 上方向に着地したかどうか

	bool _bShowTanuki;// タヌキプレイヤー表示フラグ
	bool _isTransformingToHuman = false;
	int _transformAnimId = -1;

	int _iBlocks;	// ブロック数
	std::vector<float> _vBlockFill; // 表示用現在値(0..1)
	std::vector<float> _vBlockTarget; // 目標値(0..1)
	float _fBlockAnimSpeed; // ブロックアニメーション速度

	// Effekseer を既に起動済みかどうか（メニューから二重起動を防ぐ）
	bool _effekseerLaunched = false;

	// 索敵システム
	at::spc<EnemySensor> _enemySensor;

	at::spc<soundserver::SoundServer> _soundServer;
	at::spc<soundserver::SoundItemBase> _bgmInitialize;
	at::spc<soundserver::SoundItemBase> _bgmChenge;
	bool _isChengeBgm;

	// 索敵関連の処理（簡略化）
	bool CheckAllDetections();// 全体の索敵チェック
};

