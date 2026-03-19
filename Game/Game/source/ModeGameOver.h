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
	ModeGame* _ownerGame; // 所有している ModeGame への参照（必要に応じて使用）
	int _overlayHandle;
	bool _showOverlayImmediate;
	std::string _debugCurrentStageId; // デバッグ用：検出したステージID

	// 円形暗転
	float _fadeSec = 0.0f;
	float _fadeDurationSec = 1.2f;
	int _screenCx = 0;
	int _screenCy = 0;
	float _spotRadius = 0.0f;
	float _spotStartRadius = 900.0f;
	float _spotEndRadius = 0.0f;
	int _spotRingStepDeg = 6;
	int _spotAlpha = 0;
	bool _hasValidSpotCenter = false;

	bool UpdateSpotCenterFromPlayer();
	void DrawSpotlightFade() const;
};