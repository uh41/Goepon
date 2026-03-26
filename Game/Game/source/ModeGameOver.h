#pragma once
#include "appframe.h"
#include "modegame.h"

class ModeGameOver :public ModeBase
{
	typedef ModeBase base;
public:
	// オーナーを受け取るコンストラクタを用意（nullptr可）
	explicit ModeGameOver(ModeGame* ownerGame = nullptr)
		: _ownerGame(ownerGame)
		, _overlayHandle(-1)
		, _showOverlayImmediate(false)
	{
	}

	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

private:
	bool UpdateSpotCenterFromPlayer(); // プレイヤーの位置からスポットライトの中心を更新し、成功したかを返す
	void DrawSpotlightFade() const;    // 円形暗転を描画する関数
	
	int _gameoverLogoHandle;

	ModeGame* _ownerGame; // 所有している ModeGame への参照（必要に応じて使用）
	int _overlayHandle;
	bool _showOverlayImmediate;
	std::string _debugCurrentStageId; // デバッグ用：検出したステージID

	// 円形暗転
	float _fadeSec = 0.0f;			   // フェードの進行度（秒）
	float _fadeDurationSec = 1.2f;	   // 画面中心座標（スポットライトの中心）
	int   _screenCx = 0;			   // 画面中心座標（スポットライトの中心）
	int   _screenCy = 0;			   // スポットライトの半径（最初は大きく、徐々に縮める）
	float _spotRadius = 0.0f;		   // 現在の穴の半径
	float _spotStartRadius = 900.0f;   // 穴の初期半径
	float _spotEndRadius = 0.0f;	   // 穴の最終半径
	int   _spotRingStepDeg = 6;		   // 360度を何度ごとに分割してポリゴンを描くか
	int   _spotAlpha = 0;			   // 暗転のアルファ値（0-255）
	bool  _hasValidSpotCenter = false; // playerのワールド座標からスクリーン座標変化が成功したか
	bool  _transitionStarted = false; // ゲームオーバーから次のモードへの遷移が要求されたか
};