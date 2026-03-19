
#include "playertanuki.h"
#include "appframe.h"
#include "applicationglobal.h"
#include "modegame.h"

bool PlayerTanuki::Initialize()
{
	if(!base::Initialize()) { return false; }
	if(!gGlobal.IsLoading()) SetHandle(ResourceServer::MV1LoadModel(mv1::SK_goepon_multimotion_4));
	_iAttachIndex = -1;

	_status = STATUS::NONE;

	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;

	_fColSubY = 40.0f;
	
	_fCollisionR = 30.0f;			// キャラの当たり判定用の円の半径
	_fCollisionWeight = 20.0f;		// キャラの重さ
	_cam = nullptr;
	_fMvSpeed = 7.0f;

	_normalSpeed = _fMvSpeed;
	_dash = false;
	_dashTimer = 0.0f;
	_dashCount = 0;

	_dashCoolDownTime = 0.0f;

	_dashRecoverTime = 0.0f;
	_dashRecoverActive = false;

	_bLand = true;

	_inputEnabled = true;

	_transformPlayerButtonDown = false;
	_transformPlayerMove = false;
	_transformPlayerCandidate = false;
	_transformPlayerMonoButtonDown = false;
	_transformPlayerMonoMove = false;
	_transformPlayerMonoCandidate = false;

	_gameClearModelHandle = -1;
	_gameOverModelHandle  = -1;
	return true;
}


bool PlayerTanuki::Terminate()
{
	base::Terminate();

	return true;
}

void PlayerTanuki::ResetDash()
{
	_dash = false;
	_dashTimer = 0.0f;
	_dashCount = 0;
	_dashCoolDownTime = 0.0f;
	_dashRecoverTime = 0.0f;
	_dashRecoverActive = false;
	_fMvSpeed = _normalSpeed;

	// ダッシュ用の効果音が再生中なら停止しておく
	if(gGlobal._soundServer)
	{
		auto s = gGlobal._soundServer->Get("5"); // 既存のダッシュ音キー
		if(s && s->IsPlay())
		{
			s->Stop();
		}
	}
}

bool PlayerTanuki::SoundWalk()
{
	if(gGlobal._soundServer)
	{
		auto sound = gGlobal._soundServer->Get("1");
		if(sound)
		{
			// パッドの方向キーが押されているか検出
			int key = ApplicationBase::GetInstance()->GetKey();
			bool padPressed = (key & (PAD_INPUT_LEFT | PAD_INPUT_RIGHT | PAD_INPUT_UP | PAD_INPUT_DOWN)) != 0;

			// アナログスティックでの移動判定（しきい値以上なら移動）
			float stickLen = sqrtf(lStickX * lStickX + lStickZ * lStickZ);
			bool analogMove = (stickLen >= _fAnalogDeadZone);

			// 歩行ステータスかつ方向入力（十字キー or アナログ）がある間だけ鳴らす
			if(_status == STATUS::WALK && (padPressed || analogMove))
			{
				// 既に再生中でなければ再生（毎フレーム呼ばれても重複再生しないようにする）
				if(!sound->IsPlay())
				{
					sound->Play();
				}
			}
			else
			{
				// 入力が無ければ再生中なら停止
				if(sound->IsPlay())
				{
					sound->Stop();
				}
			}
		}
	}


	return true;
}


bool PlayerTanuki::Process()
{
	base::Process();

	int key = ApplicationBase::GetInstance()->GetKey();
	int trg = ApplicationBase::GetInstance()->GetTrg();
	int rel = ApplicationBase::GetInstance()->GetRel();

	_vOldPos = _vPos;

	CharaBase::STATUS old_status = _status;

	_v = { 0,0,0 };

	// 押している状態を記録する

	float pressStickLen = sqrtf(fLx * fLx + fLz * fLz);
	bool pressPadDir = (key & (PAD_INPUT_LEFT | PAD_INPUT_RIGHT | PAD_INPUT_UP | PAD_INPUT_DOWN)) != 0;
	if(trg & PAD_INPUT_4)
	{
		_transformPlayerButtonDown = true;
		_transformPlayerMove = false;
		_transformPlayerCandidate = !(pressPadDir || pressStickLen >= _fAnalogDeadZone);

	}

	if(trg & PAD_INPUT_3)
	{
		_transformPlayerMonoButtonDown = true;
		_transformPlayerMonoMove = false;
		_transformPlayerMonoCandidate = !(pressPadDir || pressStickLen >= _fAnalogDeadZone);
	}

	if(!_inputEnabled)
	{
		_status = STATUS::WAIT;
	}
	else
	{
		float sx = _cam->GetPos().x - _cam->GetTarget().x;
		float sz = _cam->GetPos().z - _cam->GetTarget().z;
		float camrad = atan2(sz, sx);


		lStickX = fLx;
		lStickZ = fLz;

		vec::Vec3 inputLocal = vec3::VGet(0.0f, 0.0f, 0.0f);
		if(key & PAD_INPUT_DOWN)
		{
			inputLocal.z = 1.0f;
		}
		if(key & PAD_INPUT_UP)
		{
			inputLocal.z = -1.0f;
		}
		if(key & PAD_INPUT_LEFT)
		{
			inputLocal.x = -1.0f;
		}
		if(key & PAD_INPUT_RIGHT)
		{
			inputLocal.x = 1.0f;
		}

		// 左スティックの長さを計算
		float length = sqrtf(lStickX * lStickX + lStickZ * lStickZ);

		// デッドゾーン未満なら十字キーを採用（合成ではなく置き換え）
		if(length < _fAnalogDeadZone)
		{
			// 十字キーの入力を fLx/fLz と同じ軸系に寄せる
			lStickX = inputLocal.x;
			lStickZ = inputLocal.z;

			length = sqrtf(lStickX * lStickX + lStickZ * lStickZ);
		}

		// 移動検知
		bool padPress = (key & (PAD_INPUT_LEFT | PAD_INPUT_RIGHT | PAD_INPUT_UP | PAD_INPUT_DOWN)) != 0;
		bool moving = (length >= _fAnalogDeadZone) || padPress;
		if(_transformPlayerButtonDown)
		{
			if(moving)
			{
				_transformPlayerMove = true;
				_transformPlayerCandidate = false;
			}
			
		}
		if(_transformPlayerMonoButtonDown)
		{
			if(moving)
			{
				_transformPlayerMonoMove = true;
				_transformPlayerMonoCandidate = false;
			}
		}

		// ローカル角度
		float localRad = 0.0f;

		if(length >= _fAnalogDeadZone)
		{
			float moveX = lStickZ; // 前後
			float moveZ = lStickX; // 左右

			// 入力ベクトル保存
			_vInput = vec3::VGet(moveX, 0.0f, moveZ);
			// 正規化
			if(vec3::VSize(_vInput) > 0.0f)
			{
				_vInput = vec3::VNorm(_vInput);
			}

			// 角度は atan2(z, x) がXZ平面の標準
			localRad = atan2f(moveZ, moveX);

			// 速度：アナログは length(0..1想定) * _fMvSpeed にした方が自然
			float speed = _fMvSpeed;
			float moveLen = speed * length;

			// ワールド方向の移動ベクトルに変換
			_v.x = cosf(localRad + camrad) * moveLen;
			_v.z = sinf(localRad + camrad) * moveLen;

			SetTargetRotationFromDirection(_v); // 入力方向に向くように目標回転を設定

			_vDir = _v;
			_status = STATUS::WALK;

			if(!_dash && (trg & PAD_INPUT_2) && moving && _dashCount < dash::DASH_MAX && _dashCoolDownTime <= 0.0f)
			{
				_dash = true;
				_dashTimer = dash::DASH_DURATION;
				_fMvSpeed = _normalSpeed * dash::DASH_SPEED; // ダッシュ開始時に速度を上げる
				_dashCount++;
				if(_dashRecoverActive)
				{
					_dashRecoverActive = false;
					_dashRecoverTime = 0.0f;
				}
			}
		}
		else
		{
			_v = { 0.0f, 0.0f, 0.0f };
			_vInput = vec3::VGet(0.0f, 0.0f, 0.0f);
			_status = STATUS::WAIT;
		}
		if(!_dash && (trg & PAD_INPUT_2) && moving && _dashCount < dash::DASH_MAX && _dashCoolDownTime <= 0.0f)
		{
			_dash = true;
			_dashTimer = dash::DASH_DURATION;
			_fMvSpeed = _normalSpeed * dash::DASH_SPEED; // ダッシュ開始時に速度を上げる
			_dashCount++;
			if(_dashRecoverActive)
			{
				_dashRecoverActive = false;
				_dashRecoverTime = 0.0f;
			}

			if(gGlobal._soundServer)
			{
				auto s = gGlobal._soundServer->Get("5"); // ApplicationGlobal で "5" に tanuki_run を登録済み
				if(s && !s->IsPlay())
				{
					s->Play();
				}
			}
		}
	}

	if(_status == STATUS::WALK)
	{
		UpdateRotation();
	}

	if(_dash)
	{
		_dashTimer -= 1.0f/ 60.0f; // 仮に60FPSで更新されると想定してタイマーを進める
		if(_dashTimer <= 0.0f)
		{
			_dash = false;
			_dashTimer = 0.0f;
			_fMvSpeed = _normalSpeed * dash::DASH_COOLDOWN_SPEED;
			_dashCoolDownTime = dash::DASH_COOL_DOWN_DURATION; // ダッシュ終了後にクールダウン開始
			_dashRecoverTime = 0.0f;
			_dashRecoverActive = false;

			if(gGlobal._soundServer)
			{
				auto s = gGlobal._soundServer->Get("5");
				if(s && s->IsPlay())
				{
					s->Stop();
				}
			}
		}
	}
	if(_dash)
	{
		_dashTimer -= 1.0f / 60.0f; // 仮に60FPSで更新されると想定してタイマーを進める
		if(_dashTimer <= 0.0f)
		{
			_dash = false;
			_dashTimer = 0.0f;
			_fMvSpeed = _normalSpeed;
			_dashCoolDownTime = dash::DASH_COOL_DOWN_DURATION; // ダッシュ終了後にクールダウン開始
			_dashRecoverTime = 0.0f;
			_dashRecoverActive = false;
		}
	}

	// ダッシュ時のクールタイム
	if(_dashCoolDownTime > 0.0f)
	{
		_dashCoolDownTime -= 1.0f / 60.0f; // クールダウンタイマーも進める
		if(_dashCoolDownTime < 0.0f)
		{
			_dashCoolDownTime = 0.0f;
			if(_dashCount > 0 && !_dashRecoverActive)
			{
				_dashRecoverActive = true; // クールダウンが終わったら回復開始
				_dashRecoverTime = dash::DASH_RECOVER_INTERVAL;
			}
		}
	}

	// ダッシュ回復処理
	if(_dashRecoverActive)
	{
		_dashRecoverTime -= 1.0f / 60.0f; // 回復タイマーも進める
		if(_dashRecoverTime <= 0.0f)
		{
			if(_dashCount > 0)
			{
				_dashCount = _dashCount - 1;
			}
			else
			{
				_dashCount = 0;
			}

			if(_dashCount > 0)
			{
				_dashRecoverTime = dash::DASH_RECOVER_INTERVAL; // 回復インターバルをリセットして次の回復まで待つ
			}
			else
			{
				_dashRecoverActive = false; // 全て回復したら回復終了
				_dashRecoverTime = 0.0f;
			}
		}
	}

	if(old_status != _status)
	{
		SoundWalk();
	}
	
	// アニメーションの名前取得
	auto GetAnimName = [this](STATUS name) -> std::string
		{
			switch(name)
			{
			case STATUS::WAIT:
				return "idle";
			case STATUS::WALK:
				return "walk";
			default:
				return std::string();
			}
		};

	auto PlayAnim = [&](bool change)
		{
			std::string name = GetAnimName(_status);
			if(name.empty()) { return; }

			_animId = AnimationManager::GetInstance()->Play(_handle, name, true);
			_fPlayTime = 0.0f;

			if(change)
			{
				switch(_status)
				{
				case STATUS::WAIT:
					_fPlayTime += rand() % 30;
					break;
				}
			}

			if(_animId != -1)
			{
				AnimationManager::GetInstance()->SetTime(_animId, _fPlayTime);
			}
		};

	if(_animId != -1 && !AnimationManager::GetInstance()->IsPlaying(_animId))
	{
		_animId = -1;
		PlayAnim(false);
	}

	if(old_status == _status)
	{
		float anim_speed = 0.5f;
		if(_dash && _status == STATUS::WALK)
		{
			anim_speed *= dash::DASH_SPEED; // ダッシュ中はアニメーション速度も速くする
		}
		_fPlayTime += anim_speed;
		switch(_status)
		{
		case STATUS::WAIT:
			_fPlayTime += (float)(rand() % 10) / 100.0f;
			break;
		}
	}
	else
	{
		if(_animId != -1)
		{
			AnimationManager::GetInstance()->Stop(_animId);
			_animId = -1;
		}
		PlayAnim(true);
	}

	if(_fPlayTime >= _fTotalTime)
	{
		_fPlayTime = 0.0f;
	}

	if(rel & PAD_INPUT_4)
	{
		if(_transformPlayerButtonDown && !_transformPlayerMove)
		{
			ModeBase* base = ModeServer::GetInstance()->Get("game");
			if(base)
			{
				auto* game = dynamic_cast<ModeGame*>(base);
				if(!game->IsTransformRequested() && !game->IsTransforming())
				{
					game->RequestTransformToHuman();
				}
				else
				{
					if(game->IsTransformRequested() && !game->IsTransforming())
					{
						game->CancelRequestedTransform();
					}
					else if(game->IsTransforming())
					{
						game->RequestReturnToTanukiFromHuman();
					}
				}
			}
		}

		_transformPlayerButtonDown = false;
		_transformPlayerMove = false;
		_transformPlayerCandidate = false;
	}

	if(rel & PAD_INPUT_3)
	{
		if(_transformPlayerMonoButtonDown && !_transformPlayerMonoMove)
		{
			ModeBase* base = ModeServer::GetInstance()->Get("game");
			if(base)
			{
				auto* game = dynamic_cast<ModeGame*>(base);
				if(!game->IsTransformRequested() && !game->IsTransforming())
				{
					game->RequestTransformToMono();
				}
				else
				{
					if(game->IsTransformRequested() && !game->IsTransforming())
					{
						game->CancelRequestedTransform();
					}
					else if(game->IsTransforming())
					{
						game->RequestReturnToTanukiFromHuman();
					}
				}
			}
		}
		_transformPlayerMonoButtonDown = false;
		_transformPlayerMonoMove = false;
		_transformPlayerMonoCandidate = false;
	}

	return true;
}

bool PlayerTanuki::Render()
{
	base::Render();

	float vorty = GetRotationY();// モデルが標準でどちらを向いているかで式が変わる(これは-zを向いている場合)

	MATRIX mRotY = MGetRotY(vorty);

	MATRIX mRotZ = MGetRotZ(DX_PI_F * 0.5f); 

	MATRIX mTrans = MGetTranslate(DxlibConverter::VecToDxLib(_vPos)); // 平行移動行列

	MATRIX mScale = MGetScale(VGet(1.0f, 1.0f, 1.0f));					// 拡大行列

	MATRIX m = MGetIdent();

	//m = MMult(m, mRotZ);
	m = MMult(m, mRotY);
	m = MMult(m, mScale);
	m = MMult(m, mTrans);

	MV1SetMatrix(_handle, m);

	MV1DrawModel(_handle);

#ifdef _DEBUG
	// ダッシュ時間とクールダウンをデバッグ表示
	{
		unsigned int col = GetColor(255, 255, 0);
		// 表示位置は必要に応じて調整してください
	//	DrawFormatString(10, 40, col, "Dash timer: %.2f / %.2f", _dashTimer, dash::DASH_DURATION);
	//	DrawFormatString(10, 56, col, "Dash cooldown: %.2f", _dashCoolDownTime);
	//	DrawFormatString(10, 72, col, "Dash used: %d / %d", _dashCount, (int)dash::DASH_MAX);
		// 回復タイマー表示（アクティブな場合）
		if(_dashRecoverActive)
		{
			//DrawFormatString(10, 88, col, "Recover in: %.2f", _dashRecoverTime);
		}
	}
#endif

	return true;
}

bool PlayerTanuki::SetGameClearHandle(const std::string& animName, bool loop)
{
	// すでに切り替え済みなら何もしない
	if(_gameClearModelHandle >= 0 && _handle == _gameClearModelHandle)
	{
		return true;
	}

	// クリア用モデルを末ロードなら取得
	if(_gameClearModelHandle < 0)
	{
		_gameClearModelHandle = ResourceServer::MV1LoadModel(mv1::GameClear_Tanuki);
		if(_gameClearModelHandle < 0)
		{
			return false;
		}
	}
	// 今のモデルに紐づくアニメが残っている可能性があるので止める
	StopAnimation();

	// 今のモデルを破棄して差し替え
	if(_handle >= 0)
	{
		ResourceServer::MV1DeleteModel(_handle);
		_handle = -1;
	}
	_handle = _gameClearModelHandle;

	// 切替後にアニメを再生
	if(!animName.empty())
	{
		PlayGameClearAnimation(animName, loop);
	}
	return true;
}

int PlayerTanuki::PlayGameClearAnimation(std::string name, bool loop)
{
	if(_animId != -1)
	{
		AnimationManager::GetInstance()->Stop(_animId);
		_animId = -1;
	}

	if(_gameClearModelHandle == -1 || name.empty())
	{
		return -1;
	}

	_animId = AnimationManager::GetInstance()->Play(_gameClearModelHandle, name, loop);
	if(_animId != -1)
	{
		AnimationManager::GetInstance()->SetTime(_animId, 0.0f);
	}
	return _animId;
}

bool PlayerTanuki::SetGameOverHandle(const std::string& animName, bool loop)
{
	if(_gameOverModelHandle >= 0 && _handle == _gameOverModelHandle)
	{
		return true;
	}

	if(_gameOverModelHandle < 0)
	{
		_gameOverModelHandle = ResourceServer::MV1LoadModel(mv1::GameOver_Tanuki);
		if(_gameOverModelHandle < 0)
		{
			return false;
		}
	}

	StopAnimation();

	if(_handle >= 0)
	{
		ResourceServer::MV1DeleteModel(_handle);
		_handle = -1;
	}
	_handle = _gameOverModelHandle;

	// 切り替え後にアニメーション再生
	if(!animName.empty())
	{
		PlayGameOverAnimation(animName, loop);
	}
	return true;
}

int PlayerTanuki::PlayGameOverAnimation(std::string name, bool loop)
{
	if(_animId != -1)
	{
		AnimationManager::GetInstance()->Stop(_animId);
	}

	if(_gameOverModelHandle == -1 || name.empty())
	{
		return -1;
	}

	_animId = AnimationManager::GetInstance()->Play(_gameOverModelHandle, name, loop);
	if(_animId != -1)
	{
		AnimationManager::GetInstance()->SetTime(_animId, 0.0f);
		_fPlayTime = 0.0f;
		_fTotalTime = AnimationManager::GetInstance()->GetTotalTime(_animId);
	}
	return _animId;
}

bool PlayerTanuki::RestoreDefaultModel(const std::string& animName, bool loop)
{
	// 通常モデルを再取得
	const int defaultHandle = ResourceServer::MV1LoadModel(mv1::SK_goepon_multimotion_4);
	if(defaultHandle < 0)
	{
		return false;
	}

	// 既存アニメ停止
	StopAnimation();

	// 現在のモデルを破棄して差し替え
	if(_handle >= 0)
	{
		ResourceServer::MV1DeleteModel(_handle);
		_handle = -1;
	}
	_handle = defaultHandle;

	// 演出用ハンドルを保持して再利用する
	_gameClearModelHandle = -1;
	_gameOverModelHandle = -1;

	// 通常アニメ再生
	if(!animName.empty())
	{
		_animId = AnimationManager::GetInstance()->Play(_handle, animName, loop);
		if(_animId != -1)
		{
			AnimationManager::GetInstance()->SetTime(_animId, 0.0f);
		}
	}

	return true;
}