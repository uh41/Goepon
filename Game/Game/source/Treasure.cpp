#include "Treasure.h"

// ヘルパー関数
namespace
{
	// モデル行列を適用し、必要ならコリジョン情報も更新する
	inline void ApplyMatrixAndRefreshCollInfo(int handle, int hitFrame, int openFrame, const MATRIX& m)
	{
		// 無効チェック
		if(handle < 0)
		{
			return;
		}

		// モデル行列を適用する
		MV1SetMatrix(handle, m);

		// コリジョン情報を構築する
		if(hitFrame >= 0)
		{
			MV1RefreshCollInfo(handle, hitFrame);
		}
		if(openFrame >= 0)
		{
			MV1RefreshCollInfo(handle, openFrame);
		}
	}
}
// 初期化
bool Treasure::Initialize()
{
	base::Initialize();

	// モデル読み込み
	_handle = MV1LoadModel("res/Treasure/tuzura_02.mv1");
	if(_handle < 0) { DxLib::printfDx("Treasure model load failed\n"); return false; }

	// ヒット／オープン用フレームはモデル定義どおりに検索
	_hitCollisionFrame = MV1SearchFrame(_handle, "Collision_04");
	_openCollisionFrame = MV1SearchFrame(_handle, "Collision_05");

	// エラーチェック
	if(_hitCollisionFrame < 0) { DxLib::printfDx("HitFrame not found\n"); return false; }
	// Open は任意なら <0 でも続行可。使うならチェック
	// if (_openCollisionFrame < 0) { DxLib::printfDx("OpenFrame not found\n"); }

	// アタッチアニメーションインデックス初期化
	_attachIndex = -1;
	// 宝箱の状態初期化
	_objStatus = OBJSTATUS::NONE;
	// 初期位置・向き設定
	_vPos = vec::Vec3{ 600.0f, 0.0f, 2450.0f };
	_vDir = vec::Vec3{ 0.0f, 0.0f, -1.0f };

	// コリジョン情報の生成
	MV1SetupCollInfo(_handle, _hitCollisionFrame, 16, 16, 16);
	MV1SetFrameVisible(_handle, _hitCollisionFrame, FALSE);
	// オープン用フレームもあれば設定
	if(_openCollisionFrame >= 0)
	{
		MV1SetupCollInfo(_handle, _openCollisionFrame, 16, 16, 16);
		MV1SetFrameVisible(_handle, _openCollisionFrame, FALSE);
	}

	// 宝箱は初期状態で閉じている
	_isOpen = false;

	// 初期反映（描画と判定で共通）
	ApplyMatrixAndRefreshCollInfo(_handle, _hitCollisionFrame, _openCollisionFrame, MakeModelMatrix());
	return true;
}

// 終了
bool Treasure::Terminate()
{
	MV1DeleteModel(_handle);
	return true;
}

bool Treasure::Process()
{
	base::Process();

	// 判定のためにも毎フレーム反映（行列とコリジョンを同期）
	ApplyMatrixAndRefreshCollInfo(_handle, _hitCollisionFrame, _openCollisionFrame, MakeModelMatrix());

	// 宝箱の開閉処理
	if(!_isOpen && _objStatus != OBJSTATUS::OPEN)
	{
		if(_attachIndex != -1)
		{
			MV1DetachAnim(_handle, _attachIndex);
			_attachIndex = -1;
		}
		_objStatus = OBJSTATUS::OPEN;
	}
	return true;
}

bool Treasure::Render()
{
		base::Render();

	if(_handle >= 0)
	{
		// Process()で行列は反映済みなので、描画だけ
		MV1DrawModel(_handle);
	}

	return true;
}

// 現在の _vPos/_vDir/_vScale からモデル行列を生成
MATRIX Treasure::MakeModelMatrix() const
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