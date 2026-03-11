#pragma once
#include "counterui.h"
#include "Treasure.h"

namespace treasure
{
	static constexpr auto DEGUTI_X = 1700; // 出口のX座標
	static constexpr auto DEGUTI_Y = 60; // 出口のY座標
	static constexpr auto NOKORI_X = 1650; // 残りのX座標
	static constexpr auto NOKORI_Y = 60; // 残りのY座標
	static constexpr auto KAKERU_X = 1650; // 掛けるのX座標
	static constexpr auto KAKERU_Y = 920; // 掛けるのY座標
	static constexpr auto MAKIMONO_X = 1554; // 巻物のX座標
	static constexpr auto MAKIMONO_Y = 820; // 巻物のY座標
}

class TreasureUi : public CounterUi
{
typedef CounterUi base;
	public:
	TreasureUi();
	virtual ~TreasureUi() = default;
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

	void SetTreasureList(const at::vspc<Treasure>& treasure) { _treasure = treasure; }

	bool GetHandleMakimono();

protected:
	int _handleDeguti; // 出口画像
	int _handleNokori; // 残り画像
	int _handleMakimono; // 巻物画像

	int _remainCount; // 残りの宝箱の数

	at::vspc<Treasure> _treasure; // 宝箱のリスト
};

