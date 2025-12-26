#pragma once
#include "DxLib.h"
enum class LightType
{
	Directional, // 平行光源
	Point,       // 点光源
	Spot         // スポット光源
};

class Light
{
public:
	Light(LightType t = LightType::Directional);
	
	// type
	void      SetType(LightType t) { type = t   ; }
	LightType GetType() const      { return type; }

	//	方向・位置
	void   SetDir(const VECTOR& d) { dir = d   ; }	
	VECTOR GetDir() const          { return dir; }

	void SetPosition(const VECTOR& p)  { pos = p;    }
	VECTOR GetPosition() const         { return pos; }
	
	// 色設定
	void SetAmbient (const VECTOR& a, float aA = 1.0f) { ambient = a; ambientA = aA  ; }
	void SetDiffuse (const VECTOR& d, float dA = 1.0f) { diffuse = d; diffuseA = dA  ; }
	void SetSpecular(const VECTOR& s, float sA = 1.0f) { specular = s; specularA = sA; }

	// 点光源用
	void  SetRange(float r) { range = r   ; } 
	void SetAttenuation(float a0, float a1, float a2) { att0 = a0; att1 = a1; att2 = a2; }

	//　影の挙動
	void SetCastShadow(bool v) { castShadow = v; }
	bool GetCastShadow() const { return castShadow; }

	// DxLib レンダラへ適用
	void ApplyRenderer() const;

	// シャドウマップ向け設定（shadowHandle が無効なら何もしない）
	void ApplyShadowMap(int shadowHandle, const VECTOR& camTarget, float extent) const;
private:
	LightType type;
	// 方向・位置は VECTOR (x,y,z)
	VECTOR dir = { 0.0f, -1.0f, 0.0f };
	VECTOR pos = { 0.0f,  0.0f, 0.0f };

	// 色は VECTOR(3要素) + アルファ
	// 環境光
	VECTOR ambient = { 0.0f, 0.0f, 0.0f };
	float ambientA = 0.0f;

	// 拡散反射光
	VECTOR diffuse = { 1.0f, 1.0f, 1.0f };
	float diffuseA = 1.0f;

	// 鏡面反射光
	VECTOR specular = { 0.0f, 0.0f, 0.0f };
	float specularA = 0.0f;

	// 点光源用
	float range = 1000.0f; // 光源の届く範囲
	float att0 = 0.0f, att1 = 0.0f, att2 = 0.005f;  // 定数減衰

	// 影を落とすかどうか
	bool castShadow = true; 
};

