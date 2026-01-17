#include "charashadow.h"

CharaShadow::CharaShadow()
{
	_Chara = nullptr;
	_iHandle = -1;
	_fScale = 1.0f;
}

CharaShadow::~CharaShadow()
{
	Terminate();
}

bool CharaShadow::Initialize()
{
	if(!base::Initialize()) { return false; }

	// 影モデルの読み込み
	_iHandle = LoadGraph("res/Texture/shadow_.png");

	_fScale = 1.0f;

	return true;
}

bool CharaShadow::Terminate()
{
	if(_iHandle != -1)
	{
		DeleteGraph(_iHandle);
		_iHandle = -1;
	}
	base::Terminate();
	return true;
}

bool CharaShadow::Process()
{
	if(!base::Process()) { return false; }

	return true;
}

bool CharaShadow::Render()
{
	if(!base::Render()) { return false; }

	if(!_Chara)
	{
		return false;
	}

	// キャラが存在しない場合は描画しない
	if(!_Chara->IsAlive())
	{
		return false;
	}

	if(_iHandle == -1)
	{
		return false;
	}

	// キャラの足元位置を計算
	vec::Vec3 pos = _Chara->GetPos();

	pos.y = _Chara->GetColSubY();

	VECTOR vpos = VectorConverter::VecToDxLib(pos);

	float y_offset = 0.2f; // 影を少し浮かせる

	DrawGraph3D(vpos.x, vpos.y + y_offset, vpos.z, _iHandle, TRUE);

	return true;
}