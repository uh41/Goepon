#include "charashadow.h"

CharaShadow::CharaShadow()
{
	_Chara = nullptr;
	_handle = -1;
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
	_handle = LoadGraph("res/Texture/shadow_.png");

	_fScale = 1.0f;

	return true;
}

bool CharaShadow::Terminate()
{
	base::Terminate();
	if (_handle != -1)
	{
		DeleteGraph(_handle);
		_handle = -1;
	}
	return true;
}

bool CharaShadow::Process()
{
	if (!base::Process())
	{
		return false;
	}

	// キャラがいなければ何もしない
	if(!_Chara)
	{
		return true;
	}

	// シャドウの位置をキャラの位置に合わせる
	vec::Vec3 pos = _Chara->GetPos();
	pos.y += _fColSubY;
	_vPos = pos;

	return true;
}

bool CharaShadow::Render()
{
	if (!base::Render())
	{
		return false;
	}
	// キャラがいなければ何もしない
	if(!_Chara)
	{
		return true;
	}
	
		// キャラが存在しない場合は描画しない
	if(!_Chara->IsAlive())
	{
		return false;
	}

	if(_handle == -1)
	{
		return false;
	}
	// キャラの足元位置を計算
	vec::Vec3 pos = _Chara->GetPos();

	pos.y = _Chara->GetColSubY();

	VECTOR vpos = DxlibConverter::VecToDxLib(pos);

	float y_offset = 0.2f; // 影を少し浮かせる

	DrawGraph3D(vpos.x, vpos.y + y_offset, vpos.z, _handle, TRUE);

	return true;
}
