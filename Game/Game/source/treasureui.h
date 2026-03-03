#pragma once
#include "uibase.h"
#include "Treasure.h"

namespace treasure
{
	static constexpr auto CHECK_OPEN = 1.0f; // 宝箱が開くまでの時間（秒）
}

class TreasureUi : public UiBase
{
typedef UiBase base;
	public:
	TreasureUi();
	virtual ~TreasureUi() = default;
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

	void SetTreasureList(const at::vspc<Treasure>& treasure) { _treasure = treasure; }

protected:
	int _handleDeguti; // 出口画像
	int _handleNokori; // 残り画像
	int _handleKakeru; // 掛ける画像
	at::art<int, 10> _handleNum; // 数字画像

	at::vspc<Treasure> _treasure; // 宝箱のリスト

	int _totalTreasure; // 宝箱の総数
	int _openTreasure; // 開いた宝箱の数
};

