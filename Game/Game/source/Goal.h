/*********************************************************************/
// * \file   Goal.h
// * \brief  ゴール(ゲームクリアチップ)クラス
// *
// * \author 石森虹大
// * \date   2026/2/5
// * \作業内容: 新規作成 石森虹大 2026/2/5
//			 
/*********************************************************************/
#pragma once
#include "objectbase.h"
class Goal :public ObjectBase
{
	typedef ObjectBase base;
public:

	bool Initialize() override; // 初期化
	bool Terminate()  override; // 終了
	bool Process()    override; // 処理
	bool Render()     override; // 描画

	auto GetPos() const { return _vPos; }

	auto GetHitCollisionFrame() const { return _hitCollisionFrame; }

	auto GetModelHandle() const { return _handle; }


	// デバック用(モデルを表示/非表示)
	bool IsVisible() const { return _isVisible; }


	MATRIX MakeModelMatrix() const;

protected:
	int _hitCollisionFrame; // 当たり判定用フレーム
	int _attachIndex;       // モデルの当たり判定用フレームのアタッチ番号
	bool _isVisible;        // 表示フラグ
};

