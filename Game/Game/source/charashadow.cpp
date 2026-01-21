#include "charashadow.h"

CharaShadow::CharaShadow()
{
	_chara = nullptr;
	_handle = -1;
	_texturePath = "res/Texture/shadow_.png";
	_fYOffset = 0.0f;
	_fAlpha = 0.5f;
	_fSizeFactor = 1.0f;
}

CharaShadow::~CharaShadow()
{
	Terminate();
}

bool CharaShadow::Initialize()
{
	if(!base::Initialize())
	{
		return false;
	}

	_handle = LoadGraph(_texturePath.c_str());

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
	if(!_chara)
	{
		return true;
	}

	// シャドウの位置をキャラの位置に合わせる
	vec::Vec3 pos = _chara->GetPos();
	pos.y += _fYOffset;
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
	if(!_chara)
	{
		return true;
	}
	// シャドウのサイズをキャラのサイズに合わせる
	float size = _chara->GetCollisionR() * 2.0f * _fSizeFactor;
	_half_polygon_size = size / 2.0f;
	// シャドウの透明度を設定
	_diffuse = { 255, 255, 255, static_cast<unsigned char>(_fAlpha * 255) };
	return true;
}
