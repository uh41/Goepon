/*********************************************************************/
// * \file   enemy.cpp
// * \brief  エネミークラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
//				vec::Vec3を使用するように修正　鈴木裕稀　2026/01/17
/*********************************************************************/

#include "enemymove.h"
#include "enemysensor.h"
#include "enemysoundsensor.h"
#include <cmath>

EnemyMove::EnemyMove()
{
	Initialize();
}


EnemyMove::~EnemyMove()
{

}

// 初期化
bool EnemyMove::Initialize()
{
	base::Initialize();

	_handle = MV1LoadModel("res/PoorEnemyMelee/bushi_0114taiki.mv1");
	_iAttachIndex = -1;
	// ステータスを「無し」に設定
	_status = STATUS::NONE;
	// 再生時間の初期化
	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;
	// 位置、向きの初期化
	//_vPos = vec3::VGet(100.0f, 0.0f, 0.0f);
	//_vDir = vec3::VGet(0.0f, 0.0f, -1.0f);// キャラモデルはデフォルトで-Z方向を向いている
	// 腰位置の設定
	_fColSubY = 100.0f;
	// コリジョン半径の設定
	_fCollisionR = 30.0f;
	_fCollisionWeight = 10.0f;

	_fHp = 30.0f;

	// 初期位置と向きを保存
	//_initialPosition = _vPos;
	//_initialDirection = _vDir;

	// センサー関連の初期化
	_detectedPlayer = false;					// プレイヤー検出フラグの初期化
	_playerPos = vec3::VGet(0.0f, 0.0f, 0.0f);	// プレイヤー位置の初期化
	_rotationSpeed = 0.5f;						// 回転速度（調整可能）

	// 移動関連の初期化
	_moveSpeed = 5.0f;								// 移動速度（調整可能）
	_targetPosition = vec3::VGet(0.0f, 0.0f, 0.0f);	// 目標位置の初期化
	_isMoving = false;								// 移動中フラグの初期化

	// 初期位置に戻る機能の初期化
	_isReturningToInitialPos = false;	// 初期位置に戻り中フラグの初期化
	_returnSpeed = 1.5f;				// 初期位置に戻る速度（追跡より少し遅め）

	// テレポート関連の初期化
	_waitingForTeleport = false;
	_teleportTimer = 0.0f;

	// YouDiedメッセージ関連の初期化
	_showYouDiedMessage = false;
	_youDiedMessageTimer = 0.0f;

	// 敵の向き変更タイマーの初期化
	DirChangeTimer = DirChangeInterval;

	_patroll = std::make_shared<MovePointControll>();
	_isPatroll = false;
	_patrolSpeed = 5.0f;
	_patrolIndex = 0;
	_savePatrolIndex = 0;

	_hasSavePoint = false;

	_isPatrolWaiting = false;
	_patrolWaitTimer = 0.0f;
	_patrolWaitDuration = 2.0f; // 到着時にその場で視線を変える時間（秒）
	_patrolWaitDir = vec3::VGet(0.0f, 0.0f, 0.0f);

	return true;
}

// 巡回ポイントの設定
void EnemyMove::SetPatrolPoint(const at::vet<vec::Vec3>& point)
{
	if(point.size() < 2)
	{
		return; // 2点未満なら設定しない
	}

	_patroll->SetMovePoint(point);
	_isPatroll = true;
	_patrolIndex = 0;
}

// 巡回処理
void EnemyMove::ProcessPatrol()
{
	if (!_isPatroll || !_patroll->IsValid())
	{
		return;
	}

	// 音源へ移動中は巡回処理で向きを上書きしない
	if (_isMovingToSound)
	{
		return;
	}

	// 待機中の更新（到着後その場で停止する処理）
	if (_isPatrolWaiting)
	{
		const float dt = 1.0f / 60.0f; // 60FPS 前提
		_patrolWaitTimer -= dt;
		// 待機中は視線を変えない（_vDir を上書きしない）
		if (_patrolWaitTimer <= 0.0f)
		{
			// 待機終了 → 次の巡回ポイントへ進める
			_isPatrolWaiting = false;
			_patroll->MoveToNextPoint();
			_patrolIndex = _patroll->GetMovePointIndex();
			// 続行のため目標を更新（このフレームで移動させる）
		}
		else
		{
			// 待機中は他の処理を行わない
			return;
		}
	}

	// 現在の目標を取得
	vec::Vec3 target = _patroll->GetTargetPoint();

	// 移動先ベクトル（水平のみ）
	vec::Vec3 toTarget = vec3::VSub(target, _vPos);
	toTarget.y = 0.0f; // 水平方向のみ
	float dist = vec3::VSize(toTarget);

	// 到着判定（移動前）
	const float reachThreshold = 15.0f;
	// NOTE:
	// 到着判定は「距離が閾値以下」で到着とみなすべきなので <= に変更。
	// また、MoveToNextPoint() は MovePointControll 側でインデックスを
	// 進めるため、ここで別途 +1 するべきではない。
	if (dist <= reachThreshold)
	{
		// 到着したらすぐ次へ進まず、その場で停止（視線を変えない）して待機する
		_patrolWaitDir = _vDir; // 保持しておくが、待機中は _vDir を変更しない

		// 待機開始
		_patrolWaitTimer = _patrolWaitDuration;
		_isPatrolWaiting = true;

		// その場で停止して待つため、移動処理はここで終了
		return;
	}

	// 目標に向かって移動（オーバーシュート防止）
	if (dist > 0.01f)
	{
		// 1フレームでの移動量（step）
		float step = _patrolSpeed;
		// 残距離より大きければクランプ
		if (step > dist) step = dist;

		vec::Vec3 dir = vec3::VScale(vec3::VNorm(toTarget), step);
		_vPos = vec3::VAdd(_vPos, dir);
		_vDir = vec3::VNorm(toTarget); // 目標方向を向く

		// 移動後の到着判定（念のため）
		vec::Vec3 afterToTarget = vec3::VSub(target, _vPos);
		afterToTarget.y = 0.0f;
		if (afterToTarget.LengthSquare() <= (reachThreshold * reachThreshold))
		{
			// 移動で到達した場合も、その場で停止（視線は変えない）して待機する
			_patrolWaitDir = _vDir;

			_patrolWaitTimer = _patrolWaitDuration;
			_isPatrolWaiting = true;
			// 待機に入るため移動後の追加処理は行わない
		}
	}
}

void EnemyMove::CaptureInitialTransform()
{
	_initialPosition = _vPos;
	_initialDirection = _vDir;
}

// 終了
bool EnemyMove::Terminate()
{
	base::Terminate();
	return true;
}

// テレポート状態のリセット
void EnemyMove::ResetTeleport()
{
	_waitingForTeleport = false;
	_teleportTimer = 0.0f;
}

// EnemySensorを設定
void EnemyMove::SetEnemySensor(std::shared_ptr<EnemySensor> sensor)
{
	_enemySensor = sensor;
}

// プレイヤーが検出範囲外になった時の処理
void EnemyMove::OnPlayerLost()
{
	_detectedPlayer = false;

	StartReturningToInitialPosition();
}

void EnemyMove::ProcessReturnToPatrolPoint()
{
	if(!_isReturningToInitialPos)
	{
		return;
	}

	// テレポート待機中の処理（床がないためテレポート待ち）
	if(_waitingForTeleport)
	{
		// カウントダウン
		_teleportTimer -= 1.0f / 60.0f; // 60FPS 前提
		if(_teleportTimer <= 0.0f)
		{
			// タイマー経過でセーブポイントへ瞬間移動
			_effect->PlayEffect(_vPos);
			_vPos = _savePoint;
			_vDir = _initialDirection; // 向きは初期向きに戻す（必要なら変更可）
			_isReturningToInitialPos = false;
			_waitingForTeleport = false;
			_teleportTimer = 0.0f;
			_effect->PlayEffect(_vPos);

			// 巡回インデックス復帰
			_patroll->SetMovePointIndex(_savePatrolIndex);
			_patrolIndex = _savePatrolIndex;
			_isPatroll = true;

			_hasSavePoint = false;
		}
		return;
	}

	vec::Vec3 target = _savePoint;
	vec::Vec3 toTarget = vec3::VSub(target, _vPos);
	toTarget.y = 0.0f; // 水平方向のみ

	float distSq = toTarget.LengthSquare();
	float threshold = 20.0f; // 到着判定の閾値

	if(distSq < (threshold * threshold))
	{
		_isReturningToInitialPos = false;
		_patroll->SetMovePointIndex(_savePatrolIndex);
		_patrolIndex = _savePatrolIndex;
		_isPatroll = true;
		_hasSavePoint = false;
		return;
	}

	// 帰還移動
	float dist = std::sqrt(distSq);
	if(dist > 0.01f)
	{
		vec::Vec3 dir = vec3::VScale(vec3::VNorm(toTarget), _returnSpeed);
		vec::Vec3 newPos = vec3::VAdd(_vPos, dir);

		// 移動先に床が存在するか確認
		if(CheckFloorExistence(newPos))
		{
			_vPos = newPos;
			_vDir = vec3::VNorm(toTarget); // 目標方向を向く
		}
		else
		{
			// 床がない → テレポート待機を開始
			_waitingForTeleport = true;
			_teleportTimer = TELEPORT_WAIT_TIME;
			// 状態は待機にしてアニメ等を止める（Process 内で反映される）
		}
	}

}

// 初期位置に戻る処理を開始
void EnemyMove::StartReturningToInitialPosition()
{
	// 現在の巡回インデックスを保存（復帰時に使う）
	_savePatrolIndex = _patrolIndex;

	// 戻る目標点を決定：
	// 巡回ルートが有効なら現在の巡回ターゲットへ、さもなくば初期位置へ戻す
	if(!_hasSavePoint)
	{
		if(_patroll && _patroll->IsValid())
		{
			_savePoint = _patroll->GetTargetPoint();
		}
		else
		{
			_savePoint = _initialPosition;
		}
	}

	_isPatroll = false;
	_isReturningToInitialPos = true;

	// 念のためテレポート状態をリセットしておく
	ResetTeleport();
}

void EnemyMove::OnDamageStart()
{
	_savePatrolIndex = _patrolIndex;
	_isPatroll = false;
	_hasSavePoint = true;
}

void EnemyMove::OnDamageEnd()
{
	if(_patroll && _patroll->IsValid())
	{
		_patroll->SetMovePointIndex(_savePatrolIndex);
		_patrolIndex = _savePatrolIndex;
		_isPatroll = true;
		_hasSavePoint = false;
	}
}

// 計算処理
bool EnemyMove::Process()
{
	base::Process();

	CharaBase::STATUS old_status = _status;

	// ステータスがNONEの場合、WAITに設定
	if(_status == STATUS::NONE)
	{
		_status = STATUS::WAIT;
	}

	// EnemySensorがあれば、そのセンサーの位置と向きを自分の位置に同期
	if(_enemySensor)
	{
		_enemySensor->SetPos(_vPos);
		_enemySensor->SetDir(_vDir);
	}

	// 音検知タイマーの更新（音検知が有効な場合）
	if(_soundDetectionActive)
	{
		const float dt = 1.0f / 60.0f; // 60FPS想定
		_soundDetectionTimer += dt;

		// 指定時間を越えたら初期位置への帰還を開始
		if(_soundDetectionTimer >= SOUND_RETURN_TIME)
		{
			_soundDetectionActive = false;
			_soundDetectionTimer = 0.0f;
			_isMovingToSound = false;
			_waitingAtSound = false;

			StartReturningToInitialPosition();
		}
	}

	// 音源到達後の待機中はWAITステータス（プレイヤー検知で割り込み可）
	if(_waitingAtSound)
	{
		const float dt = 1.0f / 60.0f; // 60FPS想定
		_soundWaitTimer -= dt;

		if(_soundWaitTimer <= 0.0f)
		{
			_waitingAtSound = false;
			_soundWaitTimer = 0.0f;

			// 次に検知し続けないように（同じ検知情報で再度 Moving に入るのを防止）
			_soundDetectionActive = false;
			_soundDetectionTimer = 0.0f;

			// 保存してある巡回ルート（ターゲット座標）へ一旦戻してから巡回再開
			// _savePoint / _savePatrolIndex / _hasSavePoint は検知開始時に保存済み
			if(_hasSavePoint)
			{
				_isReturningToInitialPos = true;
				_isPatroll = false;
				ResetTeleport();
			}
			else
			{
				// 念のため保険：保存が無ければ通常帰還（結果的に巡回へ戻る）
				StartReturningToInitialPosition();
			}
		}
	}

	// EnemySoundSensorから音の検知情報を取得
	if(_enemySoundSensor)
	{
		// 音検知情報を取得
		const auto& detectionInfo = _enemySoundSensor->GetDetectionInfo();
		if(detectionInfo.isDetected && detectionInfo.detectedSoundLevel == 5)
		{
			if(!_detectedPlayer && (!_enemySensor || !_enemySensor->IsChasing()))
			{
				// 巡回を止めて現在の巡回インデックスを保存
				_savePatrolIndex = _patrolIndex;
				_isPatroll = false;

				// 巡回ターゲット座標を保存（復帰時に使う）
				if(_patroll && _patroll->IsValid())
				{
					_savePoint = _patroll->GetTargetPoint();
				}
				else
				{
					_savePoint = _initialPosition;
				}
				_hasSavePoint = true;

				// テレポートは行わず、プレイヤー（音源）座標へ向けて移動する
				_isMovingToSound = true;
				_soundSourcePosition = detectionInfo.soundSourcePosition;

				// 音検知タイマー開始
				_soundDetectionActive = true;
				_soundDetectionTimer = 0.0f;

				// 初期位置への帰還待機は中断
				_waitingBeforeReturn = false;
				_returnWaitTimer = 0.0f;

				// 移動フラグを設定して UpdateMovingToSound が実際に移動を行えるようにする
				_waitingAtSound = false;
			}
		}
	}

	// 優先順位: 音の追跡 > 扇形の追跡 > 帰還 > 巡回
	if(_isMovingToSound || _waitingAtSound)
	{
		if(_waitingAtSound|| _isPatrolWaiting)
		{
			_status = STATUS::WAIT;
		}
		else
		{
			_status = STATUS::WALK;
			UpdateMovingToSound();
		}

		_isReturningToInitialPos = false;
		_isPatroll = false;
	}
	else if(_detectedPlayer || (_enemySensor && _enemySensor->IsChasing()))
	{
		_status = STATUS::WALK;
		UpdateChasing();
		_isReturningToInitialPos = false;
		_isPatroll = false;		// 巡回停止
	}
	else if(_isReturningToInitialPos)
	{
		if(_waitingForTeleport)
		{
			_status = STATUS::WAIT;
		}
		else
		{
			_status = STATUS::WALK;
		}
		ProcessReturnToPatrolPoint();
	}
	else if(_isPatroll)
	{
		_status = STATUS::WALK;
		ProcessPatrol();
	}
	else
	{
		_status = STATUS::WAIT;

		if(_enemySensor && !_enemySensor->IsChasing() && !IsAtInitialPosition())
		{
			StartReturningToInitialPosition();
		}
	}

	// プレイヤーを検出している場合、プレイヤーの方向に徐々に向く
	if(_detectedPlayer)
	{
		UpdateRotationToPlayer(); // 徐々に回転
		// または即座に向きたい場合は LookAtPlayer(); を使用
	}

	// ステータスが変わっていないか？
	if(old_status == _status)
	{
		//再生時間を進める
		_fPlayTime += 0.5f;
		// 再生時間をランダムに揺らがせる
		switch(_status)
		{
		case STATUS::WAIT:
		{
			_fPlayTime += (float)(rand() % 10) / 100.0f;// 0.00 ～ 0.09 の揺らぎ。積算するとずれが起きる
			break;
		}
		}
	}
	else
	{
		// アニメーションがアタッチされていたら、デタッチする
		if(_iAttachIndex != -1)
		{
			MV1DetachAnim(_handle, StCas<int>(_iAttachIndex));
			_iAttachIndex = -1;
		}
		// ステータスに応じたアニメーションをアタッチする
		switch(_status)
		{
		case STATUS::WAIT:
		{
			int animIndex = MV1GetAnimIndex(_handle, "taiki");
			if(animIndex != -1)
			{
				_iAttachIndex = StCas<float>(MV1AttachAnim(_handle, animIndex, -1, FALSE));
				if(_iAttachIndex != -1)
				{
					_fTotalTime = MV1GetAttachAnimTotalTime(_handle, StCas<int>(_iAttachIndex));
					_fPlayTime = (float)(rand() % 30); // 少しずらす
				}
			}
			break;
		}
		case STATUS::WALK:
		{
			int animIndex = MV1GetAnimIndex(_handle, "walk");
			if(animIndex != -1)
			{
				_iAttachIndex = StCas<float>(MV1AttachAnim(_handle, animIndex, -1, FALSE));
				if(_iAttachIndex != -1)
				{
					_fTotalTime = MV1GetAttachAnimTotalTime(_handle, StCas<int>(_iAttachIndex));
					_fPlayTime = (float)(rand() % 30); // 少しずらす
				}
			}
			break;
		}
		}
		// アタッチしたアニメーションの総再生時間を取得する
		if(_iAttachIndex != -1)
		{
			_fTotalTime = MV1GetAttachAnimTotalTime(_handle, StCas<int>(_iAttachIndex));
		}
		// 再生時間を初期化
		_fPlayTime = 0.0f;
		// 再生時間をランダムにずらす
		switch(_status)
		{
		case STATUS::WAIT:
		{
			_fPlayTime += rand() % 30; // 0 ～ 29 の揺らぎ
			break;
		}
		}
	}

	// 再生時間がアニメーションの総再生時間に達したら再生時間を0に戻す
	if(_fPlayTime >= _fTotalTime)
	{
		_fPlayTime = 0.0f;
	}

	// YouDiedメッセージのタイマー更新
	if(_showYouDiedMessage)
	{
		_youDiedMessageTimer -= 1.0f / 60.0f;

		// タイマーが0以下になったらメッセージ非表示
		if(_youDiedMessageTimer <= 0.0f)
		{
			_showYouDiedMessage = false;
		}
	}

	// 定期的に方向を90度変える処理
	DirChangeTimer -= 1.0f / 60.0f;

	// タイマーが0以下になったら方向を変える
	if(DirChangeTimer <= 0.0f)
	{
		// タイマーをリセット（15秒ごとに変更）
		DirChangeTimer = 15.0f;

		// プレイヤーを検出していない、かつ追跡中でもない、かつ初期位置に戻り中でもない場合のみ回転
		if(!_detectedPlayer && (!_enemySensor || !_enemySensor->IsChasing()) && !_isReturningToInitialPos && !_isMovingToSound)
		{
			// 回転先の方向を計算
			float currentAngle = atan2f(_vDir.x, _vDir.z);
			float newAngle = currentAngle + DX_PI_F / 2.0f;

			// 新しい向きベクトルを計算
			vec::Vec3 newDir;
			newDir.x = sin(newAngle);
			newDir.y = 0.0f;
			newDir.z = cos(newAngle);

			// 新しい方向に少し移動した位置で床の存在を確認
			vec::Vec3 testPos = vec3::VAdd(_vPos, vec3::VScale(newDir, _moveSpeed * 5.0f));

			// 床がある場合のみ回転を実行
			if(CheckFloorExistence(testPos))
			{
				_vDir = newDir;
			}
		}
	}
	return true;
}

// 描画処理
bool EnemyMove::Render()
{
	base::Render();
	// 再生時間をセット
	MV1SetAttachAnimTime(_handle, StCas<int>(_iAttachIndex), _fPlayTime);
	float vorty = atan2(_vDir.x * -1, _vDir.z * -1);// ���f�����W���łǂ��������Ă��邩�Ŏ����ς��(�����-z������Ă���ꍇ)

	MATRIX mRotY = MGetRotY(vorty);

	// 位置
	MV1SetPosition(_handle, DxlibConverter::VecToDxLib(_vPos));
	// 向きからY軸回転を算出
	MATRIX mRotZ = MGetRotZ(DX_PI_F * 0.5f); // -90�x�i�K�v�ɉ����ĕ����𔽓]�j

	MATRIX mTrans = MGetTranslate(DxlibConverter::VecToDxLib(_vPos));

	MATRIX mScale = MGetScale(VGet(1.7f, 1.7f, 1.7f));

	MATRIX m = MGetIdent();

	//m = MMult(m, mRotZ);
	m = MMult(m, mRotY);
	m = MMult(m, mScale);
	m = MMult(m, mTrans);

	MV1SetMatrix(_handle, m);

	// 描画
	MV1DrawModel(_handle);

	return true;
}