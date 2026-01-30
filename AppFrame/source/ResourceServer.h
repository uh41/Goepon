#pragma once
#include "container.h"

/*********************************************************************/
// * \file  ResourceServer.h
// * \brief  リソースサーバークラス
// *
// * \author 石森虹大
// * \date   2025/12/28
// * \作業内容: 新規作成 石森虹大　2025/12/28
/*********************************************************************/


// リソース管理サーバ
// すべて静的メンバで構成する
class ResourceServer
{
public:
	static	void	Init();    // 初期化
	static	void	Release(); // 終了

	static	void	ClearGraph();
	static	int		LoadGraph(const TCHAR* FileName);
	static int		LoadDivGraph(const TCHAR* FileName, int AllNum,
		int XNum, int YNum,
		int XSize, int YSize, int* HandleBuf);

	static int		LoadDivGraph(const TCHAR* FileName, int AllNum,
		int XNum, int YNum,
		int XSize, int YSize, std::vector<int>& HandleBuf);

	static int		MV1LoadModel(const TCHAR* FileName);
	static int		MV1DeleteModel(int handle);

private:
	static std::unordered_map<std::string, int>	_mapGraph;
	typedef struct
	{
		int		AllNum;
		int* handle;
	} DIVGRAPH;
	static std::unordered_map<std::string, DIVGRAPH>	_mapDivGraph;

	static std::unordered_map<std::string, int>	_mapModel;
};
