#include "enemydog.h"
#include "enemysensor.h"
#include "enemysoundsensor.h"

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

bool EnemyDog::Terminate()
{
	base::Terminate();
	// モデルリソース解放など必要ならここで
	return true;
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

		//// 10秒経過したら初期位置への帰還を開始
		//if (_soundDetectionTimer >= SOUND_RETURN_TIME)
		//{
		//	_soundDetectionActive = false;
		//	_soundDetectionTimer = 0.0f;
		//	_isMovingToSound = false;
		//	_waitingAtSound = false;

		//	StartReturningToInitialPosition();
		//}
	}

	// 音源到達後の待機処理
	if (_waitingAtSound)
	{
		// 待機中はWAITステータスに設定
		_status = STATUS::WAIT;

		// 待機中はプレイヤー検出されたら割り込み可能
		// この処理は下のEnemySensorの処理で行われる
	}

	// EnemySoundSensorから音の検知情報を取得
	if (_enemySoundSensor)
	{
		// 音レベル5の音を検知したかチェック
		const auto& detectionInfo = _enemySoundSensor->GetDetectionInfo();
		if (detectionInfo.isDetected && detectionInfo.detectedSoundLevel == 5)
		{
			// プレイヤーを検出中または追跡中でなければ、音源に向かって移動開始
			if (!_detectedPlayer && (!_enemySensor || !_enemySensor->IsChasing()) && !_isReturningToInitialPos)
			{
				_isMovingToSound = true;
				_soundSourcePosition = detectionInfo.soundSourcePosition;

				// 音検知タイマーを開始
				_soundDetectionActive = true;
				_soundDetectionTimer = 0.0f;

				//// 初期位置への帰還を中断
				//_waitingBeforeReturn = false;
				//_returnWaitTimer = 0.0f;
			}
		}
	}

	// 音源への移動処理（プレイヤー検出より優先度は低い）
	if (_isMovingToSound && !_detectedPlayer && (!_enemySensor || !_enemySensor->IsChasing()))
	{
		UpdateMovingToSound();
	}

	// プレイヤーを検出している場合、プレイヤーの方向に徐々に向く
	if (_detectedPlayer)
	{
		UpdateRotationToPlayer(); // 徐々に回転
		// または即座に向きたい場合は LookAtPlayer(); を使用

		// センサーの存在を確認してから音波を発生させる（音量設定を先に行う）
		auto soundSensor = GetSoundSensor();
		if (soundSensor)
		{
			soundSensor->SetSoundLevel(5); // 先にレベルをセット
			soundSensor->TriggerSoundWave(GetPos(), 1000.0f, 10.0f); // そのレベルで波を生成
		}
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

	// 定期的に方向を90度変える処理
	DirChangeTimer -= 1.0f / 60.0f;

	// タイマーが0以下になったら方向を変える
	if (DirChangeTimer <= 0.0f)
	{
		// タイマーをリセット（15秒ごとに変更）
		DirChangeTimer = 15.0f;

		// プレイヤーを検出していない、かつ追跡中でもない、かつ初期位置に戻り中でもない場合のみ回転
		if (!_detectedPlayer && (!_enemySensor || !_enemySensor->IsChasing()) && !_isReturningToInitialPos && !_isMovingToSound)
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
			if (CheckFloorExistence(testPos))
			{
				_vDir = newDir;
			}
		}
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
