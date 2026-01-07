/*********************************************************************/
// * \file   enemy.cpp
// * \brief  エネミークラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "enemy.h"

// 初期化
bool Enemy::Initialize()
{
	base::Initialize();

	_iHandle = MV1LoadModel("res/PoorEnemyMelee/PoorEnemy.mv1");
	_iAttachIndex = -1;
	// ステータスを「無し」に設定
	_status = STATUS::NONE;
	// 再生時間の初期化
	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;
	// 位置、向きの初期化
	_vPos = VGet(100.0f, 0.0f, 0.0f);
	_vDir = VGet(0.0f, 0.0f, -1.0f);// キャラモデルはデフォルトで-Z方向を向いている
	// 腰位置の設定
	_fColSubY = 40.0f;
	// コリジョン半径の設定
	_fCollisionR = 30.0f;
	_fCollisionWeight = 10.0f;

	_fHp = 30.0f;


	return true;
}

// 終了
bool Enemy::Terminate()
{
	base::Terminate();
	return true;
}

// 計算処理
bool Enemy::Process()
{
	base::Process();

	CharaBase::STATUS old_status = _status;

	if(_status == STATUS::NONE)
	{
		_status = STATUS::WAIT;
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
			MV1DetachAnim(_iHandle, _iAttachIndex);
			_iAttachIndex = -1;
		}
		// ステータスに応じたアニメーションをアタッチする
		switch(_status)
		{
			case STATUS::WAIT:
			{
				int animIndex = MV1GetAnimIndex(_iHandle, "idle");
				if(animIndex != -1)
				{
					_iAttachIndex = MV1AttachAnim(_iHandle, animIndex, -1, FALSE);
					if(_iAttachIndex != -1)
					{
						_fTotalTime = MV1GetAttachAnimTotalTime(_iHandle, _iAttachIndex);
						_fPlayTime = (float)(rand() % 30); // 少しずらす
					}
				}
				break;
			}
		}
		// アタッチしたアニメーションの総再生時間を取得する
		if(_iAttachIndex != -1)
		{
			_fTotalTime = MV1GetAttachAnimTotalTime(_iHandle, _iAttachIndex);
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
bool Enemy::Render()
{
	base::Render();
	// 再生時間をセット
	MV1SetAttachAnimTime(_iHandle, _iAttachIndex, _fPlayTime);

	// 位置
	MV1SetPosition(_iHandle, _vPos);
	// 向きからY軸回転を算出
	VECTOR vrot = { 0,0,0, };
	vrot.y = atan2f(-_vDir.x, -_vDir.z);
	MV1SetRotationXYZ(_iHandle, vrot);

	// 描画
	MV1DrawModel(_iHandle);

	return true;
}