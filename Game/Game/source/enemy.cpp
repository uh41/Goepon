/*********************************************************************/
// * \file   enemy.cpp
// * \brief  エネミークラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
// *            vec::Vec3を使用するように修正　鈴木裕稀　2026/01/17
/*********************************************************************/

#include "enemy.h"
#include "enemysensor.h"

// 初期化
bool Enemy::Initialize()
{
	base::Initialize();

	_handle = MV1LoadModel(mv1::SK_busi_multimotion);
	_iAttachIndex = -1;
	// ステータスを「無し」に設定
	_status = STATUS::NONE;
	// 再生時間の初期化
	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;

	// 腰位置の設定
	_fColSubY = 100.0f;
	// コリジョン半径の設定
	_fCollisionR = 30.0f;
	_fCollisionWeight = 100.0f;

	_fHp = 30.0f;

	// センサー関連の初期化（共通は EnemyBase::Initialize で行われる）
	_rotationSpeed = 0.5f;						// 回転速度（調整可能）
	_moveSpeed = 7.2f;

	// 整合性のため他は base に委譲済み

	_attachAnimDamage = "okkake";
	_attachAnimStan = "bushi_tentou";
	_attachAnimGetUp = "idle";
	// タイマー初期化等は base にて行われている
	return true;
}

// 終了
bool Enemy::Terminate()
{
	base::Terminate();
	// モデルリソース解放など必要ならここで
	return true;
}

// 計算処理
bool Enemy::Process()
{
	// 共通処理（センサー同期、追跡・復帰の基礎）は EnemyBase::Process / UpdateChasing 等を使う
	base::Process();

	// Enemy 固有のふるまい：
	// - 基本は立ち止まって向きを変える（検知／追跡時は追跡処理を呼ぶ）
	// ※ 具体的なアニメや描画制御はここで行う

	// ダメージ中は Enemy 側の通常状態遷移・移動・通常アニメ更新を止める
	if (_isInvincible)
	{
		_status = STATUS::WAIT;
		return true;
	}

	CharaBase::STATUS old_status = _status;

	// ステータスがNONEの場合、WAITに設定
	if(_status == STATUS::NONE)
	{
		_status = STATUS::WAIT;
	}

	// 音検知タイマーの更新（音検知が有効な場合）
	if (_soundDetectionActive)
	{
		const float dt = 1.0f / 60.0f; // 60FPS想定
		_soundDetectionTimer += dt;

		// 10秒経過したら初期位置への帰還を開始
		if (_soundDetectionTimer >= SOUND_RETURN_TIME)
		{
			_soundDetectionActive = false;
			_soundDetectionTimer = 0.0f;
			_isMovingToSound = false;
			_waitingAtSound = false;

			ReturnInitialPos();
		}
	}

	// 音源到達後の待機処理
	if (_waitingAtSound)
	{
		// 待機中はWAITステータスに設定
		_status = STATUS::WAIT;

		// 待機中はプレイヤー検出されたら割り込み可能
		// この処理は下のEnemySensorの処理で行われる
	}

	// 音源への移動処理（プレイヤー検出より優先度は低い）
	if (_isMovingToSound && !_detectedPlayer && (!_enemySensor || !_enemySensor->IsChasing()))
	{
		UpdateMovingToSound();
	}

	// 検知終了後の待機処理（共通フラグを参照）
	if(_waitingReturn)
	{
		const float dt = 1.0f / 60.0f; // 60FPS想定
		_returnWaitTimer -= dt;

		if(_returnWaitTimer <= 0.0f)
		{
			_waitingReturn = false;

			// 初期位置へ戻る処理は base 実装を使う
			ReturnInitialPos();
		}

		_status = STATUS::WAIT;
	}
	else
	{
		// センサーがあれば位置・向きを同期して追跡処理（共通）を利用
		if(_enemySensor)
		{
			_enemySensor->SetPos(_vPos);
			_enemySensor->SetDir(_vDir);

			// UpdateChasing は EnemyBase 側で定義され、MoveTowardsTarget を呼びます。
			UpdateChasing();

			// センサーの状態に応じてステータスを設定
			if (_enemySensor->IsChasing())	// プレイヤーを追跡中
			{
				_status = STATUS::FOUND;
				_isReturning = false;

				// 音源への移動と待機を中断
				_isMovingToSound = false;
				_waitingAtSound = false;
				_soundWaitTimer = 0.0f;

				// 音検知タイマーをリセット
				_soundDetectionActive = false;
				_soundDetectionTimer = 0.0f;
			}
			else if (_waitingReturn)	// 検知終了後の待機処理
			{
				const float dt = 1.0f / 60.0f; // 60FPS想定
				_returnWaitTimer -= dt;

				if (_returnWaitTimer <= 0.0f)
				{
					_waitingReturn = false;

					// 初期位置へ戻る処理は base 実装を使う
					ReturnInitialPos();
				}

				_status = STATUS::WAIT;
			}
			else if (_isReturning)	// 初期位置に戻り中
			{
				if (_waitingTeleport) 
				{
					_status = STATUS::WAIT;
				}
				else 
				{
					_status = STATUS::WALK;
				}
				UpdateReturnInitialPos();	// 初期位置への帰還処理は base 実装を使う
			}
			else if (_isMovingToSound)	// 音源に向かって移動中
			{
				_status = STATUS::WALK;
			}
			else // 通常待機状態
			{
				_status = STATUS::WAIT;

				// 音源移動中や音源待機中は初期位置への帰還を開始しない
				if (!_enemySensor->IsChasing() && !IsAtInitialPos() && !_isMovingToSound && !_waitingAtSound)
				{
					ReturnInitialPos();
				}
			}
		}
	}

	// プレイヤー検出時は向くだけ（徐々に）／または追跡時は UpdateChasing が移動する
	if(_detectedPlayer)
	{
		// 共通実装を使用
		UpdateRotationToPlayer();
	}
	else if (_isReturning && !_waitingTeleport)
	{
		// 帰還中は目標方向へ徐々に向きを変える
		vec::Vec3 target = _initialPos;
		vec::Vec3 toTarget = vec3::VSub(target, _vPos);
		toTarget.y = 0.0f;

		if (toTarget.LengthSquare() > 0.01f)
		{
			vec::Vec3 targetDir = vec3::VNorm(toTarget);
			float rotSpeed = _rotationSpeed * 0.5f; // 帰還時は回転速度を少し遅めに
			_vDir.x += (targetDir.x - _vDir.x) * rotSpeed;
			_vDir.z += (targetDir.z - _vDir.z) * rotSpeed;
			_vDir.y = 0.0f;
			if (_vDir.LengthSquare() > 0.01f)
			{
				_vDir = vec3::VNorm(_vDir);
			}
		}
	}

	// --- アニメーション管理: PlayerTanuki と同様に AnimationManager を使うように変更 ---
	// アニメーション名のマッピング（Enemy用モデルの既存名に合わせる）
	auto GetAnimName = [](CharaBase::STATUS s) -> std::string
	{
		switch(s)
		{
		case CharaBase::STATUS::WAIT:
			return "idle";
		case CharaBase::STATUS::WALK:
			return "walk";
		case CharaBase::STATUS::FOUND:
			return "okkake";
		default:
			return std::string();
		}
	};

	// 再生状態が終わっていたら再生し直す（ループ想定）
	if(_animId != -1 && !AnimationManager::GetInstance()->IsPlaying(_animId))
	{
		_animId = -1;
		std::string anim_name = GetAnimName(_status);
		if(!anim_name.empty())
		{
			_animId = AnimationManager::GetInstance()->Play(_handle, anim_name, true);
			_fPlayTime = 0.0f;
			if(_animId != -1)
			{
				//_fTotalTime = AnimationManager::GetInstance()->GetTotalTime(_animId);
				AnimationManager::GetInstance()->SetTime(_animId, _fPlayTime);
			}
		}
	}

	// アニメーション時間・切り替え管理（Player と同様のロジック）
	if(old_status == _status)
	{
		float anim_speed = 0.5f;
		_fPlayTime += anim_speed;
		switch(_status)
		{
		case STATUS::WAIT:
			_fPlayTime += (float)(rand() % 10) / 100.0f;
			break;
		default:
			break;
		}
	}
	else
	{
		// ステータスが変化した場合は現在の再生を停止して新しいアニメを開始
		if(_animId != -1)
		{
			AnimationManager::GetInstance()->Stop(_animId);
			_animId = -1;
		}

		std::string anim_name = GetAnimName(_status);
		if(!anim_name.empty())
		{
			_animId = AnimationManager::GetInstance()->Play(_handle, anim_name, true);
			_fPlayTime = 0.0f;
			switch(_status)
			{
			case STATUS::WAIT:
				_fPlayTime += rand() % 30;
				break;
			default:
				break;
			}
			if(_animId != -1)
			{
				//_fTotalTime = AnimationManager::GetInstance()->GetTotalTime(_animId);
				AnimationManager::GetInstance()->SetTime(_animId, _fPlayTime);
			}
		}
	}

	if(_fPlayTime >= _fTotalTime)
	{
		_fPlayTime = 0.0f;
	}
	// --- アニメーション管理ここまで ---

	return true;
}

// 描画処理
bool Enemy::Render()
{
	base::Render();
	// 再生時間をセット（必要なら AnimationManager に反映）
	float vorty = atan2(_vDir.x * -1, _vDir.z * -1);

	MATRIX mRotY = MGetRotY(vorty);

	MV1SetPosition(_handle, DxlibConverter::VecToDxLib(_vPos));

	MATRIX mTrans = MGetTranslate(DxlibConverter::VecToDxLib(_vPos));
	MATRIX mScale = MGetScale(VGet(8.0f, 8.0f, 8.0f));

	MATRIX m = MGetIdent();
	m = MMult(m, mRotY);
	m = MMult(m, mScale);
	m = MMult(m, mTrans);

	MV1SetMatrix(_handle, m);
	MV1DrawModel(_handle);

	return true;
}