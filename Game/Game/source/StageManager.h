#pragma once
#include "appframe.h"

class StageManager
{
public:
	void SetStages(std::vector<std::string> stages)
	{
		_stages = std::move(stages);
		_index = 0;
	}

	const std::string& GetCurrentStageId() const
	{
		// 安全策：未設定/範囲外の場合は空文字を返す
		static const std::string empty;
		if (_stages.empty() || _index >= _stages.size())
		{
			return empty;
		}
		return _stages[_index];
	}

	bool CanGoNext() const
	{
		return !_stages.empty() && (_index + 1) < _stages.size();
	}

	// 次があれば進めて true、最後なら false
	bool GoNext()
	{
		if(!CanGoNext()) { return false; }
		++_index;
		return true;
	}

	void ResetToFirst()
	{
		_index = 0;
	}

private:
	std::vector<std::string> _stages;
	size_t _index = 0;
};