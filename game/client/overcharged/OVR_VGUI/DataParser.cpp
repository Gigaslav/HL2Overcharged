#include "cbase.h"
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/IInput.h>
#include "tier0/vprof.h"
#include "iclientmode.h"
#include <vgui_controls/Panel.h>
#include "overcharged/OVR_VGUI/DataParser.h"
#include <vgui/IInputInternal.h>
#include <vgui_controls/Controls.h>
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#define CONSOLE_INFO false

int GetData(IFileSystem* filesystem, const char *name)
{
	char GameInfoPath[MAX_PATH];
	g_pFullFileSystem->RelativePathToFullPath("cfg/OverCharged/Settings.cfg", "GAME", GameInfoPath, sizeof(GameInfoPath));
	Q_FixSlashes(GameInfoPath, '/');

	int value = 0;

	KeyValues *st = new KeyValues("Settings");
	if (st->LoadFromFile(filesystem, GameInfoPath, "GAME"))
	{
		if (st)
		{
			KeyValues *vars = st->GetFirstSubKey();
			if (vars)
			{
				KeyValues *pMainKey = st->FindKey(name);
				if (pMainKey)
				{
					KeyValues *pSubKey = pMainKey->GetFirstSubKey();
					if (pSubKey)
						value = pSubKey->GetInt();
				}
			}
		}
	}
	st->deleteThis();

	return value;
}
string_t GetDataString(IFileSystem* filesystem, ConVar* pCvar, const char *name)
{
	char GameInfoPath[MAX_PATH];
	g_pFullFileSystem->RelativePathToFullPath("cfg/OverCharged/Settings.cfg", "GAME", GameInfoPath, sizeof(GameInfoPath));
	Q_FixSlashes(GameInfoPath, '/');

	string_t value = "";

	KeyValues *st = new KeyValues("Settings");
	if (st->LoadFromFile(filesystem, GameInfoPath, "GAME"))
	{
		if (st)
		{
			KeyValues *vars = st->GetFirstSubKey();
			if (vars)
			{
				/*KeyValues *key = st->FindKey(name);
				if (key)
				{
					value = key->GetString();
					pCvar->SetValue(key->GetString());
				}*/

				KeyValues *pMainKey = st->FindKey(name);
				if (pMainKey)
				{
					KeyValues *pSubKey = pMainKey->GetFirstSubKey();
					if (pSubKey)
					{
						value = pSubKey->GetString();
						pCvar->SetValue(pSubKey->GetString());
					}
				}
			}
		}
	}
	st->deleteThis();

	return value;
}

void SetData(IFileSystem* filesystem, vector<ConVar*> *gp_globalsCvarNames, int type)
{
	char GameInfoPath[MAX_PATH];
	g_pFullFileSystem->RelativePathToFullPath("cfg/OverCharged/Settings.cfg", "GAME", GameInfoPath, sizeof(GameInfoPath));
	Q_FixSlashes(GameInfoPath, '/');

	KeyValues *st = new KeyValues("Settings");
	if (st->LoadFromFile(filesystem, GameInfoPath, "GAME"))
	{
		if (st)
		{
			KeyValues *vars = st->GetFirstSubKey();
			if (vars)
			{
				int count = gp_globalsCvarNames->size();

				for (int i = 0; i < count; i++)
				{
					const char * name = gp_globalsCvarNames->at(i)->GetName();

					if (type == 0)
					{
						int value = gp_globalsCvarNames->at(i)->GetInt();

						KeyValues *key = st->FindKey(name);
						if (key)
						{
							key->SetInt(name, value);
						}


						/*KeyValues *pMainKey = st->FindKey(name);
						if (pMainKey)
						{
							KeyValues *pSubKey = pMainKey->GetFirstSubKey();
							if (pSubKey)
							{
								pSubKey->SetInt(name, value);
							}
						}*/
					}
					else if (type == 1)
					{
						const char* value = gp_globalsCvarNames->at(i)->GetString();

						/*KeyValues *key = st->FindKey(name);
						if (key)
						{
							key->SetStringValue(value);
						}*/

						KeyValues *pMainKey = st->FindKey(name);
						if (pMainKey)
						{
							KeyValues *pSubKey = pMainKey->GetFirstSubKey();
							if (pSubKey)
							{
								pSubKey->SetStringValue(value);
							}
						}
					}
				}
			}
		}
	}
	/* bool bSuccess = */ st->SaveToFile(g_pFullFileSystem, GameInfoPath, NULL);
	st->deleteThis();
}