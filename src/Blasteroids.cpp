
#define _DB

#include <intrin.h>
#include "Game/Asteroids.h"	

#include "Platform/Azewin.h"
#include "Platform/Windows_Keymap.h"
#include "Platform/Windows.cpp"

s32 WinMain(WInstanceHandle instance, WInstanceHandle pre_instance, char* cmd, s32 show_command)
{    
	Asteroids();
	return 0;
}
