/*********************************************************************/
// * \file   effectpool.h
// * \brief  エフェクトの再利用クラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#pragma once
#include "appframe.h"

template<typename T>
class EffectPool
{
public:
	EffectPool(size_t initialSize = 10)		// コンストラクタで初期サイズを指定可能
	{
		_initialSize = initialSize;
		Initialize();
	}

	virtual ~EffectPool() = default;

	virtual bool Initialize()
	{
		for(size_t i = 0; i < _initialSize; ++i)
		{
			auto effect = std::make_unique<T>();
			if(!effect->Initialize())
			{
				return false; // エフェクトの初期化に失敗した場合はfalseを返す
			}
			_availableEffect.push(std::move(effect)); // 初期エフェクトを利用可能なプールに追加
		}
	}
	virtual bool Terminate()
	{
		while (!_availableEffect.empty())
		{
			auto effect = std::move(_availableEffect.front());
			_availableEffect.pop(); // プールから削除
			effect->Terminate(); // エフェクトの終了処理
		}

		for(auto& effect : _activeEffect)
		{
			if(effect)
			{
				effect->Terminate();
			}
		}
		_activeEffect.clear();

		return true;
	}

	// エフェクトを取得する関数
	T* Acquire()
	{
		T* effect = nullptr;
		if(!_availableEffect.empty())
		{
			effect = _availableEffect.front().get(); // 利用可能なエフェクトを取得
			_activeEffect.push_back(std::move(_availableEffect.front())); // 使用中のエフェクトに移動
			_availableEffect.pop(); // プールから削除
		}
		else
		{
			auto newEffect = std::make_unique<T>(); // 新しいエフェクトを作成
			newEffect->Initialize(); // エフェクトの初期化
			effect = newEffect.get(); // ポインタを取得
			_activeEffect.push_back(std::move(newEffect)); // 使用中のエフェクトに追加
		}

		return effect; // エフェクトのポインタを返す
	}

	// エフェクトを解放する関数
	void Release(T* effect)
	{
		if(!effect)
		{
			return;
		}

		// 使用中のエフェクトから解放するエフェクトを検索
		auto it = std::find_if(_activeEffect.begin(), _activeEffect.end(),
			[effect](const at::upt<T>& e) { return e.get() == effect; });

		if(it != _activeEffect.end())
		{
			effect->StopPlaying(); // エフェクトの再生を停止
			_availableEffect.push(std::move(*it)); // 利用可能なエフェクトに移動
			_activeEffect.erase(it); // 使用中のエフェクトから削除
		}
	}

	// 使用中のエフェクトを処理する関数
	void ProcessActive()
	{
		auto it = _activeEffect.begin();
		while(it != _activeEffect.end())
		{
			if(!(*it)->IsPlaying())
			{
				(*it)->StopPlaying(); // エフェクトの再生を停止
				_availableEffect.push(std::move(*it)); // 利用可能なエフェクトに移動
				it = _activeEffect.erase(it); // 使用中のエフェクトから削除
			}
			else
			{
				(*it)->Process(); // エフェクトの更新処理
				++it; // 次のエフェクトへ
			}

		}
	}

	void Update()
	{
		ProcessActive();
	}

	size_t GetAvailableCount() const { return _availableEffect.size(); }
	size_t GetActiveCount() const { return _activeEffect.size(); }
	size_t GetTotalCount() const { return GetAvailableCount() + GetActiveCount(); }

private:
	size_t _initialSize; // 初期サイズ
	at::qupt<T> _availableEffect; // 利用可能なエフェクトのキュー
	at::vupt<T> _activeEffect; // 使用中のエフェクトのベクター
};

