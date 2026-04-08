#pragma once
#include "uibase.h"
#include "Makimono.h"
namespace henshin
{
	static constexpr auto HENSHIN_X  = 1400;
	static constexpr auto HENSHIN_Y  = 800; 
	static constexpr auto TANUBITO_X = 1557;
	static constexpr auto TANUBITO_Y = 799;
	static constexpr auto TANUMONO_X = 1432;
	static constexpr auto TANUMONO_Y = 895;
}

class HenshinUi : public UiBase
{
	typedef UiBase base;
public:

	// 変身UIの選択肢
	enum class Select : int
	{
		TANUKI   = 0,
		TANUBITO = 1,
		TANUMONO = 2,
		Count    = 3, 
		_EOT_ 
	};

	HenshinUi();
	virtual ~HenshinUi() = default;
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	// 所有者のセッター
	void SetOwner(void* owner) { _owner = owner; } 

	// 表示フラグのセッター
	void SetShowPlayerUi(bool show) { _showPlayerUi = show; }
	void SetShowPlayerMonoUi(bool show) { _showPlayerMonoUi = show; }

protected:

	// 取得関係
	Makimono* _makimono;      
	Select    _select;         

	// グラフィックハンドル
	int       _handleMain;
	int       _handleTanubito; 
	int       _handleMono;     
	int       _handleNotCg;    // 変身ができないときに表示するUIのグラフィックハンドル

	// パッド入力5Aが有効かどうか
	bool _padInput5Active;	   

	// このUIを所有するオブジェクトへのポインタ
	void* _owner;             

	// プレイヤーUIとプレイヤーモノUIの表示フラグ
	bool _showPlayerUi;       
	bool _showPlayerMonoUi;    
};

