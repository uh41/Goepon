#pragma once
#include "appframe.h"

class MovePointControll
{
public:
	MovePointControll();
	~MovePointControll() = default;

	// ポイントを設定(最低でも2点は必要)
	void SetMovePoint(const at::vet<vec::Vec3>& point);

	// 次の目標を取得
	vec::Vec3 GetTargetPoint() const;

	// 目標に到達したかどうか
	bool IsReachTarget(const vec::Vec3& pos, float threshold = 10.0f) const;

	// 次の目標ポイントへ進む
	void MoveToNextPoint();

	// 現在の目標ポイントのインデックスを取得
	int GetMovePointIndex() const { return _index; }

	// インデックスを設定
	void SetMovePointIndex(int index);

	int FindNearPointIndex(const vec::Vec3& pos) const;

	// ポイント総数
	int GetMovePointCount() const { return static_cast<int>(_movePoints.size()); }

	// 有効かどうか(裁定でも2点以上必要)
	bool IsValid() const { return _movePoints.size() >= 2; }

private:
	at::vet<vec::Vec3> _movePoints;	// 移動ポイントのリスト
	int _index;                     // 現在の目標ポイントのインデックス

};

