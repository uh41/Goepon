#pragma once
#include "enemybase.h"

// 索敵範囲の情報を格納する構造体
struct DetectionSector
{
	VECTOR center;      // 扇形の中心点（敵の位置）
	VECTOR forward;     // 敵の正面方向
	float radius;       // 索敵範囲の半径
	float angle;        // 索敵角度（度）
};

class Enemy : public EnemyBase
{
	typedef EnemyBase base;

public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	// 索敵範囲の設定
	void SetDetectionSector(float radius, float angle);

	// プレイヤーが索敵範囲内にいるかチェック
	bool IsPlayerInDetectionRange(const VECTOR& playerPos) const;

	// デバッグ用：索敵範囲の描画
	void RenderDetectionSector() const;

protected:
	DetectionSector _detection_sector;  // 索敵範囲
	bool _has_detection_sector;         // 索敵範囲が設定されているか
};

