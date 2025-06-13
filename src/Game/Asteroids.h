
#pragma once

#include "Utility/Primitives.h"
#include "Utility/Assert.h"
#include "Utility/Color.h"
#include "Utility/Vector.h"
#include "Utility/Vector_Ex.h"
#include "Utility/Rect.h"
#include "Game/Random_Machine.h"
#include "Utility/Range.h"

#include "Platform/Input.h"
#include "Platform/OS.h"

#include "Utility/Bits.h"
#include "Utility/Maths.h"
#include "Utility/Utility.h"
#include "Utility/Allocator_Shell.h"
#include "Utility/Allocator.h"
#include "Sound/Sound.h"
#include "Sound/Sound.cpp"

#include "Game/Mesh.h"
#include "Game/Terminus_Font.h"
#include "Game/Action.h"
#include "Game/Pixel_Canvas.h"
#include "Game/Pixel_Canvas.cpp"
#include "Game/GUI.h"
#include "Game/GUI.cpp"
#include "Game/Action_Definitions.h"
#include "Game/Entity.h"
#include "Game/Asteroid_Maths.h"

#include "Game/Score.cpp"
#include "Game/Entity_Definitions.h"
#include "Game/Game_State.h"
#include "Game/Game_State.cpp"
#include "Game/Particle_System.h"
#include "Game/Particle_System.cpp"
#include "Game/Weapon_Factory.h"
#include "Game/Weapon_Factory.cpp"
#include "Game/Mesh_Factory.h"
#include "Game/Serialization.cpp"
#include "Game/GUI_Factory.h"
#include "Game/AI.h"
#include "Game/AI.cpp"
#include "Game/Waves.h"
#include "Game/Asteroids.cpp"


static inline void Asteroids()
{
    init_asteroids_game();

    bool update_surface = false;

    while(update_asteroids_game(Platform_Get_Frame_Time(), update_surface))
    {
        Platform_Swap_Buffer_Type swap_buffer = Platform_Swap_Buffer_Type::none;
        if(update_surface)
        {
            swap_buffer = Platform_Swap_Buffer_Type::software;
            update_surface = false;
        }

        Platform_Update(swap_buffer);
    }
}