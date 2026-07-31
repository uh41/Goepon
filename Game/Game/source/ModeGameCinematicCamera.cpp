#include "modegame.h"
#include "ModeGameClear.h"
#include "ModeGameOver.h"
#include "mymath.h"
#include "playertanuki.h"
#include "playerform.h"

bool ModeGame::DebugCinematicCameraControl()
{
	// 現在のカメラがないなら処理しない
	if(!_camera)
	{
		return false;
	}

	// シネマティックカメラが無ければ作成（デバッグ用）
	if(!_cinematicCamera)
	{
		_cinematicCamera = std::make_unique<CinematicCamera>();
		if(!_cinematicCamera->Initialize())
		{
			_cinematicCamera.reset();
			return false;
		}
	}

	// 元のカメラ保存が無ければ保存（演出開始時に戻すため）
	if(!_savedCamera && !_useCinematicCamera)
	{
		_savedCamera = _camera;
	}

	// ズーム処理をする
	if(CheckHitKey(KEY_INPUT_F1))
	{
		if(!_debugF1KeyPressed)
		{
			_debugF1KeyPressed = true;

			if(!_debugZoomActive)
			{
				_debugZoomActive = true;

				// 演出カメラに現在のカメラ状態をコピーして切り替え
				_cinematicCamera->SetPos(_camera->GetPos());
				_cinematicCamera->SetTarget(_camera->GetTarget());
				_cinematicCamera->SetClipNear(_camera->GetClipNear());
				_cinematicCamera->SetClipFar(_camera->GetClipFar());

				_useCinematicCamera = true;
				_camera = _cinematicCamera.get();

				// 現在のプレイヤーの位置を取得
				PlayerBase* targetPlayer = nullptr;
				auto* playerManager = PlayerManager::GetInstance();
				switch(playerManager->GetPlayerState())
				{
				case PlayerManager::PlayerState::TANUKI:
				{
					targetPlayer = PlayerFactory::GetTanukiPlayer();
					break;
				}
				case PlayerManager::PlayerState::HUMAN:
				{
					targetPlayer = PlayerFactory::GetHumanPlayer();
					break;
				}
				case PlayerManager::PlayerState::MONO:
				{
					targetPlayer = PlayerFactory::GetMonoPlayer();
					break;
				}
				}

				if(targetPlayer)
				{
					vec::Vec3 target = targetPlayer->GetPos();

					vec::Vec3 targetPos = _cinematicCamera->GetPos();
					float targetDist = vec3::VSize(vec3::VSub(targetPos, target));
					float endDist = targetDist * 0.25f;
					if(endDist < 50.0f) endDist = 50.0f;

					_cinematicCamera->StartZoom(target, 0.5f, targetDist, endDist);
				}
			}
			// ズーム解除
			else
			{
				_debugZoomActive = false;

				if(_cinematicCamera)
				{
					_cinematicCamera->StopAll();
				}

				if(_savedCamera)
				{
					_camera = _savedCamera;
				}
				_useCinematicCamera = false;
			}
		}
	}
	else
	{
		_debugF1KeyPressed = false;
	}

	// カメラを揺らす
	if(CheckHitKey(KEY_INPUT_F2))
	{
		if(!_debugF2KeyPressed)
		{
			_debugF2KeyPressed = true;

			if(!_debugShakeActive)
			{
				_debugShakeActive = true;

				// 演出カメラに現在のカメラ状態をコピーして切り替え
				_cinematicCamera->SetPos(_camera->GetPos());
				_cinematicCamera->SetTarget(_camera->GetTarget());
				_cinematicCamera->SetClipNear(_camera->GetClipNear());
				_cinematicCamera->SetClipFar(_camera->GetClipFar());

				_useCinematicCamera = true;
				_camera = _cinematicCamera.get();

				// 揺れ開始
				const float intensity = 50.0f;   // 揺れ幅
				const float durationSec = 0.50f; // 揺れ時間
				_cinematicCamera->StartShake(intensity, durationSec);
			}
			else
			{
				// 手動で揺れ解除
				_debugShakeActive = false;

				if(_cinematicCamera)
				{
					_cinematicCamera->StopAll();
				}

				if(_savedCamera)
				{
					_camera = _savedCamera;
				}
				_useCinematicCamera = false;
			}
		}
	}
	else
	{
		_debugF2KeyPressed = false;
	}

	// 揺れが自然終了したら元カメラに戻す
	if(_debugShakeActive && _useCinematicCamera && _cinematicCamera)
	{
		if(_cinematicCamera->GetState() == CinematicCamera::State::Idle)
		{
			_debugShakeActive = false;

			if(_savedCamera)
			{
				_camera = _savedCamera;
			}
			_useCinematicCamera = false;
		}
	}

	return true;
}

bool ModeGame::TreasureOpeningCameraControl()
{
	if(_isOpeningTreasure)
	{
		return false; // またはすでに開いている場合は処理しない
	}

	// シネマティックカメラが初期化されていない場合は作成
	if(!_cinematicCamera)
	{
		_cinematicCamera = std::make_unique<CinematicCamera>();
		if(!_cinematicCamera->Initialize())
		{
			_cinematicCamera.reset();
			return false;
		}
	}

	// 元のカメラを保存
	if(!_useCinematicCamera)
	{
		if(_cinematicCamera && _camera)
		{
			// 演出カメラに現在のカメラ位置と注目点をコピーして切り替え
			_cinematicCamera->SetPos(_camera->GetPos());
			_cinematicCamera->SetTarget(_camera->GetTarget());
			_cinematicCamera->SetClipNear(_camera->GetClipNear());
			_cinematicCamera->SetClipFar(_camera->GetClipFar());

			_savedCamera = _camera;
			_useCinematicCamera = true;
			_camera = _cinematicCamera.get();

			// Map 側が保持しているカメラ参照も合わせる
			if(_objectServer)
			{
				if(auto* map = _objectServer->GetMap())
				{
					map->SetCamera(_camera);
				}
			}
		}
		else
		{
			// カメラがない場合は処理をスキップ
			return false;
		}

		// 現在のプレイヤー位置を取得
		PlayerBase* targetPlayer = PlayerForm::GetInstance()->GetPlayer();

		// ズーム処理
		if(targetPlayer && _cinematicCamera)
		{
			vec::Vec3 target = targetPlayer->GetPos();
			vec::Vec3 currentPos = _cinematicCamera->GetPos();
			float currentDist = vec3::VSize(vec3::VSub(currentPos, target));
			float endDist = currentDist * 0.5f;  // 距離に設定
			if(endDist < 50.0f) endDist = 50.0f; // 最小距離の制限

			// ズーム演出：現在距離から近距離へ
			_cinematicCamera->StartZoom(target, 0.5f, currentDist, endDist);
		}
	}
	return true;
}

bool ModeGame::EndCinematicCamera()
{
	// 宝箱などの演出終了はメインカメラへ戻す
	return EndCinematicSequence(true);
}

bool ModeGame::StartIntroSequence()
{
	// カメラがない時は処理をしない
	if(!_camera)
	{
		return false;
	}

	// 演出カメラが無かったら用意する
	if(!_cinematicCamera)
	{
		_cinematicCamera = std::make_unique<CinematicCamera>();
		if(!_cinematicCamera->Initialize())
		{
			_cinematicCamera.reset();
			return false;
		}
	}

	// 元のカメラを保持して演出カメラに切り替え
	if(!_useCinematicCamera)
	{
		_savedCamera = _camera;

		// 元のカメラ状態をコピー
		_cinematicCamera->SetPos(_savedCamera->GetPos());
		_cinematicCamera->SetTarget(_savedCamera->GetTarget());
		_cinematicCamera->SetClipNear(_savedCamera->GetClipNear());
		_cinematicCamera->SetClipFar(_savedCamera->GetClipFar());

		_camera = _cinematicCamera.get(); // カメラ切り替え
		_useCinematicCamera = true;
	}

	// イントロ開始
	_isIntroActive = true;
	_introButtonPressed = false;
	_introTimer = 0.0f;

	PlayerBase* targetPlayer = PlayerFactory::GetTanukiPlayer();
	if(!targetPlayer || !targetPlayer->IsAlive())
	{
		_isIntroActive = false;
		return false;
	}

	if(targetPlayer)
	{
		const vec::Vec3 playerPos = targetPlayer->GetPos();
		const float playerRotY = targetPlayer->GetRotationY(); // プレイヤーの向きを取得するために必要

		vec::Vec3 playerDir{};
		playerDir.x = sinf(playerRotY);
		playerDir.z = cosf(playerRotY);

		if(vec3::VSize(playerDir) > 0.001f)
		{
			playerDir = vec3::VNorm(playerDir);
		}

		const float cameraDistance = 300.0f;
		const float cameraHeight = 500.0f;
		const float targetHeight = 60.0f;

		// DXLibの仕様に合わせて前方はマイナスに合わす
		const vec::Vec3 cameraPos = vec3::VGet
		(
			playerPos.x - playerDir.x * cameraDistance,
			playerPos.y + cameraHeight,
			playerPos.z - playerDir.z * cameraDistance
		);

		const vec::Vec3 cameraTarget = vec3::VAdd(playerPos, vec3::VGet(0.0f, targetHeight, 0.0f));

		_cinematicCamera->SetPos(cameraPos);
		_cinematicCamera->SetTarget(cameraTarget);
	}
	_introPhase = IntroPhase::RotateForward;

	// プレイヤーを中心に前方から後方へ回る
	const float rotateAngleRad = DX_PI_F * (35.0f / 180.0f);
	const float rotateDurationSec = 1.5f;
	const float rotateSpeed = rotateAngleRad / rotateDurationSec;
	_cinematicCamera->StartRotateSpeed(rotateSpeed, rotateDurationSec);

	// プレイヤーの操作を止める
	if(targetPlayer)
	{
		targetPlayer->SetInputEnabled(false);
	}
	return true;
}

bool ModeGame::ProcessIntroSequence()
{
	if(!_isIntroActive)
	{
		return false;
	}

	if(!_cinematicCamera)
	{
		return false;
	}

	// イントロ中は演出カメラの更新を回す
	_cinematicCamera->Process();

	// イントロ中はプレイヤー位置を追従
	PlayerBase* tanuki = PlayerFactory::GetTanukiPlayer();
	if(tanuki && tanuki->IsAlive())
	{
		vec::Vec3 playerPos = tanuki->GetPos();

		if(_introPhase == IntroPhase::Zoom)
		{
			_cinematicCamera->SetTarget(playerPos);
		}
	}

	// 演出が1フェーズ終わったら次へ
	if(_cinematicCamera->GetState() == CinematicCamera::State::Idle)
	{
		switch(_introPhase)
		{
		case IntroPhase::RotateForward:
		{
			_introPhase = IntroPhase::RotateBackward;

			const float rotateAngleRad = DX_PI_F * (35.0f / 180.0f);
			const float rotateDurationSec = 1.5f;
			const float rotateSpeed = rotateAngleRad / rotateDurationSec;

			_cinematicCamera->StartRotateSpeed(-rotateSpeed, rotateDurationSec);
			break;
		}

		case IntroPhase::RotateBackward:
		{
			_introPhase = IntroPhase::Zoom;

			// ズーム処理
			vec::Vec3 target = _cinematicCamera->GetTarget();
			vec::Vec3 currentPos = _cinematicCamera->GetPos();
			float currentDist = vec3::VSize(vec3::VSub(currentPos, target));
			float endDist = currentDist * 0.7f;
			// 最低距離を設定
			if(endDist < 80.0f)
			{
				endDist = 80.0f;
			}

			_cinematicCamera->StartZoom(target, 1.5f, currentDist, endDist, &mymath::EasingInBack);
			break;
		}

		case IntroPhase::Zoom:
		default:
		{
			_introPhase = IntroPhase::Done;
			EndIntroSequence();
			return true;
		}
		}
	}

	// 時間経過でイントロ終了
	_introTimer += 1.0f / 60.0f;
	if(_introTimer >= INTRO_DURATION)
	{
		_introPhase = IntroPhase::Done;
		EndIntroSequence();
		return true;
	}

	// ボタン入力でイントロ終了
	int trg = ApplicationBase::GetInstance()->GetTrg();
	if(!_introButtonPressed && (trg & PAD_INPUT_1))
	{
		_introButtonPressed = true;
	}

	if(_introButtonPressed)
	{
		_introPhase = IntroPhase::Done;
		EndIntroSequence();
		return true;
	}

	return true;
}

bool ModeGame::EndIntroSequence()
{
	if(!_isIntroActive)
	{
		return false;
	}

	_isIntroActive = false;
	_introButtonPressed = false;
	_introTimer = 0.0f;

	// プレイヤーの操作を再度有効化
	PlayerBase* tanuki = PlayerFactory::GetTanukiPlayer();
	if(tanuki)
	{
		tanuki->SetInputEnabled(true);
	}

	//カメラを戻す
	return EndCinematicSequence(true);
}

bool ModeGame::StartClearSequence()
{
	if(gGlobal._soundServer)
	{
		// 全BGMを停止（既存）
		gGlobal._soundServer->StopType(soundserver::SoundItemBase::TYPE::BGM);

		if(auto s = gGlobal._soundServer->Get("bgminitialize"))
		{
			s->Stop();
		}
		if(auto s2 = gGlobal._soundServer->Get("bgmChenge"))
		{
			s2->Stop();
		}

		// ゲームクリア演出用
		if(auto clearBgm = gGlobal._soundServer->Get("160"))
		{
			clearBgm->Play();
		}
	}

	_isGameClearCinematicActive = true;
	_clearCinematicTimer = 0.0f;
	_clearSequencePhase = 0; // 0: プレイヤー回転, 1: カメラズーム

	// ゲームクリア時は強制的にタヌキに変身 
	auto* playerManager = PlayerManager::GetInstance();
	if(playerManager->GetPlayerState() != PlayerManager::PlayerState::TANUKI)
	{
		PlayerBase* currentPlayer = nullptr;
		switch(playerManager->GetPlayerState())
		{
		case PlayerManager::PlayerState::HUMAN:
		{
			currentPlayer = PlayerFactory::GetHumanPlayer();
			break;
		}
		case PlayerManager::PlayerState::MONO:
		{
			currentPlayer = PlayerFactory::GetMonoPlayer();
			break;
		}
		default:
		{
			break;
		}
		}

		// タヌキに変身
		playerManager->TransformToTanuki();

		// 位置と向きを引き継ぐ
		if(currentPlayer)
		{
			PlayerBase* tanuki = PlayerFactory::GetTanukiPlayer();
			if(tanuki)
			{
				tanuki->SetPos(currentPlayer->GetPos());
				tanuki->SetDir(currentPlayer->GetDir());
				tanuki->_status = CharaBase::STATUS::WAIT; // 状態を待機にしておく
				tanuki->PlayAnimation("idle", true);
			}
		}
	}

	// シネマティックカメラが未作成の場合は作成
	if(!_cinematicCamera)
	{
		_cinematicCamera = std::make_unique<CinematicCamera>();
		if(!_cinematicCamera->Initialize())
		{
			_cinematicCamera.reset();
			return false;
		}
	}

	// カメラの切り替え処理
	if(!_useCinematicCamera)
	{
		if(_cinematicCamera && _camera)
		{
			// 演出カメラに現在のカメラ位置と注目点をコピー
			_cinematicCamera->SetPos(_camera->GetPos());
			_cinematicCamera->SetTarget(_camera->GetTarget());
			_cinematicCamera->SetClipNear(_camera->GetClipNear());
			_cinematicCamera->SetClipFar(_camera->GetClipFar());

			_savedCamera = _camera;
			_useCinematicCamera = true;
			_camera = _cinematicCamera.get();
		}
		else
		{
			return false;
		}
	}

	// プレイヤー回転演出を開始
	StartPlayerRotation();

	if(gGlobal._soundServer)
	{
		auto se = gGlobal._soundServer->Get("72");
		if(se)
		{
			se->Play();
		}
	}

	return true;
}

bool ModeGame::ProcessClearSequence()
{
	if(!_isGameClearCinematicActive)
	{
		return true;
	}

	PlayerBase* tanuki = PlayerFactory::GetTanukiPlayer();

	// フェーズ0: プレイヤー回転
	if(_clearSequencePhase == 0)
	{
		bool rotationComplete = ProcessPlayerRotation();

		if(rotationComplete)
		{
			// 回転終了後にクリア用モデルへ切り替え

			if(tanuki)
			{
				auto* tanukiPlayer = dynamic_cast<PlayerTanuki*>(tanuki);
				if(tanukiPlayer)
				{
					tanukiPlayer->SetGameClearHandle("clear", true);
				}
			}

			// 回転終了後、カメラズーム演出を開始
			_clearSequencePhase = 1;

			//　タヌキプレイヤーの位置でズーム演出を開始
			if(tanuki && _cinematicCamera)
			{
				vec::Vec3 playerPos = tanuki->GetPos();
				vec::Vec3 currentPos = _cinematicCamera->GetPos();
				float currentDist = vec3::VSize(vec3::VSub(currentPos, playerPos));
				float endDist = currentDist * 0.25f;
				if(endDist < 150.0f) endDist = 150.0f;

				_cinematicCamera->StartZoom(playerPos, GAMECLEAR::CLEAR_CINEMATIC_DURATION, currentDist, endDist);
			}
		}

		return true;
	}

	// フェーズ1: カメラズーム
	if(_clearSequencePhase == 1)
	{
		if(_cinematicCamera && _useCinematicCamera)
		{
			_cinematicCamera->Process();

			// タヌキプレイヤーの位置を追跡
			if(tanuki)
			{
				vec::Vec3 targetPos = vec3::VAdd(tanuki->GetPos(), vec3::VGet(0.0f, 60.0f, 0.0f));
				_cinematicCamera->SetTarget(targetPos);
			}
		}

		_clearCinematicTimer += 1.0f / 60.0f;

		if(_clearCinematicTimer >= GAMECLEAR::CLEAR_CINEMATIC_DURATION)
		{
			return EndClearSequence();
		}
	}

	return true;
}

bool ModeGame::EndClearSequence()
{
	_isGameClearCinematicActive = false;
	// ゲームクリアロード画面へ遷移
	ModeServer::GetInstance()->Add(new ModeGameClear(this), 255, "ModeGameClear");
	return true;
}

bool ModeGame::EndCinematicSequence(bool restoreToMainCamera)
{
	// 演出カメラを使用していない場合は何もしない
	if(!_useCinematicCamera || !_cinematicCamera)
	{
		return false;
	}

	// 演出カメラの停止
	_cinematicCamera->StopAll();

	// メインカメラに戻す処理
	if(restoreToMainCamera && _savedCamera)
	{
		// プレイヤー位置にカメラを再同期
		PlayerBase* startPlayer = PlayerForm::GetInstance()->GetPlayer();

		// プレイヤーの位置にカメラを再配置（必要に応じてオフセットを調整）
		if(startPlayer)
		{
			vec::Vec3 target = vec3::VAdd(startPlayer->GetPos(), vec3::VGet(0.0f, 60.0f, 0.0f));
			vec::Vec3 camDelta = vec3::VGet(0.0f, 1600.0f, -662.0f); // 元のオフセット（必要に応じて調整）
			_savedCamera->SetTarget(target);
			_savedCamera->SetPos(vec3::VAdd(target, camDelta));
		}

		// メインカメラに切り替え
		_camera = _savedCamera;

		// Map 側の参照も戻す
		if(_objectServer)
		{
			if(auto* map = _objectServer->GetMap())
			{
				map->SetCamera(_camera);
			}
		}

		// 次回の演出開始時に再保存できるようにクリア
		_savedCamera = nullptr;
	}

	_useCinematicCamera = false;
	return true;

	return true;
}

bool ModeGame::ProcessPlayerRotation()
{
	if(!_isPlayerRotating)
	{
		return false;
	}

	PlayerBase* targetPlayer = PlayerFactory::GetTanukiPlayer();

	if(!targetPlayer)
	{
		_isPlayerRotating = false; // プレイヤーが存在しない場合は回転演出を終了
		return true; // 回転終了
	}

	// 時間更新
	_playerRotationTimer += 1.0f / 60.0f; // 60FPS想定

	// 回転進捗度を計算（0.0〜1.0）
	float progress = _playerRotationTimer / _playerRotationDuration;

	// 回転が完了したかチェック
	if(progress >= 1.0f)
	{
		targetPlayer->SetTargetRotationY(_playerTargetRotation);
		targetPlayer->SetRotationY(_playerTargetRotation);
		_isPlayerRotating = false;

		targetPlayer->SetInputEnabled(true);
		return true; // 回転完了
	}

	// 最短距離で回転するための角度を計算
	float angleDiff = _playerTargetRotation - _playerInitialRotation;

	// 角度を-π〜πの範囲に正規化（最短経路を選択）
	while(angleDiff > DX_PI_F)
	{
		angleDiff -= DX_TWO_PI_F;
	}
	while(angleDiff < -DX_PI_F)
	{
		angleDiff += DX_TWO_PI_F;
	}

	// イージング
	float easedProgress = mymath::EasingInCubic
	(
		progress, // 現在の進捗度（0.0〜1.0）
		0.0f,	  // 開始値
		1.0f,	  // 終了値
		1.0f	  // 総時間

	);

	// 現在の回転角度を計算
	float currentRotation = _playerInitialRotation + (angleDiff * easedProgress);

	// 回転角度を設定
	targetPlayer->SetTargetRotationY(currentRotation);
	targetPlayer->SetRotationY(currentRotation);

	return false; // まだ回転中
}

bool ModeGame::StartGameOverSequence()
{
	if(_isGameOverCinematicActive)
	{
		return true;
	}
	if(gGlobal._soundServer)
	{
		// 全BGMを停止（既存）
		gGlobal._soundServer->StopType(soundserver::SoundItemBase::TYPE::BGM);
		if(auto s = gGlobal._soundServer->Get("bgminitialize"))
		{
			s->Stop();
		}
		if(auto s2 = gGlobal._soundServer->Get("bgmChenge"))
		{
			s2->Stop();
		}

		// ゲームオーバー演出用BGM
		if(auto overBgm = gGlobal._soundServer->Get("170"))
		{
			overBgm->Play();
		}
	}
	_isGameOverCinematicActive = true;
	_gameOverCinematicTimer = 0.0f;

	_gameOverDimAlpha = 0; // 画面暗転用のアルファ値

	// 0 = ズーム中, 1 = モデル切替＆アニメ, 2 = 余韻
	_gameOverSequencePhase = 0;

	auto* playerManager = PlayerManager::GetInstance();
	PlayerBase* currentPlayer = PlayerForm::GetInstance()->GetPlayer();

	// ゲームオーバー時は強制的にタヌキに変身
	PlayerBase* tanuki = PlayerFactory::GetTanukiPlayer();
	if(!tanuki)
	{
		_isGameOverCinematicActive = false;
		return false;
	}

	// タヌキプレイヤーに切り替え＆位置と向きを引き継ぐ
	if(currentPlayer && currentPlayer != tanuki)
	{
		tanuki->SetPos(currentPlayer->GetPos());
		tanuki->SetDir(currentPlayer->GetDir());
		tanuki->_status = CharaBase::STATUS::WAIT;
		tanuki->PlayAnimation("idle", true);
		tanuki->Process();
	}

	// タヌキに変身させる
	playerManager->TransformToTanuki();

	// 入力停止
	tanuki->SetInputEnabled(false);

	// シネマティックカメラ作成/切り替え
	if(!_cinematicCamera)
	{
		_cinematicCamera = std::make_unique<CinematicCamera>();
		if(!_cinematicCamera->Initialize())
		{
			_cinematicCamera.reset();
			_isGameOverCinematicActive = false;
			return false;
		}
	}

	// カメラ切り替え
	if(!_useCinematicCamera)
	{
		if(!_camera)
		{
			_isGameOverCinematicActive = false;
			return false;
		}
		_cinematicCamera->SetPos(_camera->GetPos());
		_cinematicCamera->SetTarget(_camera->GetTarget());
		_cinematicCamera->SetClipNear(_camera->GetClipNear());
		_cinematicCamera->SetClipFar(_camera->GetClipFar());

		_savedCamera = _camera;
		_useCinematicCamera = true;
		_camera = _cinematicCamera.get();
	}

	// ここで「ズーム開始」
	vec::Vec3 target = vec3::VAdd(tanuki->GetPos(), vec3::VGet(0.0f, 60.0f, 0.0f));
	vec::Vec3 camPos = _cinematicCamera->GetPos();

	// ターゲットとカメラの距離を計算して、ズームの開始距離と終了距離を決定
	float startDist = vec3::VSize(vec3::VSub(camPos, target));
	float endDist = startDist * 0.25f;
	if(endDist < 180.0f)endDist = 180.0f;

	_cinematicCamera->StartZoom(target, 0.8f, startDist, endDist);

	if(gGlobal._soundServer)
	{
		auto se = gGlobal._soundServer->Get("73");
		if(se)
		{
			se->Play();
		}
	}

	// ここでズーム開始
	return true;
}

bool ModeGame::ProcessGameOverSequence()
{
	if(!_isGameOverCinematicActive)
	{
		return false;
	}

	// 全体時間で終了
	if(_tanukiAttackAnimId != -1 && !AnimationManager::GetInstance()->IsPlaying(_tanukiAttackAnimId))
	{
		return EndGameOverSequence();
	}

	const int   targetAlpha = 180;          // 暗転アルファ値
	const float fadeSec = 0.5f;         // フェードインにかける時間
	const float dt = 1.0f / 60.0f; // フレーム時間

	if(_gameOverDimAlpha < targetAlpha)
	{
		const float add = (StCas<float>(targetAlpha) / fadeSec) * dt;
		_gameOverDimAlpha += add;
	}

	PlayerBase* tanuki = PlayerFactory::GetTanukiPlayer();

	if(_useCinematicCamera && _cinematicCamera)
	{
		_cinematicCamera->Process();

		// ターゲット追尾（ズームの中心がズレないようにする）
		if(tanuki)
		{
			vec::Vec3 t = vec3::VAdd(tanuki->GetPos(), vec3::VGet(0.0f, 60.0f, 0.0f));
			_cinematicCamera->SetTarget(t);
		}
	}

	_gameOverCinematicTimer += 1.0f / 60.0f;

	// ズームが終わったら、モデル切替＋アニメ開始
	if(_gameOverSequencePhase == 0 && _cinematicCamera && _cinematicCamera->GetState() == CinematicCamera::State::Idle)
	{
		_gameOverSequencePhase = 1;

		if(tanuki)
		{
			auto* tanukiPlayer = dynamic_cast<PlayerTanuki*>(tanuki);
			if(tanukiPlayer)
			{
				tanukiPlayer->SetGameOverHandle("tanuki_gameover", false);
			}
		}
	}

	// 倒れた状態のアニメーション(ループ)に切り替える
	if(_gameOverSequencePhase == 1 && tanuki)
	{
		auto* tanukiPlayer = dynamic_cast<PlayerTanuki*>(tanuki);
		if(tanukiPlayer)
		{
			bool shouldSwitch = false;

			int animId = tanukiPlayer->GetAnimId();
			if(animId != -1)
			{
				float total = AnimationManager::GetInstance()->GetTotalTime(animId);
				float play = AnimationManager::GetInstance()->GetPlayTime(animId);

				// DxLibのアニメ時間は「フレーム相当」のことが多いので、まずは 6.0f くらい前で切替
				float kLead = 6.0f;

				if(total > 0.0f && (total - play) <= kLead)
				{
					shouldSwitch = true;
				}
			}

			if(shouldSwitch || !tanukiPlayer->IsAnimationPlaying())
			{
				_gameOverSequencePhase = 2;
				tanukiPlayer->PlayGameOverAnimation("tanuki_taore", true);
			}
		}
	}

	// 倒れループに切り替わったことが確認できたら、ゲームオーバーへ
	if(_gameOverSequencePhase == 2 && tanuki)
	{
		auto* tanukiPlayer = dynamic_cast<PlayerTanuki*>(tanuki);
		if(tanukiPlayer && tanukiPlayer->IsAnimationPlaying())
		{
			return EndGameOverSequence();
		}
	}

	return true;
}

bool ModeGame::EndGameOverSequence()
{
	if(!_isGameOverCinematicActive)
	{
		return false;
	}

	_gameOverDimAlpha = 0; // 演出カメラの停止とリセット

	// プレイヤーの操作を再度有効化
	EndCinematicSequence(false);

	// ゲームオーバー画面へ遷移
	ModeServer::GetInstance()->Add(new ModeGameOver(this), 255, "ModeGameOver");
	return true;
}