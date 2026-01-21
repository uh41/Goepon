#include "Treasure.h"

// 初期化
bool Treasure::Initialize()
{
	base::Initialize();
	// モデルの読み込み
	_handle = MV1LoadModel("res/Treasure/tuzura_02.mv1");
	_attachIndex = -1;
	// 宝箱の状態を「無し」に設定
	_objStatus = OBJSTATUS::NONE;
	// 宝箱の位置、向きの初期化
	_vPos = vec::Vec3{ 0.0f, 0.0f, 10.0f };
	_vDir = vec::Vec3{ 0.0f, 0.0f, -1.0f };
	// 宝箱は最初開いていない
	_isOpen = true;

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
	// 宝箱の開閉処理
	if(!_isOpen && _objStatus != OBJSTATUS::OPEN)
	{
		// 宝箱を開ける
		if(_attachIndex != -1)
		{
			MV1DetachAnim(_handle, _attachIndex);
			_attachIndex = -1;
		}
		// アニメーションの切り替え
		//_attachIndex = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "open"), -1, FALSE);
		_objStatus = OBJSTATUS::OPEN;
	}
	return true;
}

bool Treasure::Render()
{
	base::Render();
	float vorty = atan2(_vDir.x * -1, _vDir.z * -1); // 再生時間をセットする
	// モデルの向き設定
	MATRIX mRotY = MGetRotY(vorty); 
	// 平行移動行列の取得
	MATRIX mTrans = MGetTranslate(VectorConverter::VecToDxLib(_vPos));
	// 拡大縮小行列
	MATRIX mScale = MGetScale(VGet(1.0f, 1.0f, 1.0f));
	// モデル行列の合成
	MATRIX m = MGetIdent();

	// 拡大縮小、回転、平行移動の順で行列を掛け合わせる
	m = MMult(m, mScale);
	m = MMult(m, mRotY);
	m = MMult(m, mTrans);
	// モデルに行列をセット
	MV1SetMatrix(_handle, m);
	 
	// 描画
	MV1DrawModel(_handle);

	return true;
}