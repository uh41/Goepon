#pragma once
#include "appframe.h"

class StageManager
{
public:
	// ステージIDのリストを設定。設定すると最初のステージにリセットされる
	void SetStages(std::vector<std::string> stages)
	{
		_stages = std::move(stages);
		_index = 0;
	}

	// 現在のステージIDを取得。未設定/範囲外の場合は空文字を返す
	const std::string& GetCurrentStageId() const
	{
		// 未設定/範囲外の場合は空文字を返す
		static const std::string empty;
		if (_stages.empty() || _index >= _stages.size())
		{
			return empty;
		}
		return _stages[_index];
	}

	// 現在のステージインデックスを取得
	size_t GetCurrentStageIndex() const
	{
		return _index;
	}

	// ステージIDからインデックスを設定。成功すれば true、見つからない場合は false
	bool SetCurrentStageId(const std::string& stageId)
	{
		for(size_t i = 0; i < _stages.size(); ++i)
		{
			if(_stages[i] == stageId)
			{
				_index = i;
				return true;
			}
		}
		return false;
	}

	// 次のステージがあれば true、最後なら false
	bool CanGoNext() const
	{
		return !_stages.empty() && (_index + 1) < _stages.size();
	}

	
	// 次のステージに進む。成功すれば true、最後のステージで進めない場合は false
	bool GoNext()
	{
		if(!CanGoNext()) { return false; }
		++_index;
		return true;
	}

	// 最初のステージにリセット
	void ResetToFirst()
	{
		_index = 0;
	}

	// 現在のステージインデックスを設定する
	void SetCurrentStageIndex(size_t index)
	{
		if(index < _stages.size())
		{
			_index = index;
		}
	}

	// ステージIDからインデックスを取得。見つからない場合は0を返す
	size_t GetStageIndex(const std::string& stageId)const
	{
		for(size_t i = 0; i < _stages.size(); ++i)
		{
			if(_stages[i] == stageId)
			{
				return i;
			}
		}
		return 0; // 見つからない場合は0を返す（デフォルトのステージ）
	}

	std::string GetNextStageId(const std::string stageId) const
	{
		if(_stages.empty())
		{
			return {};
		}

		for(size_t i = 0; i < _stages.size(); ++i)
		{
			if(_stages[i] == stageId)
			{
				const size_t next = i + 1;
				if(next < _stages.size())
				{
					return _stages[next];
				}
				else
				{
					return {}; // 最後のステージの場合は空文字を返す
				}
			}
		}
		return {}; // 見つからない場合は空文字を返す
	}
private:
	std::vector<std::string> _stages;
	size_t _index = 0; // 現在のステージインデックス
};