
#pragma once


enum class Game_Actions
{
    shoot = 0,
    accelerate,
    turn_left,
    turn_right,   
    pause,
    COUNT
};


enum class Global_Actions
{
    toggle_fullscreen = 0,
    quit_game,
    COUNT
};

static Action s_game_actions[(u32)Game_Actions::COUNT] = {};
static Action s_menu_actions[(u32)GUI_Menu_Actions::COUNT] = {};
static Action s_global_actions[(u32)Global_Actions::COUNT] = {};


static void load_default_menu_action()
{
    s_menu_actions[(u32)GUI_Menu_Actions::back] = {Key_Code::ESC, Button::BUT_Y};
    s_menu_actions[(u32)GUI_Menu_Actions::enter] = {Key_Code::ENTER, Button::BUT_A};
    s_menu_actions[(u32)GUI_Menu_Actions::up] = {Key_Code::UP, Button::DPAD_UP};
    s_menu_actions[(u32)GUI_Menu_Actions::down] = {Key_Code::DOWN, Button::DPAD_DOWN};
    s_menu_actions[(u32)GUI_Menu_Actions::left] = {Key_Code::LEFT, Button::DPAD_LEFT};
    s_menu_actions[(u32)GUI_Menu_Actions::right] = {Key_Code::RIGHT, Button::DPAD_RIGHT};
    s_menu_actions[(u32)GUI_Menu_Actions::mouse] = {Key_Code::MOUSE_LEFT, Button::NONE};
}


static void load_default_game_action()
{
    s_game_actions[(u32)Game_Actions::shoot] = {Key_Code::SPACE, Button::BUT_A};
    s_game_actions[(u32)Game_Actions::accelerate] = {Key_Code::W, Button::BUT_X};
    s_game_actions[(u32)Game_Actions::turn_left] = {Key_Code::LEFT, Button::DPAD_LEFT};
    s_game_actions[(u32)Game_Actions::turn_right] = {Key_Code::RIGHT, Button::DPAD_RIGHT};
    s_game_actions[(u32)Game_Actions::pause] = {Key_Code::ESC, Button::START};
}


static void load_default_global_actions()
{
    s_global_actions[(u32)Global_Actions::toggle_fullscreen] = { Key_Code::F11, Button::NONE };
    s_global_actions[(u32)Global_Actions::quit_game] = { Key_Code::DEL, Button::NONE };
}
