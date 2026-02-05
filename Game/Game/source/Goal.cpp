/*********************************************************************/
// * \file   Goal.cpp
// * \brief  ゴール(ゲームクリアチップ)クラス
// *
// * \author 石森虹大
// * \date   2026/2/5
// * \作業内容: 新規作成 石森虹大 2026/2/5
//			 
/*********************************************************************/
#include "Goal.h"

bool Goal::Initialize()
{
	base::Initialize();
	LoadModel(mv1::Goal);
	if(_handle < 0) { return false; }

	// 当たり判定用フレームの取得
	_hitCollisionFrame = MV1SearchFrame(_handle, "Collision");

 	if(_hitCollisionFrame < 0) { return false; }

	_attachIndex = -1;

	_vPos = vec::Vec3{ 389.0f, 0.0f, 2050.0f };
	_vDir = vec::Vec3{ 0.0f, 0.0f, -1.0f };

	MV1SetupCollInfo(_handle, _hitCollisionFrame, 16, 16, 16);
	MV1SetFrameVisible(_handle, _hitCollisionFrame, FALSE);

	_isVisible = true;

	// 当たり判定情報の初期更新
	ApplyMatrixAndRefreshCollInfo(_handle, _hitCollisionFrame, -1, MakeModelMatrix());
	return true;
}

bool Goal::Terminate()
{
	MV1DeleteModel(_handle);
	return true;
}

bool Goal::Process()
{
	base::Process();

	// 当たり判定情報の更新
	ApplyMatrixAndRefreshCollInfo(_handle, _hitCollisionFrame, -1, MakeModelMatrix());

	return true;
}

bool Goal::Render()
{
	base::Render();

	//// 非表示なら描画しない（デバック)
	//if(!_isVisible)
	//{
	//	return true;

	//}

	if(_handle >= 0)
	{
		MV1DrawModel(_handle);
	}

	return true;
}

MATRIX Goal::MakeModelMatrix() const
{
	float vorty = atan2(_vDir.x * -1, _vDir.z * -1);
	MATRIX mRotY = MGetRotY(vorty);
	MATRIX mTrans = MGetTranslate(DxlibConverter::VecToDxLib(_vPos));
	MATRIX mScale = MGetScale(VGet(1.2f, 1.2f, 1.2f));
	MATRIX m = MGetIdent();
	m = MMult(m, mScale);
	m = MMult(m, mRotY);
	m = MMult(m, mTrans);
	return m;
}