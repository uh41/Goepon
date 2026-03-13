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
#include "enemybase.h"
#include "treasurebase.h"
#include "MapBase.h"
#include "uibase.h"
#include "effectbase.h"
#include "ObjectServer.h"
#include "soundserver3D.h"
#include "ModeGoalConfirm.h"
#include "modegameload.h"
#include "StageManager.h"
#include "ObjectFactoy.h"

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
	bool GetDebugViewFps() const { return _d_view_fps; }
	void SetDebugViewCollsion(bool d) { this->_d_view_collision = d; }
	void SetDebugUseCollision(bool d) { this->_d_use_collision = d; }
	void SetDebugViewCameraInfo(bool d) { this->_d_view_camera_info = d; }
	void SetDebugViewShadowMap(bool d) { this->_d_view_shadow_map = d; }
	void SetCameraControlMode(bool b) { _bCameraControlMode = b; }
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
	bool EscapeCollision(CharaBase* chara, ObjectBase* obj);
	bool CheckTanukiHeadCollision(PlayerBase* player, ObjectBase* obj, vec::Vec3& outHitPos);
	bool CharaToCharaCollision(CharaBase* c1, CharaBase* c2);
	bool PlayerToMakimonoCollision(PlayerBase* player, at::vspc<CharaBase>& makimono);
	bool CharaToTreasureHitCollision(CharaBase* chara, const at::vspc<TreasureBase>& treasures);
	bool CharaToTreasureOpenCollision(PlayerBase* player, const at::vspc<TreasureBase>& treasures);
	bool CharaToTreasureRapidFireCollision(PlayerBase* player, const at::vspc<TreasureBase>& treasures);
	bool PlayerToGoalHitCollision(PlayerBase* player, ObjectBase* goal);
	bool PlayerToTutorialCollision(PlayerBase* player, at::vspc<CharaBase> tutorial);

	// 索敵範囲の当たり判定
	bool IsPlayerAttack(PlayerBase* player, at::vspc<EnemyBase>& enemy);

	// デバック関数
	bool DebugInitialize();
	bool DebugProcess();
	bool DebugRender();

	// カメラ情報
	bool CameraInfoInitialize();
	bool PlayerCameraInfo(PlayerBase* player);

	// オブジェクト関数
	bool ObjectInitialize();
	bool ObjectProcess();
	bool ObjectRender();

	// プレイヤー変身関数
	bool PlayerTransform();
	bool PlayerTransformToTanuki(bool player);
	bool RequestTransform(HenshinUi::Select select);

	void RequestTransformToMono();
	void RequestTransformToHuman();
	void RequestReturnToTanukiFromHuman();
	bool IsTransforming() const;
	bool IsLoadComplete() const { return _isLoadComplete; }
	void SetLoadComplete(bool b) { _isLoadComplete = b; }
	bool IsTransformRequested() const;
	bool IsShowingTanuki() const { return _bShowTanuki; }

	// 影関数
	bool ShadowInitialize();

	// カメラ操作公開API（メニューから呼び出すため）
	void CameraMoveBy(const vec::Vec3& delta);
	void CameraZoomTowardsTarget(float amount);
	bool DebugCameraControl();

	/// 演出関連
	bool TreasureOpeningCameraControl();
	bool EndCinematicCamera();
	bool EndCinematicSequence(bool restoreToMainCamera);
	bool DebugCinematicCameraControl();
	bool StartIntroSequence();
	bool ProcessIntroSequence();
	bool EndIntroSequence();
	bool StartClearSequence();
	bool ProcessClearSequence();
	bool EndClearSequence();
	bool StartPlayerRotation();
	bool ProcessPlayerRotation();

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
	bool CanGoal() const { return _treasureTakenCount >= _treasureRequiredCount; }

	// ステージリセット要求フラグのセッター（Process内でこのフラグをチェックしてリセット処理を行う）
	void RequestResetStage() { _requestResetStage = true; }

	void RequestNextStage() { _requestNextStage = true; }
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

	at::spc<EnemyBase> CreateEnemySensor(float soundArea, MapBase* map);

	const at::vec<EnemyBase*>& GetEnemiesInAttackRangees() const { return _enemiesInAttackRange; }

	auto GetPlayer() const { return _player; }
	auto GetPlayerTanuki() const { return _playerTanuki; }
	auto GetPlayerMono() const { return _playerMono; }

protected:
	Camera* _camera;
	Camera* _originalCamera;
	std::unique_ptr<CinematicCamera> _cinematicCamera;
	vec::Vec3 _savedCamPos;
	vec::Vec3 _savedCamTarget;
	bool _hasSavedCameraState;
	bool _useCinematicCamera;
	bool _debugF1KeyPressed = false;
	bool _debugZoomActive = false;

	bool _hasRenderOnce;
	bool _requestResetStage;
	bool _requestNextStage;

	// キャラクタ管理
	at::vspc<CharaBase> _chara;
	at::vspc<ObjectBase> _object;
	at::vspc<PlayerBase> _playerBase;
	at::vspc<EnemyBase> _enemyBase;
	at::spc<PlayerBase> _player;
	at::spc<PlayerBase> _playerTanuki;
	at::spc<PlayerBase> _playerMono;
	at::vspc<TreasureBase> _treasureBase;
	at::vspc<TreasureBase> _treasure;
	at::vspc<TreasureBase> _treasureRapidFire;
	at::vspc<CharaBase> _makimono;
	at::vspc<ObjectBase> _mapBase;
	at::spc<ObjectBase> _map1;
	at::spc<ObjectBase> _cube;
	at::spc<ObjectBase> _goal;
	at::vspc<EnemyBase> _enemy;
	at::vspc<EnemyBase> _enemyMove;
	at::vspc<EnemyBase> _enemyDog;
	at::vspc<UiBase> _uiBase;
	at::spc<UiBase> _uiHp;
	at::spc<UiBase> _uiMakimono;
	at::spc<UiBase> _henshinUi;
	at::spc<UiBase> _counterUi;
	at::spc<UiBase> _treasureUi;
	at::spc<UiBase> _attackUi;
	at::spc<UiBase> _treasureOpenUi;
	at::spc<UiBase> _dashUi;
	at::spc<EffectBase> _stunEffect;
	at::spc<UiBase> _configUi;

	at::vspc<CharaBase> _charaShadow;

	at::vspc<EffectBase> _effectBase;
	at::spc<EffectBase> _treasureEffect;
	class ObjectServer* _objectServer;

	at::spc<EffectBase> _hensinEffect;
	at::spc<EffectBase> _walkEffect;
	at::spc<EffectBase> _findEffect;
	at::spc<EffectBase> _hatenaEffect;
	at::spc<EffectBase> _doyaEffect;
	at::spc<EffectBase> _nakiEffect;
	at::spc<EffectBase> _shirimochiEffect;

	at::spc<SoundServer3D> _sound3D;
	soundserver::SoundItemBase* _soundFinish;

	at::vspc<CharaBase> _tutorial;
	bool _d_view_collision;
	bool _d_use_collision;
	bool _d_view_camera_info;
	bool _d_view_shadow_map;
	bool _d_view_fps;
	bool _bCameraControlMode;

	bool anyDetected;
	bool _bTransCancel;
	bool _bResolveOnY;
	bool _bLandedOnUp;

	bool _bShowTanuki;
	bool _showMonoPlayer;
	bool _isTransformToHuman = false;
	bool _isTransformToMono = false;
	int _transformAnimId = -1;
	bool _isTanukiAttackPlaying = false;
	int _tanukiAttackAnimId = -1;

	bool _effekseerLaunched = false;

	at::spc<EnemyBase> _enemySensor;
	at::spc<soundserver::SoundServer> _soundServer;
	soundserver::SoundItemBase* _bgmInitialize;
	soundserver::SoundItemBase* _bgmChenge;

	bool _isChengeBgm;

	bool CheckAllDetections();

	int _treasureTakenCount = 0;
	float _treasureHoldSec = 0.0f;
	bool _treasureTakenThisTreasure = false;
	bool _isOpeningTreasure = false;
	int _treasureRequiredCount = 1;

	bool _showKnockdownMessage = false;
	float _knockdownMessageSec = 0.0f;

	bool _isGameClear;

	unsigned long _gameStartMs = 0;
	bool _gameClearShown = false;

	bool _goalConfirmOpened;
	ModeGoalConfirm::Result _goalConfirmResult;
	bool _notGoalFlag;
	bool UpdateGoalConfirm(PlayerBase* player);

	bool _changeTimeActive;
	float _changeTimeLimit;
	float _changeBlinkTimer;
	bool _changeBlinkVisible;
	float _changeBlinkInterval;

	bool _subMakimono = false;

	float _loadTimeMs = 0.0f;

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
	float _processSectorDetectionMs = 0.0f;

	bool _isLoadComplete;
	ModeGameLoad* _modeGameLoad;
	StageManager _stageManager;
	std::string _initialStageId;

	bool _requestedTransformToMono = false;
	bool _requestedTransformToHuman = false;
	bool _requestedReturnToTanuki = false;

	// オブジェクトファクトリ
	ObjectFactory _objectFactory;

	// 敵の方向グループ
	at::vec<EnemyBase*> _enemiesInAttackRange;

	void RequestTransformToMono();
	void RequestTransformToHuman();
	void RequestReturnToTanukiFromHuman();
	bool IsTransforming() const;
	bool IsTransformRequested() const;
};