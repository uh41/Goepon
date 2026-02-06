#include "movepointcontroll.h"
#include <limits>

MovePointControll::MovePointControll()
{
	_index = 0;
}

void MovePointControll::SetMovePoint(const at::vet<vec::Vec3>& point)
{
	_movePoints = point;
	_index = 0;
}

vec::Vec3 MovePointControll::GetTargetPoint() const
{
	if(_movePoints.empty())
	{
		// ★★★ デバッグ出力を追加 ★★★
		OutputDebugString("警告: _movePoints が空です！\n");
		return vec3::VGet(0.0f, 0.0f, 0.0f); // ★ これが原因
	}

	return _movePoints[_index];
}

bool MovePointControll::IsReachTarget(const vec::Vec3& pos, float threshold) const
{
	if(_movePoints.empty())
	{
		return false;
	}

	vec::Vec3 target = _movePoints[_index];
	vec::Vec3 diff = vec3::VSub(target, pos);// 差分ベクトル
	diff.y = 0.0f; // Y方向は無視

	float distSq = diff.LengthSquare();			// 距離の二乗

	return distSq < (threshold * threshold);	// 閾値の二乗と比較
}

void MovePointControll::MoveToNextPoint()
{
	if(_movePoints.empty())
	{
		return;
	}

	_index++;
	if(_index >= static_cast<int>(_movePoints.size()))
	{
		_index = 0; // ループさせる
	}
}

void MovePointControll::SetMovePointIndex(int index)
{
	if(_movePoints.empty())
	{
		return;
	}

	// 範囲内に収める
	if(index < 0 || index >= static_cast<int>(_movePoints.size()))
	{
		_index = 0;
	}
	else
	{
		_index = index;
	}
}

int MovePointControll::FindNearPointIndex(const vec::Vec3& pos) const
{
	if(_movePoints.empty())
	{
		return 0;
	}

	int nearIndex = 0;
	float minDistSq = FLT_MAX;	// 十分大きな値で初期化

	// 全ポイントをチェック
	for(int i = 0; i < static_cast<int>(_movePoints.size()); i++)
	{
		vec::Vec3 diff = vec3::VSub(_movePoints[i], pos);// 差分ベクトル
		diff.y = 0.0f; // Y方向は無視
		float distSq = diff.LengthSquare();// 距離の二乗

		// 最小距離を更新
		if(distSq < minDistSq)
		{
			minDistSq = distSq;
			nearIndex = i;
		}
	}

	return nearIndex;
}