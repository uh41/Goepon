#pragma once
#include "../"
class ResourceServer
{
public:
	static	void	Init();
	static	void	Release();

	static	void	ClearGraph();
	static	int		LoadGraph(const TCHAR* FileName);
};

