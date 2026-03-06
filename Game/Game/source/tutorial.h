#pragma once
#include "objectbase.h"
class Tutorial : public ObjectBase
{
	typedef ObjectBase base;
public:
	Tutorial();
	virtual ~Tutorial() = default;
	bool Initialize() override; // 初期化
	bool Terminate()  override; // 終了
	bool Process()    override; // 処理
	bool Render()     override; // 描画

	auto GetTutorialCollisionFrame() const { return _tutorialCollisionFrame; }

	MATRIX MakeModelMatrix() const;

	auto GetEventId() const { return _eventId; }
	void SetEventId(int id) { _eventId = id; }

	void PlayTutorial();// チュートリアルエフェクトの再生

	void LoadEffect();// イベントの読み込み

protected:

	at::umtt<std::string, int> _eventHandle; // イベント名とハンドル

	int _tutorialCollisionFrame; // 当たり判定用フレーム
	int _attachIndex;             // モデルの当たり判定用フレームのアタッチ番号
	bool _isVisible;              // 表示フラグ
	int _eventId;                  // イベントID
};

