#include "modegame.h"
#include "ModeGameClear.h"

bool ModeGame::DebugCinematicCameraControl()
{
	if (!_cinematicCamera)
	{
		return false; // 演出カメラが存在しない場合は処理しない
	}

	if (!_originalCamera)
	{
		return false;
	}

	if (!_camera)
	{
		return false; // 現在のカメラが存在しない場合は処理しない
	}

	if (CheckHitKey(KEY_INPUT_F1))
	{
		if (!_debugF1KeyPressed)
		{
			_debugF1KeyPressed = true;
			if (!_debugZoomActive)
			{
				_debugZoomActive = true;
				// 演出カメラに現在のカメラ位置と注目点をコピーして切り替え
				if (_cinematicCamera && _camera)
				{
					// 演出カメラに現在のカメラ位置と注目点をコピーして切り替え
					_cinematicCamera->SetPos(_camera->GetPos());
					_cinematicCamera->SetTarget(_camera->GetTarget());
					_cinematicCamera->SetClipNear(_camera->GetClipNear());
					_cinematicCamera->SetClipFar(_camera->GetClipFar());

					_useCinematicCamera = true;
					_camera = _cinematicCamera.get();
				}
				else
				{
					// カメラが無効な場合は処理をスキップ
					return false;
				}

				// 現在のプレイヤー位置を取得してズーム演出
				PlayerBase* targetPlayer = nullptr;
				if (_bShowTanuki && _playerTanuki && _playerTanuki->IsAlive())
				{
					targetPlayer = _playerTanuki.get();
				}
				else if (_showMonoPlayer && _playerMono && _playerMono->IsAlive())
				{
					targetPlayer = _playerMono.get();
				}
				else if (_player && _player->IsAlive())
				{
					targetPlayer = _player.get();
				}
				if (targetPlayer && _cinematicCamera)
				{
					vec::Vec3 target = targetPlayer->GetPos();
					// 現在位置のカメラの位置からプレイヤーの位置への距離を計算
					vec::Vec3 currentPos = _cinematicCamera->GetPos();
					float currentDist = vec3::VSize(vec3::VSub(currentPos, target));
					float endDist = currentDist * 0.25f; // 最終的な距離（半分にする例）
					if (endDist < 50.0f) endDist = 50.0f; // 最小距離を設定（近すぎないように）

					// ズーム演出：現在距離から近距離へ
					_cinematicCamera->StartZoom(target, 0.5f, currentDist, endDist); // ターゲット位置、ズーム倍率、ズーム距離、ズーム時間
				}
			}
			else
			{

				// ズーム解除
				_debugZoomActive = false;
				_useCinematicCamera = false;

				if (_cinematicCamera)
				{
					_cinematicCamera->StopAll();
				}

				// 元のカメラに戻す前に有効性をチェック
				if (_originalCamera)
				{
					_camera = _originalCamera;
				}
			}
		}
	}
	else
	{
		_debugF1KeyPressed = false;
	}
	return true;
}

bool ModeGame::TreasureOpeningCameraControl()
{
	if (_isOpeningTreasure)
	{
		return false; // またはすでに開いている場合は処理しない
	}

	// シネマティックカメラが初期化されていない場合は作成
	if (!_cinematicCamera)
	{
		_cinematicCamera = std::make_unique<CinematicCamera>();
		if (!_cinematicCamera->Initialize())
		{
			_cinematicCamera.reset();
			return false;
		}
	}

	// 元のカメラを保存
	if (!_useCinematicCamera)
	{
		if (_cinematicCamera && _camera)
		{
			// 演出カメラに現在のカメラ位置と注目点をコピーして切り替え
			_cinematicCamera->SetPos(_camera->GetPos());
			_cinematicCamera->SetTarget(_camera->GetTarget());
			_cinematicCamera->SetClipNear(_camera->GetClipNear());
			_cinematicCamera->SetClipFar(_camera->GetClipFar());

			_originalCamera = _camera;
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
		if (_bShowTanuki && _playerTanuki && _playerTanuki->IsAlive())
		{
			targetPlayer = _playerTanuki.get();
		}
		else if (_showMonoPlayer && _playerMono && _playerMono->IsAlive())
		{
			targetPlayer = _playerMono.get();
		}
		else if (_player && _player->IsAlive())
		{
			targetPlayer = _player.get();
		}

		if (targetPlayer && _cinematicCamera)
		{
			vec::Vec3 target = targetPlayer->GetPos();
			vec::Vec3 currentPos = _cinematicCamera->GetPos();
			float currentDist = vec3::VSize(vec3::VSub(currentPos, target));
			float endDist = currentDist * 0.5f; // 最終的な距離（半分にする例）
			if (endDist < 50.0f) endDist = 50.0f; // 最小距離を設定（近すぎないように）

			// ズーム演出：現在距離から近距離へ
			_cinematicCamera->StartZoom(target, 0.5f, currentDist, endDist);
		}
	}
	return true;
}

bool ModeGame::EndCinematicCamera()
{
	if (!_useCinematicCamera || !_cinematicCamera)
	{
		return false;
	}

	// 元のカメラに戻す
	if (_originalCamera)
	{
		_camera = _originalCamera;
		_originalCamera = nullptr;
	}

	_useCinematicCamera = false;

	//　シネマティックカメラをリセット
	_cinematicCamera->StopAll();
	return true;
}


// イントロ演出開始関数を追加
bool ModeGame::StartIntroSequence()
{
	// カメラが初期化されていない場合は失敗
	if (!_camera)
	{
		return false;
	}

	// 演出カメラが未作成の場合は作成
	if (!_cinematicCamera)
	{
		_cinematicCamera = std::make_unique<CinematicCamera>();
		if (!_cinematicCamera->Initialize())
		{
			_cinematicCamera.reset();
			return false;
		}
	}

	// 元のカメラを保持して演出カメラに切り替え
	if (!_useCinematicCamera)
	{
		_originalCamera = _camera;
		_camera = _cinematicCamera.get();
		_useCinematicCamera = true;
	}

	// プレイヤーの取得（タヌキ優先）
	PlayerBase* targetPlayer = nullptr;
	if (_bShowTanuki && _playerTanuki)
	{
		targetPlayer = _playerTanuki.get();
	}
	else if (_player)
	{
		targetPlayer = _player.get();
	}

	if (targetPlayer && _cinematicCamera)
	{
		vec::Vec3 playerPos = targetPlayer->GetPos();
		vec::Vec3 playerDir = targetPlayer->GetDir();

		// プレイヤーの向きを正規化
		float dirLength = vec3::VSize(playerDir);
		if (dirLength < 0.001f)
		{
			playerDir = vec3::VGet(0.0f, 0.0f, 1.0f);
		}
		else
		{
			playerDir = vec3::VNorm(playerDir);
		}

		// ★調整パラメータ（ここだけ変更すればOK）★
		float cameraHeight = 500.0f; // カメラの高さ（Y軸のみ）
		float cameraDistance = 500.0f; // プレイヤーからの距離
		float targetHeight = 300.0f; // プレイヤーの顔の高さ

		// カメラ位置（プレイヤーの正面、指定した高さと距離）
		vec::Vec3 cameraPos = vec3::VGet(
			playerPos.x + playerDir.x * cameraDistance,
			playerPos.y + cameraHeight,
			playerPos.z + playerDir.z * cameraDistance
		);

		// カメラターゲット（プレイヤーの顔）
		vec::Vec3 cameraTarget = vec3::VAdd(playerPos, vec3::VGet(0.0f, targetHeight, 0.0f));

		// カメラ設定を適用
		_cinematicCamera->SetPos(cameraPos);
		_cinematicCamera->SetTarget(cameraTarget);
		_cinematicCamera->SetClipNear(1.0f);
		_cinematicCamera->SetClipFar(10000.0f);
	}

	// イントロ演出を開始
	_isIntroActive = true;
	_introButtonPressed = false;
	_introTimer = 0.0f;

	// ★追加: プレイヤーの操作を無効化
	if (_playerTanuki)
	{
		_playerTanuki->SetInputEnabled(false);
	}

	return true;
}

// ProcessIntroSequence()関数を追加
bool ModeGame::ProcessIntroSequence()
{
	if (!_isIntroActive)
	{
		return false;
	}

	PlayerTanuki* tanuki = _playerTanuki.get();
	if (tanuki && tanuki->IsAlive())
	{
		vec::Vec3 playerPos = tanuki->GetPos();

		_cinematicCamera->SetTarget(playerPos);
	}

	// ボタン入力でイントロ終了
	int trg = ApplicationBase::GetInstance()->GetTrg();
	if (!_introButtonPressed)
	{
		if (trg & PAD_INPUT_1)
		{
			_introButtonPressed = true;
		}
	}

	if (_introButtonPressed)
	{
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

	// シネマティックカメラが未作成の場合は作成
	if (!_cinematicCamera)
	{
		_cinematicCamera = std::make_unique<CinematicCamera>();
		if (!_cinematicCamera->Initialize())
		{
			_cinematicCamera.reset();
			return false;
		}
	}

	// ★ カメラの切り替え処理を追加（イントロと同様の処理）
	if (!_useCinematicCamera)
	{
		if (_cinematicCamera && _camera)
		{
			// 演出カメラに現在のカメラ位置と注目点をコピー
			_cinematicCamera->SetPos(_camera->GetPos());
			_cinematicCamera->SetTarget(_camera->GetTarget());
			_cinematicCamera->SetClipNear(_camera->GetClipNear());
			_cinematicCamera->SetClipFar(_camera->GetClipFar());

			_originalCamera = _camera;
			_useCinematicCamera = true;
			_camera = _cinematicCamera.get();
		}
		else
		{
			return false;
		}
	}

	// プレイヤーを取得
	PlayerBase* targetPlayer = nullptr;
	if (_bShowTanuki && _playerTanuki)
	{
		targetPlayer = _playerTanuki.get();
	}
	else if (_showMonoPlayer && _playerMono)
	{
		targetPlayer = _playerMono.get();
	}
	else if (_player)
	{
		targetPlayer = _player.get();
	}

	if (targetPlayer && _cinematicCamera)
	{
		vec::Vec3 playerPos = targetPlayer->GetPos();

		// 現在のカメラ位置からプレイヤーまでの距離を計算
		vec::Vec3 currentPos = _cinematicCamera->GetPos();
		float currentDist = vec3::VSize(vec3::VSub(currentPos, playerPos));

		// ズーム後の距離（現在の距離の25%程度に設定してより近づける）
		float endDist = currentDist * 0.25f;
		if (endDist < 150.0f) endDist = 150.0f; // 最小距離を設定（近すぎないように）

		// ★ CinematicCamera::StartZoom() を使用してズーム演出を開始
		_cinematicCamera->StartZoom(playerPos, CLEAR_CINEMATIC_DURATION, currentDist, endDist);
	}

	return true;
}

bool ModeGame::ProcessClearSequence()
{
	if (!_isGameClearCinematicActive) 
	{
		return true;
	}

	// ★ CinematicCamera::Process() を呼び出してズーム演出を更新
	if (_cinematicCamera && _useCinematicCamera)
	{
		_cinematicCamera->Process();

		// プレイヤーの位置を追跡（プレイヤーが動いた場合に対応）
		PlayerBase* targetPlayer = nullptr;
		if (_bShowTanuki && _playerTanuki)
		{
			targetPlayer = _playerTanuki.get();
		}
		else if (_showMonoPlayer && _playerMono)
		{
			targetPlayer = _playerMono.get();
		}
		else if (_player)
		{
			targetPlayer = _player.get();
		}

		// ターゲット位置を更新（プレイヤーの顔の高さに合わせる）
		if (targetPlayer)
		{
			vec::Vec3 targetPos = vec3::VAdd(targetPlayer->GetPos(), vec3::VGet(0.0f, 60.0f, 0.0f));
			_cinematicCamera->SetTarget(targetPos);
		}
	}

	_clearCinematicTimer += 1.0f / 60.0f;

	// 演出が終了したら
	if (_clearCinematicTimer >= CLEAR_CINEMATIC_DURATION)
	{
		return EndClearSequence();
	}

	return true;
}

bool ModeGame::EndClearSequence()
{
	_isGameClearCinematicActive = false;
	// 演出カメラを停止
	if (_cinematicCamera)
	{
		_cinematicCamera->StopAll();
	}

	// 元のカメラに戻す
	if (_useCinematicCamera && _originalCamera)
	{
		_camera = _originalCamera;
		_originalCamera = nullptr;
		_useCinematicCamera = false;
	}

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
	if(restoreToMainCamera && _originalCamera)
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
			_originalCamera->SetTarget(target);
			_originalCamera->SetPos(vec3::VAdd(target, camDelta));
		}

		// メインカメラに切り替え
		_camera = _originalCamera;
		
	}
	_useCinematicCamera = false;

	return true;
}
