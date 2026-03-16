#include "modegame.h"
#include "ModeGameClear.h"
#include "mymath.h"
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

	// --- F1 : Zoom（既存）---
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
				if(_bShowTanuki && _playerTanuki && _playerTanuki->IsAlive())
				{
					targetPlayer = _playerTanuki.get();
				}
				else if(_showMonoPlayer && _playerMono && _playerMono->IsAlive())
				{
					targetPlayer = _playerMono.get();
				}
				else if(_player && _player->IsAlive())
				{
					targetPlayer = _player.get();
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
			else
			{
				// ズーム解除
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

	// --- F2 : Shake（追加）---
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

				// 揺れ開始（既存APIを利用）
				const float intensity = 50.0f;   // 揺れ幅（適宜調整）
				const float durationSec = 0.50f; // 揺れ時間（適宜調整）
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
		}
		else
		{
			// カメラがない場合は処理をスキップ
			return false;
		}

		// 現在のプレイヤー位置を取得
		PlayerBase* targetPlayer = nullptr;
		if(_bShowTanuki && _playerTanuki && _playerTanuki->IsAlive())
		{
			targetPlayer = _playerTanuki.get();
		}
		else if(_showMonoPlayer && _playerMono && _playerMono->IsAlive())
		{
			targetPlayer = _playerMono.get();
		}
		else if(_player && _player->IsAlive())
		{
			targetPlayer = _player.get();
		}

		if(targetPlayer && _cinematicCamera)
		{
			vec::Vec3 target = targetPlayer->GetPos();
			vec::Vec3 currentPos = _cinematicCamera->GetPos();
			float currentDist = vec3::VSize(vec3::VSub(currentPos, target));
			float endDist = currentDist * 0.5f; // 最終的な距離（半分にする例）
			if(endDist < 50.0f) endDist = 50.0f; // 最小距離を設定（近すぎないように）

			// ズーム演出：現在距離から近距離へ
			_cinematicCamera->StartZoom(target, 0.5f, currentDist, endDist);
		}
	}
	return true;
}

bool ModeGame::EndCinematicCamera()
{
	// シネマティックカメラが使用されていない場合は処理しない
	if(!_useCinematicCamera || !_cinematicCamera)
	{
		return false;
	}

	// 元のカメラに戻す
	if(_savedCamera)
	{
		_camera = _savedCamera;
		_savedCamera = nullptr;
	}

	_useCinematicCamera = false;

	//　シネマティックカメラをリセット
	_cinematicCamera->StopAll();
	return true;
}


//// イントロ演出開始関数を追加
//bool ModeGame::StartIntroSequence()
//{
//	// カメラが初期化されていない場合は失敗
//	if (!_camera)
//	{
//		return false;
//	}
//
//	// 演出カメラが未作成の場合は作成
//	if (!_cinematicCamera)
//	{
//		_cinematicCamera = std::make_unique<CinematicCamera>();
//		if (!_cinematicCamera->Initialize())
//		{
//			_cinematicCamera.reset();
//			return false;
//		}
//	}
//
//	// 元のカメラを保持して演出カメラに切り替え
//	if (!_useCinematicCamera)
//	{
//		_savedCamera = _camera;
//		_camera = _cinematicCamera.get();
//		_useCinematicCamera = true;
//	}
//
//	// プレイヤーの取得（タヌキ優先）
//	PlayerBase* targetPlayer = nullptr;
//	if (_bShowTanuki && _playerTanuki)
//	{
//		targetPlayer = _playerTanuki.get();
//	}
//	else if (_player)
//	{
//		targetPlayer = _player.get();
//	}
//
//	// プレイヤーの位置と向きを考慮してカメラを配置
//	if (targetPlayer && _cinematicCamera)
//	{
//		vec::Vec3 playerPos = targetPlayer->GetPos(); // プレイヤーの位置を取得
//
//		// プレイヤーの回転角度を取得
//		float playerRotY = targetPlayer->GetRotationY();
//
//		// プレイヤーの前方向ベクトルを計算
//		vec::Vec3 playerDir;
//		playerDir.x = sinf(playerRotY);
//		playerDir.y = 0.0f;
//		playerDir.z = cosf(playerRotY);
//
//		// 念のため正規化
//		float dirLength = vec3::VSize(playerDir);
//
//		if (dirLength > 0.001f)
//		{
//			playerDir = vec3::VNorm(playerDir);
//		}
//
//		// 調整パラメータ 
//		float cameraDistance = 300.0f;  // プレイヤーからカメラまでの距離
//		float cameraHeight = 500.0f;    // カメラの高さオフセット
//		float targetHeight = 60.0f;     // プレイヤーの顔の高さ（注視点）
//
//		// プレイヤーの前方ににカメラを配置（本来はすべてプラスだと思うがDxlibの使用？ためマイナス）
//		vec::Vec3 cameraPos = vec3::VGet
//		(
//			playerPos.x - playerDir.x * cameraDistance,  // プレイヤーの前方に配置
//			playerPos.y + cameraHeight,                  // プレイヤーの高さにオフセットを加える
//			playerPos.z - playerDir.z * cameraDistance 
//		);
//
//		// カメラターゲット：プレイヤーの顔の位置
//		vec::Vec3 cameraTarget = vec3::VAdd(playerPos, vec3::VGet(0.0f, targetHeight, 0.0f));
//
//		// カメラ設定を適用
//		_cinematicCamera->SetPos(cameraPos);
//		_cinematicCamera->SetTarget(cameraTarget);
//		_cinematicCamera->SetClipNear(1.0f);
//		_cinematicCamera->SetClipFar(10000.0f);
//	}
//
//	// イントロ演出を開始
//	_isIntroActive = true;
//	_introButtonPressed = false;
//	_introTimer = 0.0f;
//
//	// プレイヤーの操作を無効化
//	if (_playerTanuki)
//	{
//		_playerTanuki->SetInputEnabled(false);
//	}
//
//	return true;
//}
//
//
//// ProcessIntroSequence()関数を追加
//bool ModeGame::ProcessIntroSequence()
//{
//	if(!_isIntroActive)
//	{
//		return false;
//	}
//
//
//	PlayerTanuki* tanuki = _playerTanuki.get();
//	if(tanuki && tanuki->IsAlive())
//	{
//		vec::Vec3 playerPos = tanuki->GetPos();
//
//		_cinematicCamera->SetTarget(playerPos);
//	}
//
//	// 時間経過でイントロ終了
//	_introTimer += 1.0f / 60.0f; // 60FPS想定
//
//	// イントロ演出の総時間（INTRO_DURATION）を超えたら終了
//	if(_introTimer >= INTRO_DURATION)
//	{
//		EndIntroSequence();
//		return true;
//	}
//
//	// ボタン入力でイントロ終了
//	int trg = ApplicationBase::GetInstance()->GetTrg();
//	if(!_introButtonPressed)
//	{
//		if(trg & PAD_INPUT_1)
//		{
//			_introButtonPressed = true;
//		}
//	}
//
//	if(_introButtonPressed)
//	{
//		EndIntroSequence();
//		return true;
//	}
//
//	return true;
//}

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
	_isIntroActive      = true;
	_introButtonPressed = false;
	_introTimer         = 0.0f;

	PlayerTanuki* targetPlayer = nullptr;
	if(_playerTanuki && _playerTanuki->IsAlive())
	{
		targetPlayer = _playerTanuki.get();
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
		const float cameraHeight   = 500.0f;
		const float targetHeight   = 60.0f;

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

	// Rotate は _vTarget 周りに回る＝ここで SetTarget 済みの「顔位置」中心になる
	const float rotateAngleRad = DX_PI_F * (35.0f / 180.0f);
	const float rotateDurationSec = 1.5f;
	const float rotateSpeed = rotateAngleRad / rotateDurationSec;
	_cinematicCamera->StartRotateSpeed(rotateSpeed, rotateDurationSec);

	// プレイヤーの操作を止める
	if(_playerTanuki)
	{
		_playerTanuki->SetInputEnabled(false);
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

	// イントロ中はプレイヤー位置を追従（ただし回転中心が動くと軌道が乱れるので注意）
	// ここは「回転フェーズ中は固定 / ズーム中だけ追従」がおすすめ
	PlayerTanuki* tanuki = _playerTanuki.get();
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

				// ズーム（現在距離→少し寄る）
				vec::Vec3 target = _cinematicCamera->GetTarget();
				vec::Vec3 currentPos = _cinematicCamera->GetPos();
				float currentDist = vec3::VSize(vec3::VSub(currentPos, target));
				float endDist = currentDist * 0.70f;
				if(endDist < 80.0f)
				{
					endDist = 80.0f;
				}

				_cinematicCamera->StartZoom(target, 0.70f, currentDist, endDist);
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

	// 時間経過でイントロ終了（保険）
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
// EndIntroSequence()関数を追加
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
	if(_playerTanuki)
	{
		_playerTanuki->SetInputEnabled(true);
	}

	// 汎用関数でカメラを戻す
	return EndCinematicSequence(true); // true = メインカメラに戻す
}

bool ModeGame::StartClearSequence()
{
	_isGameClearCinematicActive = true;
	_clearCinematicTimer = 0.0f;
	_clearSequencePhase = 0; // 0: プレイヤー回転, 1: カメラズーム

	// ゲームクリア時は強制的にタヌキに変身 
	if(!_bShowTanuki)
	{
		// 現在表示中のプレイヤーの位置と向きを保存
		PlayerBase* currentPlayer = nullptr;
		if(_showMonoPlayer && _playerMono)
		{
			currentPlayer = _playerMono.get();
		}
		else if(_player)
		{
			currentPlayer = _player.get();
		}

		// タヌキに切り替え
		_bShowTanuki = true;
		_showMonoPlayer = false;

		// 位置と向きを引き継ぐ
		if(currentPlayer && _playerTanuki)
		{
			_playerTanuki->SetPos(currentPlayer->GetPos());
			_playerTanuki->SetDir(currentPlayer->GetDir());
			_playerTanuki->_status = CharaBase::STATUS::WAIT;
			_playerTanuki->PlayAnimation("idle", true);
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

	return true;
}

bool ModeGame::ProcessClearSequence()
{
	if(!_isGameClearCinematicActive)
	{
		return true;
	}

	// フェーズ0: プレイヤー回転
	if(_clearSequencePhase == 0)
	{
		bool rotationComplete = ProcessPlayerRotation();

		if(rotationComplete)
		{
			// 回転終了後にクリア用モデルへ切り替え
			if(_playerTanuki)
			{
				_playerTanuki->SetClearHandle("clear", true); // ← ここを使いたいアニメ名に
			}

			// 回転終了後、カメラズーム演出を開始
			_clearSequencePhase = 1;

			// ★★★ タヌキプレイヤーの位置でズーム演出を開始 ★★★
			if(_playerTanuki && _cinematicCamera)
			{
				vec::Vec3 playerPos = _playerTanuki->GetPos();
				vec::Vec3 currentPos = _cinematicCamera->GetPos();
				float currentDist = vec3::VSize(vec3::VSub(currentPos, playerPos));
				float endDist = currentDist * 0.25f;
				if(endDist < 150.0f) endDist = 150.0f;

				_cinematicCamera->StartZoom(playerPos, CLEAR_CINEMATIC_DURATION, currentDist, endDist);
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

			// ★★★ タヌキプレイヤーの位置を追跡 ★★★
			if(_playerTanuki)
			{
				vec::Vec3 targetPos = vec3::VAdd(_playerTanuki->GetPos(), vec3::VGet(0.0f, 60.0f, 0.0f));
				_cinematicCamera->SetTarget(targetPos);
			}
		}

		_clearCinematicTimer += 1.0f / 60.0f;

		if(_clearCinematicTimer >= CLEAR_CINEMATIC_DURATION)
		{
			return EndClearSequence();
		}
	}

	return true;
}

bool ModeGame::EndClearSequence()
{
	_isGameClearCinematicActive = false;
	//// 演出カメラを停止
	//if (_cinematicCamera)
	//{
	//	_cinematicCamera->StopAll();
	//}

	//// 元のカメラに戻す
	//if (_useCinematicCamera && _originalCamera)
	//{
	//	_camera = _originalCamera;
	//	_originalCamera = nullptr;
	//	_useCinematicCamera = false;
	//}

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
		PlayerBase* startPlayer = nullptr;
		if(_bShowTanuki && _playerTanuki)
		{
			startPlayer = _playerTanuki.get();
		}
		else if(_showMonoPlayer && _playerMono)
		{
			startPlayer = _playerMono.get();
		}
		else if(_player)
		{
			startPlayer = _player.get();
		}

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

	}
	_useCinematicCamera = false;

	return true;
}

bool ModeGame::StartPlayerRotation()
{
	// ★★★ タヌキプレイヤーのみを対象に回転演出を初期化 ★★★
	PlayerTanuki* targetPlayer = _playerTanuki.get();

	if(!targetPlayer)
	{
		return false; // タヌキプレイヤーが存在しない場合は処理しない
	}

	// 回転演出の初期化
	_isPlayerRotating = true;
	_playerRotationTimer = 0.0f;
	_playerRotationDuration = 2.0f; // 回転演出の総時間（秒）

	// 現在の回転角度を取得
	_playerInitialRotation = targetPlayer->GetRotationY();

	// 目標角度: dir.z = 1 の方向 = 0度（正面）
	_playerTargetRotation = 0.0f;

	// タヌキプレイヤーの操作を無効化
	targetPlayer->SetInputEnabled(false);

	return true;
}

bool ModeGame::ProcessPlayerRotation()
{
	if(!_isPlayerRotating)
	{
		return false;
	}

	PlayerTanuki* targetPlayer = _playerTanuki.get();

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
		// 回転完了 - 目標角度に正確にセット
		targetPlayer->SetTargetRotationY(_playerTargetRotation);
		targetPlayer->SetRotationY(_playerTargetRotation);
		_isPlayerRotating = false;

		// 狸プレイヤーの操作を再度有効化
		targetPlayer->SetInputEnabled(true);
		return true; // 回転完了
	}

	// 最短経路で回転するため角度差を計算
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

	// イージング関数を適用（滑らかな回転）
	float easedProgress = mymath::EasingInCubic
	(
		progress, // 現在の進捗度（0.0〜1.0）
		0.0f,	  // 開始値（0%）
		1.0f,	  // 終了値（100%）
		1.0f	  // 総時間（1.0で正規化）

	);

	// 現在の回転角度を計算
	float currentRotation = _playerInitialRotation + (angleDiff * easedProgress);

	// 回転角度を設定
	targetPlayer->SetTargetRotationY(currentRotation);
	targetPlayer->SetRotationY(currentRotation);

	return false; // まだ回転中
}