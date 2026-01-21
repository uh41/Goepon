/*********************************************************************/
// * \file   ResourceServer.h
// * \brief  リソース管理クラス
// *
// * \author 石森虹大
// * \date   2026/01/08
// * \作業内容: 新規作成 石森虹大　2026/01/08
/*********************************************************************/
#include "../container.h"

class ResourceServer
{
public:
	// 初期化処理
	static void	Init();

	// 終了処理
	static void	Release();

	// グラフィック関連
	static void ClearGraph();

	// モデル読み込み関数
	static int LoadGraph(const TCHAR* FileName);
	static int LoadDivGraph(const TCHAR* FileName, int AllNum,
		                    int XNum, int YNum,
		                    int XSize, int YSize, int* HandleBuf);

	static int LoadDivGraph(const TCHAR* FileName, int AllNum,
		int Xnum, int YNum, int XSize);

	static int MV1LoadModel(const TCHAR* FileName);

	static int MV1DeleteModel(int handle);
private:
	static std::unordered_map<std::basic_string<TCHAR>, int> _mapGraph;
	typedef struct
	{
		int AllNum;
		int* handle;
	}DIVGRAPH;
	static std::unordered_map<std::basic_string<TCHAR>, DIVGRAPH> _mapDivGraph;

	static std::unordered_map<std::basic_string<TCHAR>, int> _mapModel;

	static std::unordered_map<std::basic_string<TCHAR>, int> _mapDivModel;
};
