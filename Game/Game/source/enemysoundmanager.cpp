/*********************************************************************/
// * \file   enemysoundmanager.cpp
// * \brief  音管理マネージャークラス
// *
// * \author 市村義春
/*********************************************************************/
#include "enemysoundmanager.h"
#include "enemybase.h"

// シングルトンインスタンスの取得
EnemySoundManager* EnemySoundManager::GetInstance()
{
	static EnemySoundManager instance;
	return &instance;
}

// 音を発生させる関数
void EnemySoundManager::EmitSound(const vec::Vec3& pos, int soundLevel, float maxRad, float speed, uint32_t emitterId)
{
	Wave* wave = nullptr;
	for(auto&& w : _wave)
	{
		// 非アクティブな波があれば再利用
		if(!w.isActive)
		{
			wave = &w;
			break;
		}
	}

	if(!wave && StCas<int>(_wave.size()) < wave::MAX_SOUND_WAVES)
	{
		_wave.emplace_back();
		wave = &_wave.back();
	}

	if(!wave)
	{
		return; // 波を追加できない場合は終了
	}

	wave->id = _nextWaveId++;
	wave->pos = pos;
	wave->rad = 0.0f;
	wave->maxRad = maxRad;
	wave->speed = speed;
	wave->soundLevel = soundLevel;
	wave->isActive = true;
	wave->emitterId = emitterId;

	_waveDetectionMap.emplace(wave->id, at::ust<uint32_t>{}); // 新しい波の検知済み敵IDセットを初期化
}

// 音波の更新関数
void EnemySoundManager::Update(float deltaTime)
{
	StCas<void>(deltaTime);// 現状は時間経過で波が広がるだけのシンプルな実装。必要に応じて波の寿命や減衰などを追加可能。

	for(auto&& wave : _wave)
	{
		if(!wave.isActive)
		{
			continue;
		}

		wave.rad += wave.speed;

		if(wave.rad >= wave.maxRad)
		{
			wave.isActive = false;				// 波が最大になったら終わりにする
			_waveDetectionMap.erase(wave.id);	// 波が消えるので検知済み敵IDセットも削除
		}
	}
}

// 敵が音を検知できるか試す関数
bool EnemySoundManager::TryDetectForEnemy(const EnemyBase& enemy, DetectionInfo& inoutInfo)
{
	float hearingrad = enemy.GetHearingRadius();

	if(hearingrad <= 0.0f)
	{
		return false; // 聴覚半径が0以下なら検出できない
	}

	uint32_t enemyId = enemy.GetEnemyId();
	if(enemyId == 0)
	{
		return false; // 敵IDが0なら無効
	}

	// すでに検知中なら新規検知はしない
	if(inoutInfo.isDetected)
	{
		return false;
	}

	for(auto&& wave : _wave)
	{
		if(!wave.isActive)
		{
			continue;
		}

		// 自分が発した音は検知しない
		if (wave.emitterId == enemyId)
		{
			continue;
		}

		if(!IsAudible(wave, enemy))
		{
			continue; // この波は敵に聞こえない
		}

		auto& detectedEnemy = _waveDetectionMap[wave.id];
		if(detectedEnemy.find(enemyId) != detectedEnemy.end())
		{
			continue; // すでにこの波を検知している
		}

		// 波を検知
		detectedEnemy.insert(enemyId);

		inoutInfo.isDetected = true;
		inoutInfo.timer = 2.0f;
		inoutInfo.soundSourcePos = wave.pos;
		inoutInfo.detectedSoundLevel = wave.soundLevel;

		return true; // 波を検知したら終了
	}

	return false;
}

// 波が敵に聞こえるかどうかの判定関数
bool EnemySoundManager::IsAudible(const Wave& wave, const EnemyBase& enemy) const
{
	vec::Vec3 wavePos = vec3::VSub(wave.pos, enemy.GetPos());
	wavePos.y = 0.0f;

	float dist = vec3::VSize(wavePos);
	float reach = wave.rad + enemy.GetHearingRadius();

	return dist <= reach;
}

// デバッグ用の描画関数
void EnemySoundManager::RenderDebug()
{
	for(const auto& wave : _wave)
	{
		if(!wave.isActive)
		{
			continue;
		}

		constexpr int segments = 32;
		constexpr float angleStep = (2.0f * DX_PI_F) / segments;

		float alpha01 = 1.0f;
		if(wave.maxRad > 0.0f)
		{
			alpha01 = 1.0f - (wave.rad / wave.maxRad);
			if(alpha01 < 0.0f) alpha01 = 0.0f;
			if(alpha01 > 1.0f) alpha01 = 1.0f;
		}

		const bool isLastEmit = (_debugLastEmitWaveId != 0) && (wave.id == _debugLastEmitWaveId) && (_debugLastEmitTimer > 0.0f);
		const unsigned int color = isLastEmit ? GetColor(255, 64, 64) : GetColor(255, 255, 0);

		const int alpha = StCas<int>(alpha01 * 255);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

		const float y = wave.pos.y + 0.5f;

		for(int i = 0; i < segments; ++i)
		{
			const float angle1 = angleStep * i;
			const float angle2 = angleStep * (i + 1);

			const VECTOR pos1 = VGet(
				wave.pos.x + wave.rad * cosf(angle1),
				y,
				wave.pos.z + wave.rad * sinf(angle1));

			const VECTOR pos2 = VGet(
				wave.pos.x + wave.rad * cosf(angle2),
				y,
				wave.pos.z + wave.rad * sinf(angle2));

			DrawLine3D(pos1, pos2, color);
		}

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

// 敵の聴覚範囲を描画する関数
void EnemySoundManager::RenderDebugEnemyHearing(const at::vspc<EnemyBase>& enemies) const
{
	for(const auto& e : enemies)
	{
		if(!e || !e->IsAlive())
		{
			continue;
		}

		const float radius = e->GetHearingRadius();
		if(radius <= 0.0f)
		{
			continue;
		}

		const vec::Vec3 center = e->GetPos();

		// 直近の音検知中かで色を変える（既存フラグを流用）
		const unsigned int color = e->IsMovingToSound();
		if (color) 
		{
			GetColor(255, 0, 0);
		}
		else 
		{
			GetColor(255, 255, 0);
		}

		constexpr int segments = 32;
		const float angleStep = (2.0f * DX_PI_F) / segments;

		for(int i = 0; i < segments; ++i)
		{
			const float angle1 = angleStep * i;
			const float angle2 = angleStep * (i + 1);

			const vec::Vec3 p1 = vec3::VAdd(center, vec3::VGet(cosf(angle1) * radius, 0.0f, sinf(angle1) * radius));
			const vec::Vec3 p2 = vec3::VAdd(center, vec3::VGet(cosf(angle2) * radius, 0.0f, sinf(angle2) * radius));

			DxlibConverter::DrawLine3D(p1, p2, color);
		}

		// 簡易マーカー
		const vec::Vec3 m1 = vec3::VAdd(center, vec3::VGet(-15.0f, 5.0f, 0.0f));
		const vec::Vec3 m2 = vec3::VAdd(center, vec3::VGet(15.0f, 5.0f, 0.0f));
		const vec::Vec3 m3 = vec3::VAdd(center, vec3::VGet(0.0f, 5.0f, -15.0f));
		const vec::Vec3 m4 = vec3::VAdd(center, vec3::VGet(0.0f, 5.0f, 15.0f));

		DxlibConverter::DrawLine3D(m1, m2, color);
		DxlibConverter::DrawLine3D(m3, m4, color);
	}
}