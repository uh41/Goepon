#include "navigationmesh.h"


NavigationMesh::NavigationMesh(const Polygon3D& mesh)
	: _mesh(mesh)
{
	_bOpenSet = false;
	_bCloseSet = false;
	_bBlocked = false;
	_nParent = nullptr;
	_fCost = 0.0f;
	_fCostToMove = 0.0f;
	_fCostToGoal = 0.0f;
	_pos = vec::Vec3(0.0f, 0.0f, 0.0f);
}

NavigationMesh::~NavigationMesh()
{
	// デストラクタ
}


NavigationMesh* NavigationMesh::FindPath(at::vet<NavigationMesh>& list, NavigationMesh* start, NavigationMesh* goal, float w)
{
	// 経路探索の初期化
	for(auto&& navimesh : list)
	{
		navimesh.Clear();
	}

	if(!goal || !start)
	{
		return nullptr;
	}

	// 最初に調べるナビメッシュ
	NavigationMesh* current = start;
	current->_bCloseSet = true;

	at::vec<NavigationMesh*> openSet;

	// ゴールに到達するまで繰り返す
	while(current != goal)
	{
		for(auto&& AdjacentData : current->_adjacentList)
		{
			// 隣接するナビメッシュのアドレスを取得
			auto adjacent = AdjacentData.first;

			if(AdjacentData.second.segment.Length() < w * 2.0f)
			{
				continue;
			}

			// 使用できない場合はすぐに次へ
			if(adjacent->_bBlocked)
			{
				continue;
			}

			// すでにクローゼットに追加されている場合は次へ
			if(!adjacent->_bCloseSet)
			{
				// まだオープンセットに追加されていない場合
				if(!adjacent->_bOpenSet)
				{
					// 隣接しているノード(ナビメッシュ)の親を自分のノードにする
					adjacent->_nParent = current;

					// 調べるノードを追加
					openSet.emplace_back(adjacent);

					// すでにオープンセットに追加されていることを知らせ、さらに追加される
					adjacent->_bOpenSet = true;

					// コストの追加
					adjacent->_pos = vec::Vec3::Lerp(AdjacentData.second.segment.start, AdjacentData.second.segment.end, 0.5f);

					// 移動コスト
					adjacent->_fCostToMove = vec::Vec3::Length(current->_pos, adjacent->_pos) + current->_fCostToMove;

					// ゴールまでのコスト
					adjacent->_fCostToGoal = vec::Vec3::Length(adjacent->_pos, goal->GetPosition());

					// 合計コスト
					adjacent->_fCost = adjacent->_fCostToMove + adjacent->_fCostToGoal + current->_fCostToMove;
				}
				else
				{
					vec::Vec3 pos = vec::Vec3::Lerp(AdjacentData.second.segment.start, AdjacentData.second.segment.end, 0.5f);

					// 移動コスト
					float costToMove = vec::Vec3::Length(current->_pos, pos) + current->_fCostToMove;

					// ゴールまでのコスト
					float costToGoal = vec::Vec3::Length(adjacent->_pos, goal->GetPosition());

					// 合計コスト
					float newCost = current->_fCostToMove + costToMove + costToGoal;

					// すでに見積れたコストよりも小さい場合は更新する
					if(adjacent->_fCost > newCost)
					{
						adjacent->_nParent = current;
						adjacent->_fCost = newCost;
						adjacent->_pos = pos;
						adjacent->_fCostToGoal = costToGoal;
						adjacent->_fCostToMove = costToMove;

					}
				}
			}
		}

		// オープンセットからコストが一番小さいナビメッシュを探す
		if(openSet.empty())
		{
			break;			// 見つからなかった場合は終了
		}

		// 最もコストが小さいナビメッシュを探す
		auto iter = std::min_element(openSet.begin(), openSet.end(),
			[](NavigationMesh* a, NavigationMesh* b)
			{
				return a->_fCost < b->_fCost;// コストが小さい方を返す
			});

		// 評価しているノードを変更
		current = *iter;

		// オープンセットから削除
		openSet.erase(iter);

		current->_bOpenSet = false;// オープンセットから削除したのでfalseにする
		current->_bCloseSet = true;// クローズセットに追加
	}

	if(current == goal)
	{
		return current;
	}
	else
	{
		return nullptr;
	}
}

void NavigationMesh::RegisterAdjacent(NavigationMesh* adjacent, const AdjacentData& data)
{
	// すでに登録されている場合は登録しない
	for(auto&& a : _adjacentList)
	{
		// 既存の隣接ノードと同じ場合は登録しない
		if(a.first == adjacent)
		{
			return;
		}
	}
	_adjacentList.emplace_back(adjacent, data);
}

void NavigationMesh::CreateList(const at::vec<Polygon3D>& polygon, at::vec<NavigationMesh>& outlist)
{
	// 初期化
	outlist.clear();

	// ナビメッシュの作成
	for(auto&& poly : polygon)
	{
		outlist.emplace_back(NavigationMesh(poly));
	}

	// 隣接リストの作成
	for(int a = 0; a < polygon.size(); a++)
	{
		for(int b = a + 1; b < polygon.size(); b++)
		{
			at::vec<vec::Vec3> vector;

			at::arc<vec::Vec3, 3> aVertexA = { polygon[a]._vVertex1, polygon[a]._vVertex2, polygon[a]._vVertex3 };
			at::arc<vec::Vec3, 3> aVertexB = { polygon[b]._vVertex1, polygon[b]._vVertex2, polygon[b]._vVertex3 };

			at::arc<Segment, 3> segA =
			{
				Segment(polygon[a]._vVertex1, polygon[a]._vVertex2),
				Segment(polygon[a]._vVertex2, polygon[a]._vVertex3),
				Segment(polygon[a]._vVertex3, polygon[a]._vVertex1)
			};

			at::arc<Segment, 3> segB =
			{
				Segment(polygon[b]._vVertex1, polygon[b]._vVertex2),
				Segment(polygon[b]._vVertex2, polygon[b]._vVertex3),
				Segment(polygon[b]._vVertex3, polygon[b]._vVertex1)
			};

			// polys[b] の各頂点が polys[a] の辺上にあるかチェック
			for(int i = 0; i < 3 * 3; i++)
			{
				int j = i / 3;
				int k = i % 3;

				if(Collision::SetPointDistSquare(aVertexB[j], segA[k]) < 0.01f)
				{
					vector.emplace_back(aVertexB[j]);
					break;
				}
			}

			// polys[a] の各頂点が polys[b] の辺上にあるかチェック
			for(int i = 0; i < 3 * 3; i++)
			{
				int j = i / 3;
				int k = i % 3;
				if(Collision::SetPointDistSquare(aVertexA[j], segB[k]) < 0.01f)
				{
					vector.emplace_back(aVertexA[j]);
					break;
				}
			}

			// vectorのsize()が2未満だと、頂点が1つしかないので使用しない
			if(vector.size() < 2)
			{
				continue;
			}

			//頂点情報は、順番に並んでいるため、最初と最後の頂点情報は走査だと調べられないので、走査前にやる
			float longstDisSquare = vec::Vec3::LengthSquare(vector.front(), vector.back());
			
			vec::Vec3 pos = vec::Vec3::Lerp(vector.front(), vector.back(), 0.5f);

			Segment seg(vector.front(), vector.back());

			for(int a = 0; a < vector.size() - 1; a++)
			{
				float disSquare = vec::Vec3::LengthSquare(vector[a], vector[a + 1]);

				if(disSquare > longstDisSquare)
				{
					longstDisSquare = disSquare;
					seg = Segment(vector[a], vector[a + 1]);
					pos = vec::Vec3::Lerp(vector[a], vector[a + 1], 0.5f);
				}
			}

			// 隣接リストに登録
			if(longstDisSquare > 0.001f)
			{
				outlist[a].RegisterAdjacent(&outlist[b], AdjacentData{ seg });
				outlist[b].RegisterAdjacent(&outlist[a], AdjacentData{ seg });
			}
		}
	}
}
