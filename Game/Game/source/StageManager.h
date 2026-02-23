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
		return _stages.at(_index);
	}

	bool CanGoNext() const
	{
		return !_stages.empty() && (_index + 1) < _stages.size();
	}

	// ŽŸ‚ª‚ ‚ê‚Îi‚ß‚Ä trueAÅŒã‚È‚ç false
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