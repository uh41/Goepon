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
	static void	Init();
	static void	Release();

	static void ClearGraph();

	static int LoadGraph   (const TCHAR* FileName);
	static int LoadDivGraph(const TCHAR* FileName, int AllNum,
		                          int Xnum, int YNum, int XSize,
		                          int HandleBuf                 );

	static int LoadDivGraph(const TCHAR* FileName, int AllNum,
							      int Xnum, int YNum, int XSize);

	static int MV1LoadModel(const TCHAR* FileName);

	static int MV1DeleteModel(int handle);
private:
	static std::unordered_map<std::basic_string<TCHAR>, int> _graphMap;
	typedef struct
	{
		int AllNum;
		int* handle;
	}DIVGRAPH;
	static std::unordered_map<std::basic_string<TCHAR>, DIVGRAPH> _mapDivGraph;

	static std::unordered_map<std::basic_string<TCHAR>, int> _modelMap;

	static std::unordered_map<std::basic_string<TCHAR>, int> _divModelMap;
};
