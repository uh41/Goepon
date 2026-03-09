#pragma once
#include "uibase.h"

namespace henshin
{
	static constexpr auto MAKIMONO_X = 1654;
	static constexpr auto MAKIMONO_Y = 20;
	static constexpr auto TANUBITO_X = 1700;
	static constexpr auto TANUBITO_Y = 1024;
	static constexpr auto TANUMONO_X = 1850;
	static constexpr auto TANUMONO_Y = 1025;
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

protected:

	Select _select;     // 変身UIの選択肢
	int _handleTanubito; // タヌキ人間UIのグラフィックハンドル
	int _handleMono;     // モノUIのグラフィックハンドル

	bool _padInput5Active;	// パッド入力5Aが有効かどうか

	void* _owner; // このUIを所有するオブジェクトへのポインタ
};

