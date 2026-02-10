#pragma once
#include "enemybase.h"
#include "playerbase.h"
#include "MapBase.h"

// 前方宣言
class Map;

// 音検知範囲の情報を格納する構造体
struct SoundSensorArea
{
	vec::Vec3 center;	// センサーの中心位置
	float radius;       // センサーの検知半径
};

// 音波の情報を格納する構造体
struct SoundWave
{
	vec::Vec3 origin;		// 音波の発生位置
	float currentRadius;	// 現在の半径
	float maxRadius;		// 最大半径
	float expandSpeed;		// 拡大速度
	float alpha;			// 描画用の透明度
	bool isActive;			// 音波が有効かどうか
	int soundLevel;			// 音の大きさレベル
};

// 音検知状態の情報
struct SoundDectionInfo
{
	bool isDetected;				// 音が検出されているか
	float timer;					// 検出表示タイマー
	vec::Vec3 soundSourcePosition;  // 検出した音源の位置
	int detectedSoundLevel;			// 検出した音の大きさレベル
};

class EnemySoundSensor : public EnemyBase
{
	typedef EnemyBase base;

public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	void SetMap(MapBase* map) { _map = map; }
	void SetSoundLevel(int level);
	const SoundDectionInfo& GetDetectionInfo() const { return _sounddetectionInfo; }	// 検出情報の取得
	void SetSoundSensorArea(float radius);	

	// 音波の発生
	void TriggerSoundWave(const vec::Vec3& origin, float maxRadius, float speed);

protected:
	SoundSensorArea _soundsensorarea;
	SoundDectionInfo _sounddetectionInfo;
	MapBase* _map;

	std::vector<SoundWave> _soundWaves;
	static constexpr int MAX_SOUND_WAVES = 10;  

	// 音波の描画と更新
	void RenderSoundWaves();

	// 音波の更新
	void UpdateSoundWaves();

	// 音波とセンサー範囲の衝突判定
	bool CheckSoundWaveCollision(const SoundWave& wave) const;

	// 音検知時の処理
	void OnSoundDetected(const vec::Vec3& soundOrigin);

	int _soundLevel;	// 音の大きさレベル
};


