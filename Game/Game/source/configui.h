#pragma once
#include "uibase.h"

namespace ui
{
	static constexpr const auto CONFIG_X = 0;
	static constexpr const auto CONFIG_Y = 1030;
}

class ConfigUi : public UiBase
{
	typedef UiBase base;
public:
	ConfigUi();
	virtual ~ConfigUi() = default;
	bool Initialize() override;
	bool Terminate() override;
	bool Process() override;
	bool Render() override;

	enum class FormType
	{
		TANUKI,
		TANUBITO,
		TANUMONO,
		_EOT_
	};

	bool SetTransForm(FormType form);

protected:
	int _handleConfigUiTanuki;
	int _handleConfigUiTanubito;
	int _handleConfigUiTanumono;

	int _handleActive;
	FormType _activeForm;
};

