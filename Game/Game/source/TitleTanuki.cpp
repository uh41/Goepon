#include "TitleTanuki.h"

bool TitleTanuki::Initialize()
{
	base::Initialize();
	LoadModel(mv1::SK_goepon_multimotion_4);
	_iAttachIndex = -1;
	_status = STATUS::WAIT;

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

	float vorty = atan2(_vDir.x * -1, _vDir.z * -1);// モデルが標準でどちらを向いているかで式が変わる(これは-zを向いている場合)

	// 回転行列、平行移動行列、拡大行列を作成
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
	return true;
}
