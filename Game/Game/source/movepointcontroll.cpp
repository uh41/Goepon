#include "movepointcontroll.h"
#include <limits>

MovePointControll::MovePointControll()
{
	_index = 0;
}

void MovePointControll::SetMovePoint(const at::vet<vec::Vec3>& point)
{
	// ★★★ デバッグ出力：受信した配列サイズ ★★★
	char buf[256];
	sprintf_s(buf, "MovePointControll::SetMovePoint: 受信したポイント数 = %d\n", point.size());
	OutputDebugString(buf);

	// ★★★ デバッグ出力：各ポイントの座標 ★★★
	for(size_t i = 0; i < point.size(); i++)
	{
		sprintf_s(buf, "  受信Point[%d]: (%.1f, %.1f, %.1f)\n", i, point[i].x, point[i].y, point[i].z);
		OutputDebugString(buf);
	}

	_movePoints = point;
	_index = 0;

	// ★★★ デバッグ出力：コピー後の確認 ★★★
	sprintf_s(buf, "MovePointControll: コピー後 _movePoints.size() = %d\n", _movePoints.size());
	OutputDebugString(buf);

	if(!_movePoints.empty())
	{
		sprintf_s(buf, "  _movePoints[0]: (%.1f, %.1f, %.1f)\n", 
			_movePoints[0].x, _movePoints[0].y, _movePoints[0].z);
		OutputDebugString(buf);
	}
}

vec::Vec3 MovePointControll::GetTargetPoint() const
{
	// ★★★ デバッグ出力：現在の状態 ★★★
	char buf[256];
	sprintf_s(buf, "GetTargetPoint: _movePoints.size()=%d, _index=%d\n", 
		_movePoints.size(), _index);
	OutputDebugString(buf);

	if(_movePoints.empty())
	{
		OutputDebugString("警告: _movePoints が空です！(0,0,0) を返します\n");
		return vec3::VGet(0.0f, 0.0f, 0.0f);
	}

	vec::Vec3 target = _movePoints[_index];
	
	// ★★★ デバッグ出力：返す座標 ★★★
	sprintf_s(buf, "  返すポイント[%d]: (%.1f, %.1f, %.1f)\n", 
		_index, target.x, target.y, target.z);
	OutputDebugString(buf);

	return target;
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