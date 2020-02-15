#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zCVob.h"
#include "zCWorld.h"
#include "zCBspTree.h"

class zCView;

enum class GOTHIC_KEY : int
{
	F1 = 0x3B,
	F2 = 0x3C,
	F3 = 0x3D,
	F4 = 0x3E,
	F5 = 0x3F,
	F6 = 0x40,
	F7 = 0x41,
	F8 = 0x42,
	F9 = 0x43,
	F10 = 0x44,
};

class oCNPC;
class oCGame
{
public:

	/** Hooks the functions of this Class */
	static void Hook()
	{
		XHook(HookedFunctions::OriginalFunctions.original_oCGameEnterWorld, GothicMemoryLocations::oCGame::EnterWorld, oCGame::hooked_EnterWorld);
	}

	static void __fastcall hooked_EnterWorld(void * thisptr, void * unknwn, oCNPC* playerVob, int changePlayerPos, const zSTRING& startpoint)
	{
		HookedFunctions::OriginalFunctions.original_oCGameEnterWorld(thisptr, playerVob, changePlayerPos, startpoint);

		/*if (!Engine::GAPI->GetLoadedWorldInfo()->BspTree) // Happens in Gothic II - Johannes Edition, zCBspTree::LoadBIN isn't called for some reason
		{
			zCWorld* w = (zCWorld *)thisptr;

			LogWarn() << "Weird ZEN-File: zCBspTree::LoadBIN wasn't called, trying to load geometry now...";

			// Load the world-geometry now
			zCBspTree::LoadLevelGeometry(w->GetBspTree());
		}*/

		Engine::GAPI->OnWorldLoaded();

		// TODO: Player sometimes gets invisible, apparently.
		// Re-Add the player npc to the world because it sometimes would be invisible after a world-change
		//auto const&& player = (zCVob *)oCGame::GetPlayer();
		//auto const&& playerHomeworld = player->GetHomeWorld();
		//Engine::GAPI->OnRemovedVob(player, playerHomeworld);
		//Engine::GAPI->OnAddVob(player, playerHomeworld);
	}

	void TestKey(GOTHIC_KEY key)
	{
		XCALL(GothicMemoryLocations::oCGame::TestKeys);
	}

	static oCNPC* GetPlayer()
	{
		return *(oCNPC **)GothicMemoryLocations::oCGame::Var_Player;
	}

	zCView* GetGameView()
	{
		return *(zCView **)THISPTR_OFFSET(GothicMemoryLocations::oCGame::Offset_GameView);
	}

	bool GetSingleStep()
	{
#ifdef BUILD_SPACER
		return false;
#else
		return (*(int *)THISPTR_OFFSET(GothicMemoryLocations::oCGame::Offset_SingleStep)) != 0;
#endif
	}

    int _vtbl;
    int _zCSession_csMan;        //zCCSManager*
    zCWorld* _zCSession_world;        //zCWorld*
    int _zCSession_camera;       //zCCamera*
    int _zCSession_aiCam;        //zCAICamera*
    zCVob * _zCSession_camVob;       //zCVob *
    zCView* _zCSession_viewport;     //zCView*

	static oCGame* GetGame() { return *(oCGame**)GothicMemoryLocations::GlobalObjects::oCGame; };
};