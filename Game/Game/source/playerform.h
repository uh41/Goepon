/*********************************************************************/
// * \file   playerform.h
// * \brief  プレイヤーの状態変化クラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#pragma once
#include "playerbase.h"
#include "appframe.h"
#include "savemanager.h"

class PlayerForm
{
public:
	static PlayerForm* GetInstance();

	bool Initialize();
	bool Terminate();

	PlayerBase* GetPlayer() const;
	PlayerBase::PlayerType GetPlayerType() const { return _playerType; }

	void ChangeState(PlayerBase::PlayerType type);
	void RestoreFromSaveData(const SaveData& saveData);
	void TransformForEnemyDetetion(PlayerBase::PlayerType type, const vec::Vec3& pos, const vec::Vec3& dir);

private:
	PlayerForm() = default;
	~PlayerForm() = default;

	PlayerForm(const PlayerForm&) = delete;
	PlayerForm& operator=(const PlayerForm&) = delete;

	PlayerBase::PlayerType _playerType = PlayerBase::PlayerType::TANUKI;
};

