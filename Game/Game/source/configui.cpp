#include "configui.h"

ConfigUi::ConfigUi()
{
	_handleConfigUiTanuki = -1;
	_handleConfigUiTanubito = -1;
	_handleConfigUiTanumono = -1;
	_handleActive = -1;
	_activeForm = FormType::TANUKI;
}

bool ConfigUi::Initialize()
{
	base::Initialize();
	_handleConfigUiTanuki = LoadGraph(ui::Tanuki_config);
	_handleConfigUiTanubito = LoadGraph(ui::Tanubito_config);
	_handleConfigUiTanumono = LoadGraph(ui::Tanumono_config);

	SetTransForm(_activeForm);
	_visible = true;
	return true;
}

bool ConfigUi::Terminate()
{
	base::Terminate();
	if(_handleConfigUiTanuki != -1)
	{
		DeleteGraph(_handleConfigUiTanuki);
		_handleConfigUiTanuki = -1;
	}
	if(_handleConfigUiTanubito != -1)
	{
		DeleteGraph(_handleConfigUiTanubito);
		_handleConfigUiTanubito = -1;
	}
	if(_handleConfigUiTanumono != -1)
	{
		DeleteGraph(_handleConfigUiTanumono);
		_handleConfigUiTanumono = -1;
	}
	return true;
}

bool ConfigUi::Process()
{
	base::Process();
	return true;
}

bool ConfigUi::Render()
{
	base::Render();
	if(_visible)
	{
		// 現在のフォームに対応するハンドルだけ描画する
		if(_handleActive != -1)
		{
			DrawGraph(ui::CONFIG_X, ui::CONFIG_Y, _handleActive, TRUE);
		}
	}
	return true;
}

bool ConfigUi::SetTransForm(FormType form)
{
	_activeForm = form;
	switch(form)
	{
	case FormType::TANUKI:
		_handleActive = _handleConfigUiTanuki;
		break;
	case FormType::TANUBITO:
		_handleActive = _handleConfigUiTanubito;
		break;
	case FormType::TANUMONO:
		_handleActive = _handleConfigUiTanumono;
		break;
	default:
		return false; // 不正なフォームタイプ
	}
	return true;
}
