#include <Windows.h>
#include <iostream>

#include "hook.h"       /* hooking library... currently using the xerox's extremely lightweight one header */

#include "data.h"       /* however you are choosing to impliment a "database" */
#include "mem.h"        /* currently just the pattern scanning function */

PlayerDataManager dataManager;
uintptr_t AGameModePostLogin;
uintptr_t AGameModeLogout;

__forceinline std::wstring getPlayerIP(uintptr_t player)
{
    /* So we've been given an aPlayerController class pointer, which inherets from aController.
    aController + 0x480 has an APlayerState class pointer, which contains the player information we're intrested
    in. Offsets or class implimentations may change in a future update */

    uintptr_t aPlayerState = *(uintptr_t*)(player + 0x480);
    wchar_t* clientIP = *(wchar_t**)(aPlayerState + 0x540);

    return std::wstring(clientIP);
}

__int64 __fastcall aGameModePostLoginHook(uintptr_t pThis, uintptr_t newPlayer)
{
    /* Gets called after a player has succesfully logged in, passing in the aPlayerController* 
    of the new player as it's second param. */

    /* let's execute the original aGameMode::PostLogin function before we do anything, and store
    it's return value for later */
    __int64 r;
    typedef __int64(__fastcall* OriginalPostLogin)(uintptr_t pThis, uintptr_t newPlayer);
    OriginalPostLogin AGameModePostLoginFunc = OriginalPostLogin(AGameModePostLogin);
    /* why make a trampoline when you can just do this! jk... but who cares really? */
    hook::disable((void*)AGameModePostLogin);
    r = AGameModePostLoginFunc(pThis, newPlayer);
    hook::enable((void*)AGameModePostLogin);

    /* having executed the original function, let's add them to our list of saved players.
    for demonstration purpouse the PlayerDataManager just uses a vector, but you should be able
    to change implimentation to use a database instead to add communication
    between multiple players, and kick duplicate players */
    
    std::wstring newPlayerIP = getPlayerIP(newPlayer);
    wprintf(L"[+] Player with IP %s has connected \n", newPlayerIP.c_str());

    if(!dataManager.Exists(newPlayerIP))
        dataManager.Insert(newPlayerIP);
    else
    {
        // kick newPlayer implimenation 
    }

    return r;
}

__int64 __fastcall aGameModeLogoutHook(uintptr_t pThis, uintptr_t oldPlayer)
{
    /* Gets called when a player is logging out, passing in the aPlayerController* 
    of the player logging out as it's second param. */

    /* let's execute the original aGameMode::Logout function before we do anything, and store
    it's return value for later */
    __int64 r;
    typedef __int64(__fastcall* OriginalLogout)(uintptr_t pThis, uintptr_t oldPlayer);
    OriginalLogout AGameModeLogoutFunc = OriginalLogout(AGameModeLogout);
    /* why make a trampoline when you can just do this! jk... but who cares really? */
    hook::disable((void*)AGameModeLogout);
    r = AGameModeLogoutFunc(pThis, oldPlayer);
    hook::enable((void*)AGameModeLogout);

    /* having executed the original function, let's remove them from our list of saved players */
    wprintf(L"[-] Player with IP %s has disconnected \n", getPlayerIP(oldPlayer).c_str());
    dataManager.Remove(getPlayerIP(oldPlayer));

    return r;
}

bool Init()
{
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    /* On initialization, we need to find the functions a game will call when a player
    logs into the server, logs out of the server, and how we can kick a player.
    If we fail to find any of these three, then we don't run. */
    uintptr_t ModuleBase = (uintptr_t)GetModuleHandleW(L"ShooterGameServer.exe");
    if (!ModuleBase) return false;

    /* In the future it would be smart to add either additional functions that server the same purpouse,
    or additional ways of finding these functions - preventing the plugin from breaking if the signature on any of these
    changes due to an update... */
    AGameModePostLogin = PatternScan(ModuleBase, "40 ? 53 56 57 41 ? 41 ? 41 ? 48 8B ? 48 81 EC ? ? ? ? 48 C7 45 B0 ? ? ? ? 48 8B");
    AGameModeLogout = PatternScan(ModuleBase, "48 89 ? ? ? 57 48 83 EC ? 48 8B ? 48 8B ? E8 ? ? ? ? 48 8B ? 48 85 ? 74 ? 48 8B ? 48 8B");

    if (!AGameModeLogout || !AGameModeLogout) return false;

    hook::make_hook((void*)AGameModePostLogin, aGameModePostLoginHook);
    hook::make_hook((void*)AGameModeLogout, aGameModeLogoutHook);

    return true;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        Init();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

