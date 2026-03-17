#pragma once
#include "appframe.h"

class EnemyBase;

namespace wave
{
	static constexpr auto MAX_SOUND_WAVES = 32;	// 同時に存在できる音波の最大数
}

class EnemySoundManager
{
public:
	struct Wave
	{
		uint32_t id;									// 音波のID
		vec::Vec3 pos = vec::Vec3(0.0f, 0.0f, 0.0f);	// 音波の位置
		float rad = 0.0f;								// 音波の半径
		float maxRad = 0.0f;							// 音波の最大半径
		float speed = 0.0f;								// 音波の拡散速度
		int soundLevel = 0;								// 音の大きさレベル
		bool isActive = false;							// 音波が有効かどうか
		uint32_t emitterId = 0;
	};

	struct DetectionInfo
	{
		bool isDetected = false;								// 音が検出されたかどうか
		float timer = 0.0f;										// 検出状態のタイマー
		vec::Vec3 soundSourcePos = vec::Vec3(0.0f, 0.0f, 0.0f);	// 音源の位置
		int detectedSoundLevel = 0;								// 検出された音の大きさレベル
	};

	static EnemySoundManager* GetInstance();

	EnemySoundManager() = default;

	// 音を発生させる関数
	void EmitSound(const vec::Vec3& pos, int soundLevel, float maxRad, float speed, uint32_t emitterId = 0);

	// 音波の更新関数
	void Update(float deltaTime);

	// 敵が音を検知できるか試す関数
	bool TryDetectForEnemy(const EnemyBase& enemy, DetectionInfo& inoutInfo);

	void RenderDebug(); // デバッグ用の描画関数
	void RenderDebugEnemyHearing(const at::vspc<EnemyBase>& enemies) const; // 敵の聴覚範囲を描画する関数

protected:
	at::vet<Wave> _wave;					// 現在存在する音波のリスト
	std::atomic<uint32_t> _nextWaveId = 1;  // 次に発生する音波のID

	// waveId->検知済みの敵IDのセット
	at::umtt<uint32_t, at::ust<uint32_t>> _waveDetectionMap;// 音波IDと、その音波を検知した敵のIDのセット

	// デバッグ用の情報を管理する変数
	bool IsAudible(const Wave& wave, const EnemyBase& enemy) const;

private:
	uint32_t _debugLastEmitWaveId = 0;	// デバッグ用：直近で発生させた音波のID
	float _debugLastEmitTimer = 0.0f;	// デバッグ用：直近で発生させた音波のタイマー
	static constexpr float DEBUG_LAST_EMIT_HIGHLIGHT_TIME = 0.35f;	// デバッグ用：直近で発生させた音波をハイライトする時間
};

