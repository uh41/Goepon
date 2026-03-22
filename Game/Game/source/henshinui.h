#pragma once
#include "uibase.h"

namespace henshin
{
	static constexpr auto HENSHIN_X = 1400; //1400
	static constexpr auto HENSHIN_Y = 800; //800
	static constexpr auto TANUBITO_X = 1559;
	static constexpr auto TANUBITO_Y = 847;
	static constexpr auto TANUMONO_X = 1530;
	static constexpr auto TANUMONO_Y = 847;
}

class HenshinUi : public UiBase
{
	typedef UiBase base;
public:

	enum class Select : int
	{
		TANUKI = 0,
		TANUBITO = 1,
		TANUMONO = 2,
		Count = 3,
		_EOT_
	};

	HenshinUi();
	virtual ~HenshinUi() = default;
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	void SetOwner(void* owner) { _owner = owner; }

	void SetShowPlayerUi(bool show) { _showPlayerUi = show; }
	void SetShowPlayerMonoUi(bool show) { _showPlayerMonoUi = show; }

protected:

	Select _select;     // 変身UIの選択肢
	int _handleTanubito; // タヌキ人間UIのグラフィックハンドル
	int _handleMono;     // モノUIのグラフィックハンドル

	bool _padInput5Active;	// パッド入力5Aが有効かどうか

	void* _owner; // このUIを所有するオブジェクトへのポインタ

	bool _showPlayerUi; // プレイヤーUIを表示するかどうか
	bool _showPlayerMonoUi;  // ものUIを表示するかどうか
};

