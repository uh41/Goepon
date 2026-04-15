/*********************************************************************/
// * \file   playermanager.cpp
// * \brief  プレイヤー状態管理クラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#include "playermanager.h"
#include "PlayerFactory.h"
#include "effectmanager.h"
#include "playerform.h"
#include "appframe.h"
#include "playertanuki.h"
#include "applicationglobal.h"

PlayerManager* PlayerManager::GetInstance()
{
	static PlayerManager instance;
	return &instance;
}

bool PlayerManager::Initialize()
{
	_playerState = PlayerState::TANUKI; // 初期状態をタヌキに設定
	_requestedTransformToHuman = false;
	_requestedTransformToMono = false;
	_requestedTransformToTanuki = false;
	_transformTimeActive = false;
	_transformTimeLimit = 0.0f;
	_transformTimer = 0.0f;
	_blinkVisible = true;
	_blinkTimer = 0.0f;
	_blinkInterval = PlayerConstant::BLINK_INTERVAL;
	_transformAnimId = -1;

	return true;
}

bool PlayerManager::Terminate()
{
	// 何もしない
	return true;
}

void PlayerManager::TransformToHuman()
{
	auto tanuki = PlayerFactory::GetTanukiPlayer();
	auto human = PlayerFactory::GetHumanPlayer();
	if(!tanuki || !human)
	{
		return;
	}

	human->CopyStateFrom(tanuki);
	EffectManager::UpdatePlayerPosition(human);

	ApplyTransition(PlayerState::HUMAN);

	PlayerForm::GetInstance()->ChangeState(PlayerBase::PlayerType::HUMAN);

	_transformTimeActive = true;
	_transformTimeLimit = PlayerConstant::TRANSORM_TIME_LIMIT;
	_transformTimer = 0.0f;
	_blinkTimer = 0.0f;
	_blinkVisible = true;
}

void PlayerManager::TransformToTanuki()
{
	auto tanuki = PlayerFactory::GetTanukiPlayer();
	auto human = PlayerFactory::GetHumanPlayer();
	auto mono = PlayerFactory::GetMonoPlayer();
	if(!tanuki)
	{
		return;
	}

	// 現在の状態から状態を保存
	if(human && _playerState == PlayerState::HUMAN)
	{
		tanuki->CopyStateFrom(human);
	}
	else if(mono && _playerState == PlayerState::MONO)
	{
		tanuki->CopyStateFrom(mono);
	}

	EffectManager::UpdatePlayerPosition(tanuki);
	ApplyTransition(PlayerState::TANUKI);

	PlayerForm::GetInstance()->ChangeState(PlayerBase::PlayerType::TANUKI);

	_transformTimeActive = false;
	_transformTimeLimit = 0.0f;
	_transformTimer = 0.0f;
	_blinkTimer = 0.0f;
	_blinkVisible = true;
}

void PlayerManager::TransformToMono()
{
	auto tanuki = PlayerFactory::GetTanukiPlayer();
	auto mono = PlayerFactory::GetMonoPlayer();
	if(!tanuki || !mono)
	{
		return;
	}
	mono->CopyStateFrom(tanuki);
	EffectManager::UpdatePlayerPosition(mono);
	ApplyTransition(PlayerState::MONO);
	PlayerForm::GetInstance()->ChangeState(PlayerBase::PlayerType::MONO);
	_transformTimeActive = true;
	_transformTimeLimit = PlayerConstant::TRANSORM_TIME_LIMIT;
	_transformTimer = 0.0f;
	_blinkTimer = 0.0f;
	_blinkVisible = true;
}

PlayerManager::PlayerState PlayerManager::GetPlayerState()
{
	return _playerState;
}

bool PlayerManager::IsShowTanuki()
{
	return _playerState == PlayerState::TANUKI;
}

bool PlayerManager::IsShowHuman()
{
	return _playerState == PlayerState::HUMAN;
}

bool PlayerManager::IsShowMono()
{
	return _playerState == PlayerState::MONO;
}

bool PlayerManager::IsTransforming()
{
	return _transformAnimId != -1;
}

bool PlayerManager::IsTransformRequest()
{
	return _requestedTransformToHuman || _requestedTransformToMono || _requestedTransformToTanuki;
}

void PlayerManager::SetTransformTimeLimit(float second)
{
	_transformTimeLimit = second;
	_transformTimer = 0.0f;
	_transformTimeActive = true;
}

float PlayerManager::GetTransformTimeLimit()
{
	return _transformTimeLimit;
}

void PlayerManager::UpdateTransformTimer(float dt)
{
	if(!_transformTimeActive)
	{
		return;
	}

	_transformTimer += dt;
	_transformTimeLimit -= dt;

	// タイムリミットが0以下になったらタヌキに変身する
	if(_transformTimeLimit <= 0.0f)
	{
		_transformTimeActive = false;
		_transformTimeLimit = 0.0f;
		TransformToTanuki();
	}

	UpdateBlinkTimer(dt);
}

bool PlayerManager::IsTransformTimeLimitActive()
{
	return _transformTimeActive;
}

void PlayerManager::RequestTransformToHuman()
{
	_requestedTransformToHuman = true;
}

void PlayerManager::RequestTransformToMono()
{
	_requestedTransformToMono = true;
}

void PlayerManager::RequestTransformToTanuki()
{
	_requestedTransformToTanuki = true;
}

void PlayerManager::CancelTransformRequest()
{
	if(IsTransforming())
	{
		return;
	}
	_requestedTransformToHuman = false;
	_requestedTransformToMono = false;
	_requestedTransformToTanuki = false;
}

void PlayerManager::StartTransformAnimation()
{
	if(IsTransforming())
	{
		return;
	}

	if(!IsTransformRequest())
	{
		return;
	}

	auto* tanuki = dynamic_cast<PlayerTanuki*>(PlayerFactory::GetTanukiPlayer());
	if(!tanuki)
	{
		return;
	}

	int animId = tanuki->PlayAnimation("henge", false);
	SetTransformAnimation(animId);

	// 変身音を再生
	auto henshinSound = gGlobal._soundServer->Get("2");
	if(henshinSound && !henshinSound->IsPlay())
	{
		henshinSound->Play();
	}
}

void PlayerManager::CompleteTransform()
{
	// アニメーションIDをリセット
	_transformAnimId = -1;

	// リクエストに基づいて変身を完了
	if(_requestedTransformToMono)
	{
		_requestedTransformToMono = false;
		TransformToMono();
		return;
	}

	if(_requestedTransformToHuman)
	{
		_requestedTransformToHuman = false;
		TransformToHuman();
		return;
	}

	if(_requestedTransformToTanuki)
	{
		_requestedTransformToTanuki = false;
		TransformToTanuki();
		return;
	}
}

void PlayerManager::ProcessTransformRequest()
{
	// 変身アニメーション再生中は処理しない
	if(IsTransforming())
	{
		return;
	}

	// リクエストがない場合は処理しない
	if(!IsTransformRequest())
	{
		return;
	}

	// モノへの変身リクエスト
	if(_requestedTransformToMono)
	{
		_requestedTransformToMono = false;
		TransformToMono();
		return;
	}

	// 人間への変身リクエスト
	if(_requestedTransformToHuman)
	{
		_requestedTransformToHuman = false;
		TransformToHuman();
		return;
	}

	// タヌキへの変身リクエスト
	if(_requestedTransformToTanuki)
	{
		_requestedTransformToTanuki = false;
		TransformToTanuki();
		return;
	}
}

bool PlayerManager::GetBlinkVisible()
{
	return _blinkVisible;
}

void PlayerManager::UpdateBlinkTimer(float dt)
{
	if(!_transformTimeActive)
	{
		return;
	}
	_blinkTimer += dt;

	float blinkInterval = _blinkInterval;
	
	if(_transformTimeLimit <= PlayerConstant::BLINK_SPEED_THRESHOLD_2)
	{
		blinkInterval *= PlayerConstant::BLINK_SPEED_4X;
	}
	else if(_transformTimeLimit <= PlayerConstant::BLINK_SPEED_THRESHOLD_1)
	{
		blinkInterval *= PlayerConstant::BLINK_SPEED_2X;
	}

	if(_blinkTimer >= blinkInterval)
	{
		_blinkTimer = 0.0f;
		_blinkVisible = !_blinkVisible;
	}
}

void PlayerManager::SetInitialPlayerState(PlayerState state)
{
	_playerState = state;
}

void PlayerManager::SetTransformAnimation(int animId)
{
	_transformAnimId = animId;
}

int PlayerManager::GetTransformAnimation()
{
	return _transformAnimId;
}

bool PlayerManager::IsTransformAnimationPlaying()
{
	if(_transformAnimId == -1)
	{
		return false;
	}
	auto am = AnimationManager::GetInstance();
	if(!am)
	{
		return false;
	}

	return am->IsPlaying(_transformAnimId);
}

void PlayerManager::TransformToTanukiImmediate()
{
	auto tanuki = PlayerFactory::GetTanukiPlayer();
	auto human = PlayerFactory::GetHumanPlayer();
	auto mono = PlayerFactory::GetMonoPlayer();
	if(!tanuki)
	{
		return;
	}

	// 現在の状態から状態を保存
	if(human && _playerState == PlayerState::HUMAN)
	{
		tanuki->CopyStateFrom(human);
	}
	else if(mono && _playerState == PlayerState::MONO)
	{
		tanuki->CopyStateFrom(mono);
	}

	EffectManager::UpdatePlayerPosition(tanuki);
	ApplyTransition(PlayerState::TANUKI);

	PlayerForm::GetInstance()->ChangeState(PlayerBase::PlayerType::TANUKI);
	_transformTimeActive = false;
	_transformTimeLimit = 0.0f;
	_transformTimer = 0.0f;
	_blinkTimer = 0.0f;
	_blinkVisible = true;
	_transformAnimId = -1;
	_requestedTransformToTanuki = false;
}

void PlayerManager::ApplyTransition(PlayerState newState)
{
	_playerState = newState;
}