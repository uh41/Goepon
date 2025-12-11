#include "appframe.h"


class ApplicationGlobal
{
public:
	ApplicationGlobal();
	virtual ~ApplicationGlobal();

	bool Init();

	int _cg_cursor;
};

// 他のソースでgGlobalを使えるように
extern ApplicationGlobal gGlobal;
