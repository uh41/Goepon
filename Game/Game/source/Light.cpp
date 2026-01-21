/*********************************************************************/
// * \file   camera.cpp
// * \brief  カメラクラス
// *
// * \author 石森虹大
// * \date   2025/12/26
// * \作業内容: 新規作成 石森虹大　2025/12/26
/*********************************************************************/

#include "Light.h"

Light::Light(LightType t): type(t)
{
	// 何もしない
}

void Light::ApplyRenderer()const
{
	// ライティング有効化
	SetUseLighting(TRUE);

	// グローバルアンビエントライトの設定
	SetGlobalAmbientLight(GetColorF(ambient.x, ambient.y, ambient.z, ambientA));

	// 標準のライトを対象にする
	int index = 0;

	// ライトの基準性能を設定
	

	// ライトの種類ごとに DxLib のラッパーを呼ぶ
	if(type == LightType::Directional)
	{
		ChangeLightTypeDir(dir);
	}
	else if(type == LightType::Point)
	{
		ChangeLightTypePoint(pos, range, att0, att1, att2);
	}
	else if(type == LightType::Spot)
	{
		// スポットライトの実装例（将来拡張用）
		// ChangeLightTypeSpot(pos, dir, outAngle, inAngle, range, att0, att1, att2);
	}
}

// シャドウマップ向け設定（shadowHandle が無効なら何もしない）(extentは範囲）
void Light::ApplyShadowMap(int shadowHandle, const VECTOR& camTarget, float extent)const
{
	// 影を落とさない設定なら何もしない
	if(!castShadow     ) return;
	if(shadowHandle < 0) return;

	//ライトの方向をシャドウマップにセット
	SetShadowMapLightDirection(shadowHandle, dir);

	//シャドウマップに描画する
	VECTOR min = VAdd(camTarget, VGet(-extent, -1.0f, -extent)); // AABB の最小点を作成。
	VECTOR max = VAdd(camTarget, VGet(extent, 1000.0f, extent)); // 最大点を作成。
	SetShadowMapDrawArea(shadowHandle, min, max);// 描画範囲を設定(これによりシャドウカメラ(ライト空間)の投影範囲が決まる）
}

