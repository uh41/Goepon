#include "TitleTanuki.h"

bool TitleTanuki::Initialize()
{
	base::Initialize();
	LoadModel(mv1::SK_goepon_multimotion_4);
	_iAttachIndex = -1;
	_status = STATUS::WAIT;

	_vPos = vec::Vec3(0.0f, 0.0f, 0.0f);
	_vDir = vec::Vec3(0.0f, 0.0f, 1.0f);
	// アニメーション関係
	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;
	return true;
}

bool TitleTanuki::Terminate()
{
	base::Terminate();
	return true;
}

bool TitleTanuki::Process()
{
	base::Process();

	return true;
}

bool TitleTanuki::Render()
{
	base::Render();

	float vorty = atan2(_vDir.x * -1, _vDir.z * -1);

	// 回転行列、移動行列、拡大行列を作成
	MATRIX mRotY = MGetRotY(vorty);
	MATRIX mRotZ = MGetRotZ(DX_PI_F * 0.5f);
	MATRIX mTrans = MGetTranslate(DxlibConverter::VecToDxLib(_vPos));
	MATRIX mScale = MGetScale(VGet(1.2f, 1.2f, 1.2f)); // 少し大きく表示

	MATRIX m = MGetIdent();
	m = MMult(m, mRotY);
	m = MMult(m, mScale);
	m = MMult(m, mTrans);

	MV1SetMatrix(_handle, m);
	MV1DrawModel(_handle);

	return true;
}