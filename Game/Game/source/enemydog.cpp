#include "enemydog.h"
#include "enemysensor.h"
#include "enemysoundsensor.h"
#include "enemysoundmanager.h"

bool EnemyDog::Initialize()
{
	base::Initialize();
	_handle = MV1LoadModel(mv1::SK_Dog);
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
	// センサー関連の初期化（共通は EnemyBase::Initialize で行われる）
	_rotationSpeed = 0.5f;                     // 回転速度（調整可能）
	_moveSpeed = 2.0f;

	// ランダムウォーク用の初期化
	_randomWalkTimer = 0.0f;
	_randomWalkInterval = 3.0f;		// 3秒ごとに方向変更
	_randomWalkDir = vec3::VGet(0.0f, 0.0f, 0.0f);
	_isRandomWalking = false;
	_randomWalkDistance = 0.0f;
	_randomWalkTraveledDistance = 0.0f;

	// 移動範囲制限の初期化
	_hasMovementArea = false;
	_movementAreaPoints.clear();

	// 整合性のため他は base に委譲済み
	// タイマー初期化等は base にて行われている
	return true;
}

// 初期位置と向きをキャプチャ
void EnemyDog::CaptureInitialTransform()
{
	_initialPosition = _vPos;
	_initialDirection = _vDir;
}

// EnemySensorを設定
void EnemyDog::SetEnemySensor(std::shared_ptr<EnemySensor> sensor)
{
	_enemySensor = sensor;
}

// 移動範囲を設定
void EnemyDog::SetMovementArea(const std::vector<vec::Vec3>& areaPoints)
{
	_movementAreaPoints = areaPoints;
	_hasMovementArea = (areaPoints.size() >= 3);
}

// 指定した位置が移動範囲内かチェック（2D平面でのポリゴン内外判定）
bool EnemyDog::IsPositionInArea(const vec::Vec3& pos) const
{
	if (!_hasMovementArea || _movementAreaPoints.size() < 3)
	{
		return true; // 範囲が設定されていない場合は常にtrue
	}

	// レイキャスティング法（Ray Casting Algorithm）を使用
	// X-Z平面で判定（Y座標は無視）
	int intersections = 0;
	size_t n = _movementAreaPoints.size();

	for (size_t i = 0; i < n; ++i)
	{
		size_t j = (i + 1) % n;

		const vec::Vec3& p1 = _movementAreaPoints[i];
		const vec::Vec3& p2 = _movementAreaPoints[j];

		// 点が線分のY範囲内にあるかチェック（Z座標で判定）
		if ((p1.z > pos.z) != (p2.z > pos.z))
		{
			// 交点のX座標を計算
			float intersectX = (p2.x - p1.x) * (pos.z - p1.z) / (p2.z - p1.z) + p1.x;

			// 点のX座標より右側に交点がある場合、カウント
			if (pos.x < intersectX)
			{
				intersections++;
			}
		}
	}

	// 交点の数が奇数なら内側、偶数なら外側
	return (intersections % 2) == 1;
}

bool EnemyDog::Terminate()
{
	base::Terminate();
	// モデルリソース解放など必要ならここで
	return true;
}

// 新しいランダム方向を設定
void EnemyDog::SetNewRandomDirection()
{
	// ランダムな角度を生成（0～360度）
	float randomAngle = (float)(rand() % 360) * (DX_PI_F / 180.0f);

	// ランダムな距離を設定（50～200の範囲）
	_randomWalkDistance = 50.0f + (float)(rand() % 151);
	_randomWalkTraveledDistance = 0.0f;

	// 角度から方向ベクトルを計算
	vec::Vec3 testDir;
	testDir.x = sin(randomAngle);
	testDir.y = 0.0f;
	testDir.z = cos(randomAngle);

	// 移動先の床の存在を確認
	vec::Vec3 testPos = vec3::VAdd(_vPos, vec3::VScale(testDir, _moveSpeed * 10.0f));

	if (CheckFloorExistence(testPos))
	{
		// 床がある場合は新しい方向を設定
		_randomWalkDir = testDir;
		_isRandomWalking = true;
	}
	else
	{
		// 床がない場合は別の方向を試す（最大8回まで試行）
		for (int i = 0; i < 16; i++)
		{
			randomAngle = (float)(rand() % 360) * (DX_PI_F / 180.0f);
			testDir.x = sin(randomAngle);
			testDir.y = 0.0f;
			testDir.z = cos(randomAngle);

			testPos = vec3::VAdd(_vPos, vec3::VScale(testDir, _moveSpeed * 10.0f));

			if (CheckFloorExistence(testPos))
			{
				_randomWalkDir = testDir;
				_isRandomWalking = true;
				return;
			}
		}

		// すべて失敗した場合は待機
		_isRandomWalking = false;
	}

	// 次の方向変更までの時間をランダムに設定（2～5秒）
	_randomWalkInterval = 2.0f + (float)(rand() % 31) / 10.0f;
	_randomWalkTimer = _randomWalkInterval;
}

// ランダムウォークの処理
void EnemyDog::ProcessRandomWalk()
{
	const float deltaTime = 1.0f / 60.0f; // 60FPS

	// タイマーを更新
	_randomWalkTimer -= deltaTime;

	// タイマーが0以下、または目標距離に到達したら新しい方向を設定
	if (_randomWalkTimer <= 0.0f || _randomWalkTraveledDistance >= _randomWalkDistance)
	{
		SetNewRandomDirection();
		return;
	}

	// ランダム移動中の場合
	if (_isRandomWalking)
	{
		// 移動量を計算
		vec::Vec3 movement = vec3::VScale(_randomWalkDir, _moveSpeed);
		vec::Vec3 newPos = vec3::VAdd(_vPos, movement);

		// 床の存在を確認
		if (CheckFloorExistence(newPos) && IsPositionInArea(newPos))
		{
			_vPos = newPos;
			_randomWalkTraveledDistance += _moveSpeed;

			// 徐々に移動方向を向く
			float currentAngle = atan2f(_vDir.x, _vDir.z);
			float targetAngle = atan2f(_randomWalkDir.x, _randomWalkDir.z);

			// 角度差を計算（-π から π の範囲に正規化）
			float angleDiff = targetAngle - currentAngle;
			while (angleDiff > DX_PI_F) angleDiff -= 2.0f * DX_PI_F;
			while (angleDiff < -DX_PI_F) angleDiff += 2.0f * DX_PI_F;

			// 回転速度を制限
			if (abs(angleDiff) > _rotationSpeed)
			{
				angleDiff = (angleDiff > 0) ? _rotationSpeed : -_rotationSpeed;
			}

			// 新しい角度を計算
			float newAngle = currentAngle + angleDiff;
			_vDir.x = sin(newAngle);
			_vDir.z = cos(newAngle);

			// ステータスをWALKに設定
			_status = STATUS::WALK;
		}
		else
		{
			// 床がない場合は新しい方向を設定
			SetNewRandomDirection();
		}
	}
	else
	{
		// 待機中
		_status = STATUS::WAIT;
	}
}

bool EnemyDog::Process()
{
	base::Process();

	CharaBase::STATUS old_status = _status;

	// ステータスがNONEの場合、WAITに設定
	if (_status == STATUS::NONE)
	{
		_status = STATUS::WAIT;
	}

	// 音検知タイマーの更新（音検知が有効な場合）
	if (_soundDetectionActive)
	{
		const float dt = 1.0f / 60.0f; // 60FPS想定
		_soundDetectionTimer += dt;

		// 音検知から一定時間経過したら初期位置に戻る
		if (_soundDetectionTimer >= SOUND_RETURN_TIME && !_detectedPlayer)
		{
			_isMovingToSound = false;
			_waitingAtSound = false;
			_soundWaitTimer = 0.0f;
			_soundDetectionActive = false;
			_soundDetectionTimer = 0.0f;
			StartReturningToInitialPosition();
		}
	}

	// 音源到達後の待機処理
	if (_waitingAtSound)
	{
		// 待機中はWAITステータスに設定
		_status = STATUS::WAIT;

		const float dt = 1.0f / 60.0f; // 60FPS想定
		_soundWaitTimer -= dt;

		// 待機時間が終了したら初期位置に戻る
		if (_soundWaitTimer <= 0.0f)
		{
			_waitingAtSound = false;
			_soundWaitTimer = 0.0f;
			_isMovingToSound = false;
			_soundDetectionActive = false;
			_soundDetectionTimer = 0.0f;
			StartReturningToInitialPosition();
		}
	}

	// 検知終了後の待機処理（帰還前の待機）
	if (_waitingBeforeReturn)
	{
		const float dt = 1.0f / 60.0f; // 60FPS想定
		_returnWaitTimer -= dt;

		_status = STATUS::WAIT;

		if (_returnWaitTimer <= 0.0f)
		{
			_waitingBeforeReturn = false;
			_returnWaitTimer = 0.0f;
			StartReturningToInitialPosition();
			_playSightOffOnReturn = true;
		}
		// 待機中にプレイヤーを再検出したら待機をキャンセル
		if (_detectedPlayer && _enemySensor && _enemySensor->IsChasing())
		{
			_waitingBeforeReturn = false;
			_returnWaitTimer = 0.0f;
		}
		return true;
	}

	// EnemySoundSensorから音の検知情報を取得
	if (_enemySoundSensor)
	{
		const auto& detectionInfo = _enemySoundSensor->GetDetectionInfo();
		if (detectionInfo.isDetected && detectionInfo.detectedSoundLevel == 5)
		{
			if (!_detectedPlayer && (!_enemySensor || !_enemySensor->IsChasing()) && !_isReturningToInitialPos)
			{
				_isMovingToSound = true;
				_soundSourcePosition = detectionInfo.soundSourcePosition;
				_soundDetectionActive = true;
				_soundDetectionTimer = 0.0f;
				_isRandomWalking = false; // ランダム移動を停止
			}
		}
	}

	// 優先順位: プレイヤー検出（追跡） > 音源への移動 > 初期位置への帰還 > ランダムウォーク
	if (_detectedPlayer && _enemySensor && _enemySensor->IsChasing())
	{
		// プレイヤーを追跡する（EnemyBaseの追跡処理を使用）
		UpdateChasing();

		// EnemySoundManagerを使用して音波を発生させる
		auto soundManager = EnemySoundManager::GetInstance();
		if (soundManager)
		{
			soundManager->EmitSound(GetPos(), 5, 1000.0f, 10.0f, GetEnemyId());
		}

		_isRandomWalking = false; // ランダム移動を停止
		_isMovingToSound = false; // 音源への移動を停止
		// ステータスはUpdateChasing()内で設定される
	}
	else if (_isMovingToSound && !_detectedPlayer && (!_enemySensor || !_enemySensor->IsChasing()))
	{
		UpdateMovingToSound();
	}
	else if (_isReturningToInitialPos)
	{
		UpdateReturningToInitialPosition();
		_isRandomWalking = false; // ランダム移動を停止
	}
	else if (!IsStun())
	{
		// 通常時はランダムウォーク
		ProcessRandomWalk();
	}
	else
	{
		// スタン中は待機
		_status = STATUS::WAIT;
		_isRandomWalking = false;
	}

	// ステータスが変わっていないか？
	if (old_status == _status)
	{
		//再生時間を進める
		_fPlayTime += 0.5f;
		// 再生時間をランダムに揺らがせる
		switch (_status)
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
		if (_iAttachIndex != -1)
		{
			MV1DetachAnim(_handle, StCas<int>(_iAttachIndex));
			_iAttachIndex = -1;
		}
		// ステータスに応じたアニメーションをアタッチする
		switch (_status)
		{
		case STATUS::WAIT:
		{
			int animIndex = MV1GetAnimIndex(_handle, "taiki");
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
		case STATUS::WALK:
		{
			int animIndex = MV1GetAnimIndex(_handle, "walk");
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
		if (_iAttachIndex != -1)
		{
			_fTotalTime = MV1GetAttachAnimTotalTime(_handle, StCas<int>(_iAttachIndex));
		}
		// 再生時間を初期化
		_fPlayTime = 0.0f;
		// 再生時間をランダムにずらす
		switch (_status)
		{
		case STATUS::WAIT:
		{
			_fPlayTime += rand() % 30; // 0 ～ 29 の揺らぎ
			break;
		}
		}
	}

	// 再生時間がアニメーションの総再生時間に達したら再生時間を0に戻す
	if (_fPlayTime >= _fTotalTime)
	{
		_fPlayTime = 0.0f;
	}

	return true;
}

bool EnemyDog::Render()
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
