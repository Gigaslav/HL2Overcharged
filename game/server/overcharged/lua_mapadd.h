#include "cbase.h"
#include "OverCharged/luamanager.h"

class MapAddLua : public LuaHandle
{
public:
	MapAddLua();
	~MapAddLua();

	void Init();
	void Shutdown();

	bool OpenFile(const char* filename);
	void OpenCommon();
	void CallFunction(const char* function);

	void RegFunctions();
	void RegGlobals();

private:
	bool m_bLuaLoaded;
	bool m_bCommonLuaLoaded;
	unsigned m_szNewBuffer;
	unsigned m_szOldBuffer;

	unsigned m_szNewCommonBuffer;
	unsigned m_szOldCommonBuffer;
};

MapAddLua* g_LuaHandle = NULL;
extern MapAddLua *GetLuaHandle();
