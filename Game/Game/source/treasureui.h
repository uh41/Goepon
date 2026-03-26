#pragma once
#include "counterui.h"
#include "TreasureBase.h"

class ModeGame;

namespace treasure
{
	static constexpr auto DEGUTI_X = 1600;   // 出口のX座標
	static constexpr auto DEGUTI_Y = 50;     // 出口のY座標
	static constexpr auto NOKORI_X = 1600;   // 残りのX座標
	static constexpr auto NOKORI_Y = 50;     // 残りのY座標
	static constexpr auto KAKERU_X = 1500;   // 掛けるのX座標
	static constexpr auto KAKERU_Y = 50;     // 掛けるのY座標
	static constexpr auto GOEXIT_X = 200;	     // メッセージのX座標
	static constexpr auto GOEXIT_Y = 700;    // メッセージのY座標

	static constexpr float GO_DEGUTI_DURATION = 3.0; // お宝をすべて獲得してから出口UIを表示する秒数
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

	void SetTreasureList(const at::vspc<TreasureBase>& treasure)
	{
		_treasure = treasure;
		_isTreasureListAssigned = true;
	}
	void SetOwner(ModeGame* owner) { _ownerGame = owner; }

protected:
	int _handleDeguti;   // 出口画像
	int _handleNokori;   // 残り画像
	int _handleGoDeguti; // お宝をすべて獲得したら一回だけ出るUI

	int _remainCount;   // 残りの宝箱の数
	int _treasureCount; // 宝箱の総数（このUIが認識している数）


	bool _isTreasureListAssigned; // SetTreasureList() が呼ばれたか

	// 表示時間制御
	bool _isGoDegutiShowActive; // 巻物UIを表示中か
	float _goDegutiShowElapsed; // 表示開始からの経過秒
	
	at::vspc<TreasureBase> _treasure; // 宝箱のリスト
	ModeGame* _ownerGame;             // 所有するゲームモードへのポインタ
};