#pragma once
#include "appframe.h"
#include <string>
#include <vector>
#include <memory>
#include "modemenu.h"
#include "charabase.h"
#include "objectbase.h"
#include "camera.h"
#include "player.h"
#include "enemy.h"
#include "map.h"
#include "cube.h"



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

	bool PushChara(CharaBase* move, CharaBase* stop);
	
	bool IsHitCircle(CharaBase* c1, CharaBase* c2);
	bool IsHitCircle(CharaBase* target)
	{
		return IsHitCircle(_player.get(), target);
	}

	// 当たり判定処理
	bool EscapeCollision();// キャラの回避処理
	bool CharaToCharaCollision(CharaBase* c1, CharaBase* c2);// キャラ同士の当たり判定処理
	bool CharaToCubeCollision(CharaBase* chara, Cube* cube);// キャラとキューブの当たり判定処理
	bool LandCheck();// 着地判定処理
	bool UpdateCheckAttackCollision();// 攻撃用当たり判定の更新処理

	// デバック関数
	bool DebugInitialize();
	bool DebugProcess();
	bool DebugRender();

	// カメラ情報
	bool PlayerCameraInfo();// プレイヤーのカメラ情報表示

	// オブジェクト関数
	bool ObjectInitialize();

protected:
	Camera* _camera;

	// キャラクタ管理
	std::vector<std::shared_ptr<CharaBase>> _chara;
	std::vector<std::shared_ptr<ObjectBase>> _object;
	std::shared_ptr<Player> _player;
	// マップ
	std::shared_ptr<Map> _map;
	// キューブ
	std::shared_ptr<Cube> _cube;
	// 敵
	std::vector<std::shared_ptr<Enemy>> _enemy;
	// デバッグ用
	bool _d_view_collision;
	bool _d_use_collision;
	bool _d_view_camera_info;
	bool _d_view_shadow_map;

	bool _resolve_on_y;// Y方向のコリジョン解決を行うかどうか
	bool _landed_on_up;// 上方向に着地したかどうか

};

