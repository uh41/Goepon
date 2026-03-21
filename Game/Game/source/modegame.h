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
#include "TreasureRapidFire.h"
#include "TreasureBase.h"
#include "map.h"
#include "cube.h"
#include "enemysensor.h"
#include "uibase.h"
#include "uihp.h"
#include "UiMakimono.h"	
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
#include "doyaeffect.h"
#include "treasureopen.h"
#include "nakieffect.h"
#include "shirimochieffect.h"
#include "stuneffect.h"
#include "Makimono.h"
#include "enemysoundmanager.h"
#include "modegameload.h"
#include "StageManager.h"
#include "henshinui.h"
#include "counterui.h"
#include "treasureui.h"
#include "attackui.h"
#include "treasureopenui.h"
#include "dashui.h"
#include "tutorial.h"
#include "savemanager.h"
#include "savepoint.h"
#include "savepointeffect.h"
#include "makimonogeteffect.h"
#include "goaleffect.h"

// 定数定義

// ゲームクリア関連
namespace GAMECLEAR
{
	static constexpr float CLEAR_CINEMATIC_DURATION = 3.0f; // クリア演出の総時間（秒）
}

// ゲームオーバー関連
namespace GAMEOVER
{
	static constexpr float GAMEOVER_CINEMATIC_DURATION = 3.0f; // ゲームオーバー演出の総時間（秒）
}

constexpr float CHECK_OPEN_TIME = 1.0f; // 宝箱が開くまでの時間（秒）

namespace timelimit
{
	static constexpr auto START_TIME_LIMIT = 5.0f; // 制限時間（秒）
	static constexpr auto MIDDLE_TIME_LIMIT = 2.0f; // 制限時間の半分（秒）
}

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
	bool GetDebugViewFps() const       { return _d_view_fps; }
	void SetDebugViewCollsion(bool d) { this->_d_view_collision = d; }
	void SetDebugUseCollision(bool d) { this->_d_use_collision = d; }
	void SetDebugViewCameraInfo(bool d) { this->_d_view_camera_info = d; }
	void SetDebugViewShadowMap(bool d) { this->_d_view_shadow_map = d; }
	void SetCameraControlMode(bool b) {_bCameraControlMode = b;}
	bool GetCameraControlMode() const { return _bCameraControlMode; }
	void SetDebugViewFps(bool d) { this->_d_view_fps = d; }

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
	bool EscapeCollision(CharaBase* chara, ObjectBase* obj);				// キャラの回避処理
	bool CheckTanukiHeadCollision(PlayerTanuki* player, ObjectBase* obj, vec::Vec3& outHitPos);
	bool CharaToCharaCollision(CharaBase* c1, CharaBase* c2);				// キャラ同士の当たり判定処理
	bool PlayerToMakimonoCollision(PlayerBase* player, at::vspc<Makimono>& makimono);// プレイヤーと巻物の当たり判定処理
    // キャラと宝箱の当たり判定処理
	bool CharaToTreasureHitCollision(CharaBase* chara, const at::vspc<TreasureBase>& treasures);
	bool CharaToTreasureOpenCollision(PlayerBase* player, const at::vspc<TreasureBase>& treasures);
	bool CharaToTreasureRapidFireCollision(PlayerBase* player, const at::vspc<TreasureRapidFire>& treasures);
	// プレイヤーとゴールの当たり判定
	bool PlayerToGoalHitCollision(PlayerBase* player, Goal* goal);
	bool PlayerToTutorialCollision(PlayerBase* player,at::vspc<Tutorial> tutorial);

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
	bool RequestTransform(HenshinUi::Select select);

	void RequestTransformToMono();             // タヌキ -> モノ（巻物を消費して変身）
	void RequestTransformToHuman();            // タヌキ -> 人間（アニメあり）
	void RequestReturnToTanukiFromHuman();     // 人間表示時にタヌキへ即時戻す（UI選択）
	bool IsTransforming() const;
	bool IsLoadComplete() const { return _isLoadComplete; }
	void SetLoadComplete(bool b) { _isLoadComplete = b; }
	bool IsTransformRequested() const;
	bool IsShowingTanuki() const { return _bShowTanuki; }

	// カメラ操作公開API（メニューから呼び出すため）
	void CameraMoveBy(const vec::Vec3& delta);
	void CameraZoomTowardsTarget(float amount);
	bool DebugCameraControl();

	/// 演出関連
	bool TreasureOpeningCameraControl(); // 宝箱オープン演出カメラ制御
	bool EndCinematicCamera();           // 演出カメラ終了
	bool EndCinematicSequence(bool restoreToMainCamera); // 汎用的な演出カメラ終了関数
	bool DebugCinematicCameraControl();  // デバッグ用の演出カメラ制御
	// イントロ演出関数
	bool StartIntroSequence();      // イントロシーケンス開始
	bool ProcessIntroSequence();    // イントロシーケンスの更新
	bool EndIntroSequence();        // イントロシーケンス終了
	// ゲームクリア演出			    
	bool StartClearSequence();      // クリアシーケンス開始
	bool ProcessClearSequence();    // クリアシーケンスの更新
	bool EndClearSequence();        // クリアシーケンス終了
	bool StartPlayerRotation();     // プレイヤー回転演出開始
	bool ProcessPlayerRotation();   // プレイヤー回転演出の更新
	// ゲームオーバー演出
	bool StartGameOverSequence();   // ゲームオーバーシーケンス開始
	bool ProcessGameOverSequence(); // ゲームオーバーシーケンスの更新
	bool EndGameOverSequence();     // ゲームオーバーシーケンス終了


	// メニューから開始/終了されるカメラ編集（現在のカメラ状態を保存・復元）
	void StartCameraControlAndSave();
	void EndCameraControlAndRestore();

	// BGMチェンジ
	bool ChangeBGM();

	bool LoadStageData();

	auto GetShowTanuki() const { return _bShowTanuki; }

	// 取得数（UI等で使う想定）
	int GetTreasureTakenCount() const { return _treasureTakenCount; }
	// クリアに必要な宝箱数
	int GetTreasureRequiredCount() const { return _treasureRequiredCount; }

	// ゴール可能か？
	bool CanGoal() const{ return _treasureTakenCount >= _treasureRequiredCount; }

	// ステージリセット要求フラグのセッター（Process内でこのフラグをチェックしてリセット処理を行う）
	void RequestResetStage() { _requestResetStage = true; }

	void RequestNextStage()  { _requestNextStage  = true; }
	bool HasRenderOnce() const { return _hasRenderOnce; }

	// 現在のステージIDを取得
	std::string GetCurrentStageId() const { return _stageManager.GetCurrentStageId(); }

	// 初期ステージIDを設定
	void SetInitialStageId(const std::string& stageId);

	using enemygroup = at::umtt<std::string, at::vet<vec::Vec3>>;

	// 敵の生成関数
	void CreateEnemy(const nlohmann::json& object,
		const enemygroup& patrolGroup,
		const enemygroup& dogMovementArea,
		uint32_t& nextEnemyId,
		MapBase* map);

	at::spc<EnemySensor> CreateEnemySensor(float soundArea, MapBase* map);
	bool ProcessEnemyContainer(at::vspc<EnemyBase>& container, PlayerBase* player, bool isHumanForm, bool& anyDetected, bool& reEffect);

	const at::vec<EnemyBase*>& GetEnemiesInAttackRangees() const { return _enemiesInAttackRange; }

	auto GetPlayer() const { return _player; }
	auto GetPlayerTanuki() const { return _playerTanuki; }
	auto GetPlayerMono() const { return _playerMono; }
	bool IsShowingMono() const { return _showMonoPlayer; }
	void CancelRequestedTransform();
	void ShowHenshinPlayer(bool show) { if(_henshinUi) _henshinUi->SetShowPlayerUi(show); }
	void ShowHenshinMonoUi(bool show) { if(_henshinUi) _henshinUi->SetShowPlayerMonoUi(show); }


	void SavePlayer(PlayerBase* player);
	void ApplySaveData(const SaveData& data);
	bool PlayerToSavePointCollision(PlayerBase* player);
	void ResetEnemiesToInitialPositions();
	void ResetEnemyRoot();

protected:
	Camera* _camera;      // メインカメラ
	Camera* _savedCamera; // メニューから戻ったときにカメラ状態を復元するための一時的な保存用
	std::unique_ptr<CinematicCamera> _cinematicCamera;
    // メニュー開始前のカメラ状態を保存するためのメンバ
	vec::Vec3 _savedCamPos;    // カメラ位置
	vec::Vec3 _savedCamTarget; // カメラターゲット 
	bool _hasSavedCameraState;  
	bool _useCinematicCamera;  // 演出カメラ切り替えフラグ
	// デバッグ用演出カメラ制御用変数
	bool _debugF1KeyPressed = false; // F1キーの連続入力防止用
	bool _debugZoomActive   = false; // ズーム演出が実行中かどうか
	bool _debugF2KeyPressed = false; // F2キーの連続入力防止用
	bool _debugShakeActive  = false; // カメラシェイク演出が実行中かどうか

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
	at::vspc<TreasureBase> _treasureBase;
	at::vspc<Treasure>     _treasure;
	at::vspc<TreasureRapidFire> _treasureRapidFire;
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
	at::spc<CounterUi> _counterUi;
	at::spc<TreasureUi> _treasureUi;
	at::spc<AttackUi> _attackUi;
	at::spc<TreasureOpenUi> _treasureOpenUi;
	at::spc<DashUi> _dashUi;
	at::spc<StunEffect> _stunEffect;

	// エフェクト
	at::vspc<EffectBase> _effectBase;
	at::spc<TreasureEffect> _treasureEffect;
	// オブジェクトサーバー
	class ObjectServer* _objectServer;

	at::spc<HensinEffect> _hensinEffect;
	at::spc<WalkEffect> _walkEffect;
	at::spc<FindEffect> _findEffect;
	at::spc<HatenaEffect> _hatenaEffect;
	at::spc<DoyaEffect> _doyaEffect;
	at::spc<TreasureopenEffect> _TreasureOpenEffect;
	at::spc<NakiEffect> _nakiEffect;
	at::spc<ShirimochiEffect> _shirimochiEffect;
	at::spc<SavePointEffect> _savePointEffect;
	at::spc<MakimonoGetEffect> _makimonoGetEffect;
	at::spc<GoalEffect> _goalEffect;

	at::spc<SoundServer3D> _sound3D;
	soundserver::SoundItemBase* _soundFinish;

	at::vspc<SavePoint> _savePoint;
	SavePoint* _lastSavedPoint;

	// チュートリアル
	at::vspc<Tutorial> _tutorial;
	// デバッグ用
	bool _d_view_collision;
	bool _d_use_collision;
	bool _d_view_camera_info;
	bool _d_view_shadow_map;
	bool _d_view_fps;
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
	std::string _initialStageId; //

	bool _requestedTransformToMono = false;      // タヌキ -> モノ 要求
	bool _requestedTransformToHuman = false;     // タヌキ -> 人間 要求（アニメ）
	bool _requestedReturnToTanuki = false;       // 人間 -> タヌキ（即時）要求

	at::vec<EnemyBase*> _enemiesInAttackRange; // 攻撃範囲内の敵のリスト

	// 宝箱ごとの進行度を管理するマップを追加
	at::umtt<TreasureBase*, float> _treasureProgressMap;
	TreasureBase* _currentOpeningTreasure = nullptr; // 現在開けている宝箱

private:
	void RenderShadowCastersFromModeGame(); // シャドウキャスターの描画（ModeGameから呼び出す）
	// イントロ演出用
	bool  _isIntroActive;	   // イントロ演出が有効か
	bool  _introButtonPressed; // イントロ中にボタンが押されたか
	float _introTimer;		   // イントロの経過時間
	static constexpr float INTRO_DURATION = 3.0f; // イントロの総時間（秒）

	// イントロのフェーズを管理するための列挙型と変数
	enum class IntroPhase
	{
		RotateForward, 
		RotateBackward,
		Zoom,
		Done
	};
	IntroPhase _introPhase; // イントロのフェーズ管理用変数

	// ゲームクリア演出
	bool _isGameClearCinematicActive;					    // ゲームクリア演出が有効か
	float _clearCinematicTimer;							    // クリア演出の経過時間
	
	// ゲームオーバー演出
	bool  _isGameOverCinematicActive; // ゲームオーバー演出が有効か
	float _gameOverCinematicTimer;    // ゲームオーバー演出の経過時間
	int   _gameOverSequencePhase;     // ゲームオーバー演出のフェーズ管理用変数
	int   _gameOverDimAlpha;		  // ゲームオーバー演出の暗転アルファ値

	// プレイヤー回転演出用
	bool _isPlayerRotating = false;		  // プレイヤー回転演出が有効か
	float _playerRotationTimer = 0.0f;    // プレイヤー回転演出の経過時間
	float _playerRotationDuration = 2.0f; // プレイヤーが回転する演出の時間（秒）
	float _playerInitialRotation = 0.0f;  // プレイヤー回転演出開始時のプレイヤーの向き（角度）
	float _playerTargetRotation = 0.0f;   // プレイヤー回転演出の目標の向き（角度）
	int _clearSequencePhase = 0;		  // クリア演出のフェーズ管理用変数
	float progress;
	
	SaveData _saveData; // セーブデータのインスタンス
};

