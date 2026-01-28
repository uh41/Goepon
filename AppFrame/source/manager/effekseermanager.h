#pragma once
#include "../container.h"
#include "../mymath/vectorconverter.h"
#include "../aliastemplate.h"

class EffekseerManager
{
public:
	static EffekseerManager* GetInstance()
	{
		static EffekseerManager instance;
		return &instance;
	}

	bool Initialize();
	bool Terminate();

	// エフェクトの読み込み
	int LoadEffect(const char* path, float scale = 1.0f);

	// リソース削除
	bool DeleteEffect(int handle);

	// path指定でアンロード
	bool UnloadEffect(const char* path);

	// 全リソースの削除
	void DeleteAllEffect();

	// 読み込み済みかどうか
	bool IsLoadedEffect(const char* path) const;

	// エフェクトを再生
	int PlayEffect3D(int handle);

	// エフェクトを再生(位置指定)
	int PlayEffect3DPos(int handle, const vec::Vec3& pos);

	// エフェクトを停止
	bool StopEffect(int handle);

	// 再生中かどうか
	bool IsPlayingEffect(int handle) const;

	// 一時停止/再開
	bool SetPauseEffect(int handle, bool pause);

	// 回転
	bool SetRotationEffect(int handle, const vec::Vec3& rad);
	vec::Vec3 GetRotationEffect(int handle) const;

	// 位置設定
	bool SetPosEffect(int handle, const vec::Vec3& pos);
	vec::Vec3 GetPosEffect(int handle) const;

	// 速度設定
	bool SetSpeedEffect(int handle, float speed);
	float GetSpeedEffect(int handle) const;

	// スケール設定
	bool SetScaleEffect(int handle, float scale);

	void Update();	// 更新
	void Render();

	// 2Dエフェクト再生

	int PlayEffect2D(int handle);

	// 2Dエフェクト再生(位置指定)
	int PlayEffect2DPos(int handle, const vec::Vec3& pos);

	// 2Dエフェクト停止
	bool StopEffect2D(int handle);

	// 2Dエフェクト再生中かどうか
	bool IsPlayingEffect2D(int handle) const;

	// 2Dエフェクト一時停止/再開
	bool SetPauseEffect2D(int handle, bool pause);

	// 2Dエフェクト速度設定
	bool SetSpeedEffect2D(int handle, float speed);
	float GetSpeedEffect2D(int handle) const;

	// 2Dエフェクトスケール設定
	bool SetScaleEffect2D(int handle, float scale);
	vec::Vec3 GetScaleEffect2D(int handle) const;

	// 2Dエフェクト位置設定
	bool SetPosEffect2D(int handle, const vec::Vec3& pos);
	vec::Vec3 GetPosEffect2D(int handle) const;

	// 2Dエフェクト回転設定
	bool SetRotationEffect2D(int handle, const vec::Vec3& rad);
	vec::Vec3 GetRotationEffect2D(int handle) const;

	// 2Dエフェクト更新
	void Update2D();
	void Render2D();

private:

	EffekseerManager() { _initialize = false; }
	~EffekseerManager() {};

	EffekseerManager(const EffekseerManager&) = delete;
	EffekseerManager& operator=(const EffekseerManager&) = delete;

	bool _initialize;

	at::umtc<std::string, int> _effect; // エフェクトパスとハンドルのマップ

	//pause 復帰のために、速度を保持
	at::umtt<int, float> _effectSpeed; // エフェクトハンドルと速度のマップ
	at::umtt<int, vec::Vec3> _effectRotation; // エフェクトハンドルと回転のマップ
	at::umtt<int, vec::Vec3> _effectPos; // エフェクトハンドルと位置のマップ

};