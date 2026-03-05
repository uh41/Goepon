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
#include "enemysoundmanager.h"
#include <cmath>

// 初期化
bool EnemyMove::Initialize()
{
	base::Initialize();

	_handle = MV1LoadModel("res/PoorEnemyMelee/busi_multimotion.mv1");
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
	_fCollisionWeight = 10.0f;

	_fHp = 30.0f;

	// センサー関連の初期化
	_detectedPlayer = false;					
	_playerPos = vec3::VGet(0.0f, 0.0f, 0.0f);	
	_rotationSpeed = 0.5f;						

	// 移動関連の初期化
	_moveSpeed = 8.25f;								
	_targetPos = vec3::VGet(0.0f, 0.0f, 0.0f);	
	_isMoving = false;								

	// 初期位置に戻る機能の初期化
	_isReturning = false;				
	_returnSpeed = 5.0f;				// 初期位置に戻る速度（追跡より少し遅め）

	// テレポート関連の初期化
	_waitingForTeleport = false;
	_teleportTimer = 0.0f;

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

void EnemyMove::SetPatrolWaitDirection(int id)
{
	_patrolWaitDir = DirIdToVec3(id);

	if(_patrolWaitDir.LengthSquare() > 0.01f)
	{
		_patrolWaitDir = vec3::VNorm(_patrolWaitDir);
		if(!IsStun())
		{
			_vDir = _patrolWaitDir; // 待機中の向きを即座に反映
		}
	}
}

void EnemyMove::SetPatrolPointInfo(const at::vec<ApplicationGlobal::PatrolPointInfo>& points)
{
	_patrolPointInfo.assign(points.begin(), points.end());

	at::vet<vec::Vec3> posList;
	posList.reserve(_patrolPointInfo.size());
	for(auto&& p : _patrolPointInfo)
	{
		posList.push_back(p.pos);
	}

	SetPatrolPoint(posList);
}

// 巡回処理
void EnemyMove::ProcessPatrol()
{
	if(!_isPatroll || !_patroll->IsValid())
	{
		return;
	}

	if(_isMovingToSound)
	{
		return;
	}

	// 待機中
	if(_isPatrolWaiting)
	{
		const float dt = 1.0f / 60.0f;
		_patrolWaitTimer -= dt;

		// 待機中は指定された向きを維持
		if(_patrolWaitDir.LengthSquare() > 0.01f && !IsStun())
		{
			_vDir = vec3::VNorm(_patrolWaitDir);
		}

		if(_patrolWaitTimer <= 0.0f)
		{
			_isPatrolWaiting = false;
			_patroll->MoveToNextPoint();
			_patrolIndex = _patroll->GetMovePointIndex();

			// 次のポイントの direction を取得して向きを設定
			const int nextIdx = _patroll->GetMovePointIndex();
			if(nextIdx >= 0 && nextIdx < StCas<int>(_patrolPointInfo.size()))
			{
				int nextDirId = _patrolPointInfo[nextIdx].id;
				if(nextDirId > 0)
				{
					// 次のポイントに direction が指定されていれば、その向きへ変更
					_patrolWaitDir = DirIdToVec3(nextDirId);
					if(_patrolWaitDir.LengthSquare() > 0.0001f && !IsStun())
					{
						_patrolWaitDir = vec3::VNorm(_patrolWaitDir);
						_vDir = _patrolWaitDir;
					}
				}
				else
				{
					// direction が指定されていない場合は次のターゲット方向へ
					vec::Vec3 nextTarget = _patroll->GetTargetPoint();
					vec::Vec3 toNextTarget = vec3::VSub(nextTarget, _vPos);
					toNextTarget.y = 0.0f;
					if(toNextTarget.LengthSquare() > 0.0001f)
					{
						_patrolWaitDir = vec3::VNorm(toNextTarget);
						if(!IsStun())
						{
							_vDir = _patrolWaitDir;
						}
					}
					else
					{
						_patrolWaitDir = vec3::VGet(0.0f, 0.0f, 0.0f);
					}
				}
			}
			else
			{
				// インデックスが範囲外の場合は向きをリセット
				_patrolWaitDir = vec3::VGet(0.0f, 0.0f, 0.0f);
			}
		}
		else
		{
			// 待機中は他の処理を行わない
			_status = STATUS::WAIT;
			return;
		}
	}

	vec::Vec3 target = _patroll->GetTargetPoint();

	vec::Vec3 toTarget = vec3::VSub(target, _vPos);
	toTarget.y = 0.0f;
	float dist = vec3::VSize(toTarget);

	const float reachThreshold = 15.0f;
	if(dist <= reachThreshold)
	{
		// 到着したポイント index に対応する direction / waittime を採用
		int dirId = 0;
		float waitSec = 0.0f;
		const int idx = _patroll->GetMovePointIndex();
		if(idx >= 0 && idx < StCas<int>(_patrolPointInfo.size()))
		{
			dirId = _patrolPointInfo[idx].id;
			waitSec = _patrolPointInfo[idx].waitTime;
		}

		// direction があれば向きを変える
		if(dirId > 0)
		{
			_patrolWaitDir = DirIdToVec3(dirId);
			if(_patrolWaitDir.LengthSquare() > 0.0001f && !IsStun())
			{
				_patrolWaitDir = vec3::VNorm(_patrolWaitDir);
				_vDir = _patrolWaitDir; // 到着時に即座に向きを変更
			}
		}
		else
		{
			// direction が指定されていない場合は移動方向を維持
			if(toTarget.LengthSquare() > 0.0001f)
			{
				_patrolWaitDir = vec3::VNorm(toTarget);
			}
			else
			{
				_patrolWaitDir = _vDir;
			}
		}

		// waittime があればその秒数止まる
		if(waitSec > 0.0f)
		{
			_patrolWaitDuration = waitSec;
		}

		// 待機開始
		_status = STATUS::WAIT;
		_patrolWaitTimer = _patrolWaitDuration;
		_isPatrolWaiting = true;
		return;
	}

	// 通常移動
	if(dist > 0.01f)
	{
		float step = _patrolSpeed;
		if(step > dist) step = dist;

		vec::Vec3 dir = vec3::VScale(vec3::VNorm(toTarget), step);
		_vPos = vec3::VAdd(_vPos, dir);

		// 移動中は移動方向を向く
		if(!IsStun())
		{
			_vDir = vec3::VNorm(toTarget);
		}

		vec::Vec3 afterToTarget = vec3::VSub(target, _vPos);
		afterToTarget.y = 0.0f;
		if(afterToTarget.LengthSquare() <= (reachThreshold * reachThreshold))
		{
			_patrolWaitTimer = _patrolWaitDuration;
			_isPatrolWaiting = true;
		}
	}
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
	if(IsStun())
	{
		return; // 無敵状態なら見失い処理を行わない
	}

	_detectedPlayer = false;

	ReturnInitialPos();
}

// 初期位置に戻る処理の更新
void EnemyMove::ProcessReturnToPatrolPoint()
{
	if(!_isReturning)
	{
		return;
	}

	if(IsStun())
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
			_vDir = _initialDir; // 向きは初期向きに戻す（必要なら変更可）
			_isReturning = false;
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
		_isReturning = false;
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
void EnemyMove::ReturnInitialPos()
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
			_savePoint = _initialPos;
		}
	}

	_isPatroll = false;
	_isReturning = true;

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

// 敵サウンドマネージャーから音源に向かって移動する処理を開始
void EnemyMove::StartMoveToSound(const vec::Vec3& soundPos, int soundLevel)
{
	// スタン中は音に反応しない
	if (IsStun())
	{
		return;
	}

	// 追跡/プレイヤー検出中は音より優先（既存方針に合わせる）
	if(_detectedPlayer || (_enemySensor && _enemySensor->IsChasing()))
	{
		return;
	}

	// レベルフィルタ（今のロジックに合わせて 5 のみ反応）
	if(soundLevel != 5)
	{
		return;
	}

	// 巡回を止めて現在の巡回インデックスを保存
	_savePatrolIndex = _patrolIndex;
	_isPatroll = false;

	// 巡回待機状態をリセット
	_isPatrolWaiting = false;
	_patrolWaitTimer = 0.0f;

	// 巡回ターゲット座標を保存（復帰時に使う）
	if(_patroll && _patroll->IsValid())
	{
		_savePoint = _patroll->GetTargetPoint();
	}
	else
	{
		_savePoint = _initialPos;
	}
	_hasSavePoint = true;

	// 音源へ向けて移動開始
	_isMovingToSound = true;
	_soundSourcePosition = soundPos;

	// 音検知タイマー開始
	_soundDetectionActive = true;
	_soundDetectionTimer = 0.0f;

	// 初期位置への帰還待機は中断
	_waitingBeforeReturn = false;
	_returnWaitTimer = 0.0f;

	_waitingAtSound = false;
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

			ReturnInitialPos();
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
				_isReturning = true;
				_isPatroll = false;
				ResetTeleport();
			}
			else
			{
				// 念のため保険：保存が無ければ通常帰還（結果的に巡回へ戻る）
				ReturnInitialPos();
			}
		}
	}

	// 優先順位: 音の追跡 > 扇形の追跡 > 帰還 > 巡回
	if(_isMovingToSound || _waitingAtSound)
	{
		if(_waitingAtSound)
		{
			_status = STATUS::WAIT;
		}
		else
		{
			_status = STATUS::WALK;
			UpdateMovingToSound();
		}

		_isReturning = false;
		_isPatroll = false;
	}
	else if (_enemySensor && _enemySensor->IsChasing())	// プレイヤーを検出している場合
	{
		_status = STATUS::FOUND;
		UpdateChasing();
		_isReturning = false;
		_isPatroll = false;		// 巡回停止
	}
	else if (_isReturning)	// 初期位置に戻り中
	{
		if(_waitingForTeleport)
		{
			_status = STATUS::WAIT;
		}
		else
		{
			_status = STATUS::WALK;
		}
		ProcessReturnToPatrolPoint();	// 初期位置への帰還処理
	}
	else if (_isPatroll)	// 巡回中
	{
		_status = STATUS::WALK;
		ProcessPatrol();
	}
	else // どの状態にも当てはまらない場合は待機
	{
		_status = STATUS::WAIT;

		if(_enemySensor && !_enemySensor->IsChasing() && !IsAtInitialPos() && !IsStun())
		{
			ReturnInitialPos();
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
			int animIndex = MV1GetAnimIndex(_handle, "bushi_idle");
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
			int animIndex = MV1GetAnimIndex(_handle, "kari_walk");
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
		case STATUS::FOUND:
		{
			int animIndex = MV1GetAnimIndex(_handle, "bushi_okkake");
			if (animIndex != -1)
			{
				_iAttachIndex = StCas<float>(MV1AttachAnim(_handle, animIndex, -1, FALSE));
				if (_iAttachIndex != -1)
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

	return true;
}

// 描画処理
bool EnemyMove::Render()
{
	base::Render();
	// 再生時間をセット
	MV1SetAttachAnimTime(_handle, StCas<int>(_iAttachIndex), _fPlayTime);
	float vorty = atan2(_vDir.x * -1, _vDir.z * -1);

	MATRIX mRotY = MGetRotY(vorty);

	// 位置
	MV1SetPosition(_handle, DxlibConverter::VecToDxLib(_vPos));
	// 向きからY軸回転を算出
	MATRIX mRotZ = MGetRotZ(DX_PI_F * 0.5f); 
	MATRIX mTrans = MGetTranslate(DxlibConverter::VecToDxLib(_vPos));
	MATRIX mScale = MGetScale(VGet(8.0f, 8.0f, 8.0f));
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