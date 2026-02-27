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
#include "CinematicCamera.h"
#include "playerbase.h"
#include "player.h"
#include "playertanuki.h"
#include "enemybase.h"
#include "enemy.h"
#include "enemymove.h"
#include "enemydog.h"
#include "treasure.h"
#include "map.h"
#include "cube.h"
#include "enemysensor.h"
#include "uibase.h"
#include "uihp.h"
#include "UiMakimono.h"	
#include "charashadow.h"
#include "playermono.h"
#include "effectbase.h"
#include "treasureeffect.h"
#include "MapBase.h"
#include "Map1.h"
#include "ObjectServer.h"
#include "Goal.h"
#include "soundserver3D.h"
#include "ModeGoalConfirm.h"
#include "hensineffect.h"
#include "walkeffect.h"
#include "findeffect.h"
#include "hatenaeffect.h"
#include "aseeffect.h"
#include "doyaeffect.h"
#include "nakieffect.h"
#include "Makimono.h"
#include "enemysoundmanager.h"
#include "modegameload.h"
#include "StageManager.h"
#include "henshinui.h"

constexpr float CHECK_OPEN_TIME = 1.0f; // 宝箱が開くまでの時間（秒）

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
	bool PlayerToMakimonoCollision(PlayerBase* player, at::vspc<Makimono>& makimono);// プレイヤーと巻物の当たり判定処理
    // キャラと宝箱の当たり判定処理
	bool CharaToTreasureHitCollision(CharaBase* chara, const at::vspc<Treasure>& treasures);
	bool CharaToTreasureOpenCollision(PlayerBase* player, const at::vspc<Treasure>& treasures);
	// プレイヤーとゴールの当たり判定
	bool PlayerToGoalHitCollision(PlayerBase* player, Goal* goal);

	// 索敵範囲の当たり判定
	bool IsPlayerAttack(PlayerBase* player, at::vspc<EnemyBase>& enemy);
	
	// デバック関数
	bool DebugInitialize();
	bool DebugProcess();
	bool DebugRender();

	// カメラ情報
	bool CameraInfoInitialize();
	bool PlayerCameraInfo(PlayerBase* player);// プレイヤーのカメラ情報表示

	// オブジェクト関数
	bool ObjectInitialize();
	bool ObjectProcess();
	bool ObjectRender();

	// プレイヤー変身関数
	bool PlayerTransform();
	bool PlayerTransformToTanuki(bool player);

	// 影関数
	bool ShadowInitialize();

	// カメラ操作公開API（メニューから呼び出すため）
	void CameraMoveBy(const vec::Vec3& delta);
	void CameraZoomTowardsTarget(float amount);
	bool DebugCameraControl();
	//　演出カメラ
	bool TreasureOpeningCameraControl();
	bool EndCinematicCamera();
	bool DebugCinematicCameraControl();
	// メニューから開始/終了されるカメラ編集（現在のカメラ状態を保存・復元）
	void StartCameraControlAndSave();
	void EndCameraControlAndRestore();

	// BGMチェンジ
	bool ChangeBGM();

	bool LoadStageData();

	// 取得数（UI等で使う想定）
	int GetTreasureTakenCount() const { return _treasureTakenCount; }
	// クリアに必要な宝箱数
	int GetTreasureRequiredCount() const { return _treasureRequiredCount; }

	// ゴール可能か？
	bool CanGoal() const{ return _treasureTakenCount >= _treasureRequiredCount; }

	// ステージを完全リセットして初期状態に戻す
	bool ResetStage();

	// ステージリセット要求フラグのセッター（Process内でこのフラグをチェックしてリセット処理を行う）
	void RequestResetStage() { _requestResetStage = true; }

	void RequestNextStage()  { _requestNextStage  = true; }
	bool HasRenderOnce() const { return _hasRenderOnce; }

	bool IsLoadComplete() const { return _isLoadComplete; }
	void SetLoadComplete(bool b) { _isLoadComplete = b; }

protected:
	Camera* _camera;
	Camera* _originalCamera;
	std::unique_ptr<CinematicCamera> _cinematicCamera;
    // メニュー開始前のカメラ状態を保存するためのメンバ
    vec::Vec3 _savedCamPos;
    vec::Vec3 _savedCamTarget;
    bool _hasSavedCameraState;
	bool _useCinematicCamera; // 演出カメラ切り替えフラグ
	// デバッグ用演出カメラ制御用変数
	bool _debugF1KeyPressed = false; // F1キーの連続入力防止用
	bool _debugZoomActive = false;   // ズーム演出が実行中かどうか

	bool _hasRenderOnce;
	bool _requestResetStage; // ステージリセット要求フラグ
	bool _requestNextStage; // 次のステージ要求フラグ
	// キャラクタ管理
	at::vspc<CharaBase> _chara;
	at::vspc<ObjectBase> _object;
	at::vspc<PlayerBase> _playerBase;
	at::vspc<EnemyBase> _enemyBase;
	at::spc<Player> _player;
	at::spc<PlayerTanuki> _playerTanuki;
	at::spc<PlayerMono> _playerMono;
	// 宝箱(オブジェクト)
	at::vspc<Treasure> _treasure;
	// 巻物
	at::vspc<Makimono> _makimono;
	//ステージベース
	at::vspc<MapBase> _mapBase;
	// map1
	at::spc<Map1> _map1;
	// キューブ
	at::spc<Cube> _cube;
	// ゴール
	at::spc<Goal> _goal;
	// 敵
	at::vspc<Enemy> _enemy;
	at::vspc<EnemyMove> _enemyMove;
	at::vspc<EnemyDog> _enemyDog;
	// UI
	at::vspc<UiBase> _uiBase;
	at::spc<UiHp> _uiHp;
	at::spc<UiMakimono> _uiMakimono;
	at::spc<HenshinUi> _henshinUi;
	// シャドウ
	at::vspc<CharaShadow> _charaShadow;

	// エフェクト
	at::vspc<EffectBase> _effectBase;
	at::spc<TreasureEffect> _treasureEffect;
	// オブジェクトサーバー
	class ObjectServer* _objectServer;

	at::spc<HensinEffect> _hensinEffect;
	at::spc<WalkEffect> _walkEffect;
	at::spc<FindEffect> _findEffect;
	at::spc<HatenaEffect> _hatenaEffect;
	at::spc<AseEffect> _aseEffect;
	at::spc<DoyaEffect> _doyaEffect;
	at::spc<NakiEffect> _nakiEffect;

	at::spc<SoundServer3D> _sound3D;
	// デバッグ用
	bool _d_view_collision;
	bool _d_use_collision;
	bool _d_view_camera_info;
	bool _d_view_shadow_map;

	// メニューから切り替える「カメラ操作モード」フラグ
	bool _bCameraControlMode;
	
	bool anyDetected;	// プレイヤー検出フラグ
	bool _bTransCancel;	// 変身キャンセルフラグ
	bool _bResolveOnY;// Y方向のコリジョン解決を行うかどうか
	bool _bLandedOnUp;// 上方向に着地したかどうか

	bool _bShowTanuki;// タヌキプレイヤー表示フラグ
	bool _showMonoPlayer;// モノプレイヤー表示フラグ
	bool _isTransformToHuman = false;
	bool _isTransformToMono = false;
	int _transformAnimId = -1;
	bool _isTanukiAttackPlaying = false;
	int _tanukiAttackAnimId = -1;

	// Effekseer を既に起動済みかどうか（メニューから二重起動を防ぐ）
	bool _effekseerLaunched = false;

	// 索敵システム
	at::spc<EnemySensor> _enemySensor;
	at::spc<soundserver::SoundServer> _soundServer;
	soundserver::SoundItemBase* _bgmInitialize;
	soundserver::SoundItemBase* _bgmChenge;

	bool _isChengeBgm;

	// 索敵関連の処理（簡略化）
	bool CheckAllDetections();// 全体の索敵チェック

	// --- 宝箱取得（A長押し）用 ---
	int _treasureTakenCount = 0;             // 取得した宝箱の数
	float _treasureHoldSec = 0.0f;           // 押下時間カウント用
	bool _treasureTakenThisTreasure = false; // 宝箱ごとに1回だけカウントするフラグ
	bool _isOpeningTreasure = false;         // 宝箱を開けている最中かどうか（UI表示用）
	// クリアに必要な宝箱の数
	int _treasureRequiredCount = 1;

	// --- 画面メッセージ（敵を転ばせた） ---
	bool _showKnockdownMessage = false;
	float _knockdownMessageSec = 0.0f;

	// ゲームクリア処理
	bool _isGameClear;

	// ゲーム開始時刻（ms）・クリア表示済みフラグ
	unsigned long _gameStartMs = 0;
	bool _gameClearShown = false;

	// ゴール確認モード関連
	bool _goalConfirmOpened;
	ModeGoalConfirm::Result _goalConfirmResult;
	// 「No を選んだ直後は、ゴールから離れるまで確認を出さない」ためのフラグ
	bool _notGoalFlag;
	// ゴール確認処理
	bool UpdateGoalConfirm(PlayerBase* player);

	bool _changeTimeActive;		// プレイヤーの時間制限フラグ
	float _changeTimeLimit;		// プレイヤーの時間制限（秒）
	float _changeBlinkTimer;	// プレイヤーの点滅タイマー
	bool _changeBlinkVisible;	// プレイヤーの点滅表示フラグ
	float _changeBlinkInterval; // プレイヤーの点滅間隔

	// 巻物関連
	bool _subMakimono = false; // 変身開始時に巻物を消費する予約をする

	// ロード時間計測用
	float _loadTimeMs = 0.0f; // ロードにかかった時間（ミリ秒）

	// Process内の各処理セクション実行時間計測用（マイクロ秒）
	float _processCameraMs = 0.0f;
	float _processAnimationMs = 0.0f;
	float _processEffekseerMs = 0.0f;
	float _processEnemySoundMs = 0.0f;
	float _processObjectServerMs = 0.0f;
	float _processSoundListenerMs = 0.0f;
	float _processPlayerTransformMs = 0.0f;
	float _processObjectProcessMs = 0.0f;
	float _processCollisionMs = 0.0f;
	float _processEnemyAIMs = 0.0f;
	float _processPlayerCollisionMs = 0.0f;
	float _processPlayerEnemyMs = 0.0f;
	float _processGoalMs = 0.0f;
	float _processAttackMs = 0.0f;
	float _process3DSoundMs = 0.0f;
	float _processChangeTimeMs = 0.0f;
	float _processBGMMs = 0.0f;
	float _processTotalMs = 0.0f;
	float _processSectorDetectionMs = 0.0f; // 扇形検出処理の時間

	bool _isLoadComplete; // ロード中かどうか（デバッグ用）
	ModeGameLoad* _modeGameLoad;
	StageManager _stageManager; // ステージ管理
};

