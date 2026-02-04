#pragma once
#include "enemybase.h"
#include "playerbase.h"

// 前方宣言
class Map;

struct SoundSensorArea
{
	vec::Vec3 center;   // センサーの中心点（敵の位置）
	float radius;       // センサー範囲の半径
};

// 音の波紋エフェクト用構造体
struct SoundWave
{
	vec::Vec3 origin;     // 発生源
	float currentRadius;  // 現在の半径
	float maxRadius;      // 最大半径
	float expandSpeed;    // 拡大速度
	bool isActive;        // アクティブフラグ
	float alpha;          // 透明度（0.0 ～ 1.0）
};

struct SoundDectionInfo
{
	bool isDetected;        // 検出されているか
	float timer;            // 検出表示タイマー
	
};

class EnemySoundSensor : public EnemyBase
{
	typedef EnemyBase base;

public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	void SetMap(Map* map) { _map = map; }

	// サウンドセンサー範囲の設定
	void SetSoundSensorArea(float radius);	// 半径

	// 音の波紋を発生させる
	void TriggerSoundWave(const vec::Vec3& origin, float maxRadius, float speed);

protected:
	SoundSensorArea _soundsensorarea;
	SoundDectionInfo _sounddetectionInfo;
	Map* _map;

	// 音の波紋管理
	std::vector<SoundWave> _soundWaves;
	static constexpr int MAX_SOUND_WAVES = 10;  // 同時に表示できる波紋の最大数

	// 波紋の描画
	void RenderSoundWaves();
	void UpdateSoundWaves();

	// ★ 新規追加：音の波紋とサウンドセンサーの当たり判定
	bool CheckSoundWaveCollision(const SoundWave& wave) const;

	// ★ 新規追加：音を検知したときの処理
	void OnSoundDetected(const vec::Vec3& soundOrigin);
};

