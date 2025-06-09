

// ===================================
// Copyright (c) 2024 by Valtteri Kois
// All rights reserved.
// ===================================

#pragma once

// ------------------------------------------------------------------
#define PER_THREAD_MEMORY_AMOUNT 52428800
// ------------------------------------------------------------------

#ifndef COMPILE_OPENGL
#define COMPILE_OPENGL 0
#endif


static _wglCreateContextAttribsARB*  wglCreateContextAttribsARB = 0;
static _wglChoosePixelFormatARB*     wglChoosePixelFormatARB    = 0;
static _wglSwapIntervalEXT*          wglSwapIntervalEXT         = 0;
static XInputGetState*              _XInputGetState             = 0;


constexpr s32 s_desired_opengl_major_version = 3;

constexpr s32 s_opengl_context_attribs[] = 
{
    WGL_CONTEXT_MAJOR_VERSION_ARB, s_desired_opengl_major_version,
    WGL_CONTEXT_MINOR_VERSION_ARB, 0,
    WGL_CONTEXT_FLAGS_ARB, 0,
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
    0
};

#if 0
constexpr s32 s_opengl_context_attribs[] = 
{
    WGL_CONTEXT_MAJOR_VERSION_ARB, s_desired_opengl_major_version,
    WGL_CONTEXT_MINOR_VERSION_ARB, 0,
    WGL_CONTEXT_FLAGS_ARB,         0,
    //WGL_RED_BITS_ARB,              8,
    //WGL_GREEN_BITS_ARB,            8,
    //WGL_BLUE_BITS_ARB,             8,
    //WGL_ALPHA_BITS_ARB,            8,
    //WGL_DEPTH_BITS_ARB,            32,
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
    0
};
#endif


struct Bitmap
{
    Color* buffer;
    u32 allocated_capacity;
    s32 width;
    s32 height;
};
static Bitmap s_software_render_target = {};


struct Platform_State
{
    WWindowHandle window;
    WDeviceHandle device_context;
    
    f64 frame_time;
    u64 frame_count;
    u64 flags;
    u64 system_clock_freqeuncy;
    u64 last_time_stamp;
    s32 opengl_context_version;
    s32 vsync_interval;
    v2s window_dimensions;
    f32 scroll_delta;
    
    Controller_State controller_state[1];
    f64 next_controler_test_time;
    
    Char_Array typing_information;
    WSystemInfo system_info;
    WWindowPlacement window_placement;
    
    volatile u32 main_thread_id;
    
    u32 opengl_framebuffer;
    u32 opengl_framebuffer_texture2D;
};
static Platform_State s_platform_state = {};


struct Thread_Info
{
    u32 idx;
};


struct Work_Queue_Entry
{
    Platform_Work_Function work_function;
    Work_Data work_data;
};


struct Thread_Queue
{
    Work_Queue_Entry tasks[64];
    volatile u32 task_write_head;
    volatile u32 task_read_head;
    
    WHandle semaphore_handle;
};
static Thread_Queue s_thread_queue;


#if 1
struct DirectSound
{
    WCOMObjectDirectSoundBuffer* com;
    s32 samples_per_second;
    s16 channels;
    s16 bits_per_sample_per_channel;
    s16 bytes_per_sample;
    u32 post_play_cursor_sample_count;
    u64 sample_write_counter;
    f64 accumulated_frame_time;
    
    u32 last_write_cursor;
    
    u32 client_buffer_size;
    Target_Buffer_Sound_Sample* client_buffer;
    
    bool first_time_output;
};
static DirectSound s_sound = {};
#endif


static Platform_Flags::T Platform_Get_Flags()
{
    return Platform_Flags::T(s_platform_state.flags);
}


static bool Set_VSync(s32 interval)
{
    bool result = false;
    
    if(wglSwapIntervalEXT && wglSwapIntervalEXT(interval))
    {                
        s_platform_state.vsync_interval = interval;
        result = true;
        
        Inverse_Bit_Mask(&s_platform_state.flags, Platform_Flags::vsync);
        if(interval)
        {
            s_platform_state.flags |= Platform_Flags::vsync;
        }
    }
    
    return result;
}


u32 __stdcall Worker_Thread_Procedure(void* param)
{
    Thread_Info thread_info = *(Thread_Info*)param;
    
    u32 max_tasks = Array_Lenght(s_thread_queue.tasks);
    Linear_Allocator thread_work_memory = {};
    
    void* thread_memory = VirtualAlloc(0, PER_THREAD_MEMORY_AMOUNT, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    thread_work_memory.init(thread_memory, PER_THREAD_MEMORY_AMOUNT);
    
    for(;;)
    {        
        u32 expected_read_head_pos = s_thread_queue.task_read_head;
        
        if(expected_read_head_pos != s_thread_queue.task_write_head)
        {
            u32 next_read_head_position = (expected_read_head_pos + 1) % max_tasks;
            u32 work_idx = _InterlockedCompareExchange(
                                                       (volatile s32*)&s_thread_queue.task_read_head, 
                                                       next_read_head_position, 
                                                       expected_read_head_pos);
            
            if(work_idx == expected_read_head_pos)
            {
                Work_Queue_Entry* entry = s_thread_queue.tasks + work_idx;
                
                _ReadWriteBarrier();
                Assert_If(entry->work_function)
                {
                    entry->work_function(&entry->work_data, &thread_work_memory);
                    thread_work_memory.clear();
                    
                    OutputDebugStringA("Threaded work completed!\n");
                }
            }
        }
        else
        {
            WaitForSingleObjectEx(s_thread_queue.semaphore_handle, INFINITE, 0);
        }
    }
    
    OutputDebugStringA("Thread exiting!\n");
    Assert(0);
}


static void Init_Sound()
{
    s_sound.samples_per_second = 48000;
    s_sound.post_play_cursor_sample_count = s_sound.samples_per_second / 15;
    s_sound.channels = 2;
    s_sound.bits_per_sample_per_channel = 16;
    s_sound.bytes_per_sample = u16(s_sound.channels * s_sound.bits_per_sample_per_channel / 8);
    s_sound.first_time_output = true;
    
    Assert(s_sound.bytes_per_sample == 4);
    
    u32 buffer_size = s_sound.samples_per_second * s_sound.bytes_per_sample;
    s_sound.client_buffer_size = buffer_size;
    
    u32 commit = MEM_COMMIT | MEM_RESERVE;
    s_sound.client_buffer = (Target_Buffer_Sound_Sample*)VirtualAlloc(0, buffer_size, commit, PAGE_READWRITE);
    
    WModuleHandle dsound_module = LoadLibraryA(DirectSoundModuleName);
    
    if(dsound_module)
    {
        DirectSoundCreate* _DirectSoundCreate = (DirectSoundCreate*)GetProcAddress(dsound_module, DirectSoundCreateProcName);
        
        if(_DirectSoundCreate)
        {
            WCOMObjectDirectSound* com_ds = 0;
            if(SUCCEEDED(_DirectSoundCreate(0, &com_ds, 0)))
            {
                WWaveFormatEx format = {};
                format.format = WAVE_FORMAT_PCM;
                format.channels = s_sound.channels;
                format.samples_per_second = s_sound.samples_per_second;
                format.block_alling = s_sound.bytes_per_sample;
                format.bits_per_sample = s_sound.bits_per_sample_per_channel;
                format.avarage_bytes_per_second = format.samples_per_second * format.block_alling;
                
                
                if(SUCCEEDED(com_ds->dsound->SetCooperativeLevel(com_ds, s_platform_state.window, DSSCL_PRIORITY)))
                {
                    WDirectSoundBufferDescription buffer_description = {};
                    buffer_description.size = sizeof(buffer_description);
                    buffer_description.flags = DSBCAPS_PRIMARYBUFFER;
                    buffer_description.buffer_bytes = 0;
                    
                    WCOMObjectDirectSoundBuffer* com_primary_buffer;
                    if(SUCCEEDED(com_ds->dsound->CreateSoundBuffer(com_ds, &buffer_description, &com_primary_buffer, 0)))
                    {
                        if(!SUCCEEDED(com_primary_buffer->dsound_buffer->SetFormat(com_primary_buffer, &format)))
                        {
                            Assert(0); // Failure point!
                        }
                    }
                    else
                    {
                        Assert(0); // Failure point!
                    }
                }
                else
                {
                    Assert(0); // Failure point!
                }
                
                WDirectSoundBufferDescription buffer_description = {};
                buffer_description.size = sizeof(buffer_description);
                buffer_description.flags = 0;
                buffer_description.buffer_bytes = buffer_size;
                buffer_description.fx_format = &format;
                
                
                if(!SUCCEEDED(com_ds->dsound->CreateSoundBuffer(com_ds, &buffer_description, &s_sound.com, 0)))
                {
                    Assert(0); // Failure point!
                }
            }
            else
            {
                Assert(0); // Failed to initialize the direct sound object.
            }
            
        }
        else
        {
            Assert(0); // Cound not get ptr to DirectSoundCreate from the dll.
        }
    }
    else
    {
        Assert(0); // Failed to load direct sound!
    }
    
    if(s_sound.com && !SUCCEEDED(s_sound.com->dsound_buffer->Play(s_sound.com, 0, 0, DSBPLAY_LOOPING)))
    {
        Assert(0);
    }
}


static void Load_OpenGL_Functions()
{
    // NOTE: To load opengl functions one must have a valid render context, therefore this function first creates a dummy one.
    
    // NOTE: Perhaps the reason why Casey created a dummy window class here is that the pixel format is now associated with the window so, doing this in the actual window will screw things up???
    
    WWindowClass dummy_class = {};
    
    dummy_class.window_procedure = DefWindowProcA;
    dummy_class.instance = GetModuleHandleA(0);
    dummy_class.class_name = "DummyWindowForOpenGL";
    
    RegisterClassA(&dummy_class);
    
    WWindowHandle dummy_window = CreateWindowExA(0, dummy_class.class_name, "OpenGLDummy", 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, dummy_class.instance, 0);
    
    Assert(dummy_window);
    
    WDeviceHandle dc = GetDC(dummy_window);
    
    {
        WPixel_Format_Descriptor desired_pixel_format = {};
        desired_pixel_format.size = sizeof(desired_pixel_format);
        desired_pixel_format.version = 1;
        desired_pixel_format.flags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
        desired_pixel_format.color_bits = 32;
        desired_pixel_format.alpha_bits = 8;
        desired_pixel_format.layer_type = PFD_MAIN_PLANE;
        desired_pixel_format.pixel_type = PFD_TYPE_RGBA;
        desired_pixel_format.depth_bits = 32;
        
        s32 suggested_format_index = ChoosePixelFormat(dc, &desired_pixel_format);
        
        WPixel_Format_Descriptor suggested_pixel_format;
        u32 size = sizeof(suggested_pixel_format);
        DescribePixelFormat(dc, suggested_format_index, size, &suggested_pixel_format);
        b32 success = SetPixelFormat(dc, suggested_format_index, &suggested_pixel_format);
    }
    
    
    WGLRenderContextHandle context = wglCreateContext(dc);
    Assert(context);
    
    b32 success = wglMakeCurrent(dc, context);
    Assert(success);
    
    
    wglChoosePixelFormatARB = (_wglChoosePixelFormatARB*)wglGetProcAddress("wglChoosePixelFormatARB");
    Assert(wglChoosePixelFormatARB);
    
    
    // TODO(Valtteri): Fix the imported signature names to be consistnat with the new format.
    wglCreateContextAttribsARB = (_wglCreateContextAttribsARB*)wglGetProcAddress("wglCreateContextAttribsARB");
    Assert(wglCreateContextAttribsARB);
    
    wglSwapIntervalEXT = (_wglSwapIntervalEXT*)wglGetProcAddress("wglSwapIntervalEXT");
    Assert(wglSwapIntervalEXT);
    
    DestroyWindow(dummy_window);
}

#if COMPILE_OPENGL
static void Choose_Pixel_Format_OpenGL(WDeviceHandle dc)
{
    s32 suggested_pixel_format_idx = 0;
    u32 extension_picked = 0;
    s32 attrib_list[] =
    {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,                 // 0
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB, // 1
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,                 // 2
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,                  // 3
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,           // 4
        WGL_RED_BITS_ARB,   8,                           // 5
        WGL_GREEN_BITS_ARB, 8,                           // 6
        WGL_BLUE_BITS_ARB,  8,                           // 7
        WGL_ALPHA_BITS_ARB, 8,                           // 8
        WGL_DEPTH_BITS_ARB, 24,                          // 9
        0,
    };
    
    Assert(wglChoosePixelFormatARB);
    
    wglChoosePixelFormatARB(dc, attrib_list, 0, 1, &suggested_pixel_format_idx, &extension_picked);
    
    WPixel_Format_Descriptor dummy_format_struct = {};
    b32 success = SetPixelFormat(dc, suggested_pixel_format_idx, &dummy_format_struct);
    
    Assert(success);
}


static bool Init_OpenGL()
{
    WDeviceHandle dc = s_platform_state.device_context;
    
    Load_OpenGL_Functions();
    
    bool success = false;
    
    Choose_Pixel_Format_OpenGL(dc);
    WGLRenderContextHandle opengl_context = wglCreateContext(dc);
    
    Assert_If(opengl_context)
    {
        s_platform_state.opengl_context_version = 1;
        
        Assert_If(wglMakeCurrent(dc, opengl_context))
        {
            
            s32* attribs = (s32*)s_opengl_context_attribs;
            WGLRenderContextHandle modern_context = wglCreateContextAttribsARB(dc, 0, attribs);
            Assert_If(modern_context)
            {
                Assert_If(wglMakeCurrent(dc, modern_context))
                {
                    s_platform_state.opengl_context_version = s_desired_opengl_major_version;
                    wglDeleteContext(opengl_context);
                    
                    success = true;
                    
                    glDepthMask(GL_TRUE);
                    glEnable(GL_BLEND);
                    glEnable(GL_ALPHA_TEST);
                    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                    glAlphaFunc(GL_GEQUAL, 0.1f);
                    glDepthFunc(GL_GEQUAL);
                }
                
            }
        }
    }
    
    
    Assert(success);
    
    return success;
}
#endif

static void Init_Thread_Pool()
{
    static Thread_Info thread_info[4];
    
    s_thread_queue.semaphore_handle = CreateSemaphoreExA(0, 0, Array_Lenght(thread_info), 0, 0, SEMAPHORE_ALL_ACCESS);
    
    for(u32 i = 0; i < Array_Lenght(thread_info); ++i)
    {
        
        Thread_Info* tf = thread_info + i;
        
        tf->idx = i;
        
        u32 thread_id;
        void* param = tf;
        WHandle thread_handle = CreateThread(0, 0, Worker_Thread_Procedure, param, 0, &thread_id);
        CloseHandle(thread_handle);        
    }
}


static inline void Set_Zero_To_CW_DEFAULT(s32* value)
{
    // CONSIDER: Is this a bit overkill for setting a value?
    
    Assert(value);
    
    if(!*value)
    {
        *value = CW_USEDEFAULT;
    }
}


static s64 Display_Window_Proc(WWindowHandle window, u32 message, u64 wparam, s64 lparam)
{
    s64 result = 0;
    
    switch (message)
    {
        // NOTE: For any message to get passed into the main thread they have to be included here!
        case WM_CLOSE:
        case WM_DESTROY:
        case WM_SIZE:
        case WM_CHAR:
        case WM_MOUSEWHEEL:
        case WM_ACTIVATEAPP:
        {
            PostThreadMessageA(s_platform_state.main_thread_id, message, wparam, lparam);
        }break;
        
        default:
        {
            result = DefWindowProcA(window, message, wparam, lparam);
        }break;
    }
    return result;
}


struct Message_Thread_Paramater
{
    char* window_title;
    v2s window_position;
    v2s window_dimensions;
    
    volatile bool init_done;
};


static u32 __stdcall Message_Thread(void* thread_parameter)
{
    Message_Thread_Paramater* mtp = (Message_Thread_Paramater*)thread_parameter;
    
    WWindowClass window_class = {};
    
    char* window_class_name = "DangerousThreadsCrewClass";
    
    window_class.style = CS_OWNDC;
    window_class.window_procedure = &Display_Window_Proc;
    window_class.instance = GetModuleHandleA(0);
    window_class.icon = LoadIconA(window_class.instance, WMakeResource(101));
    window_class.cursor = LoadCursorA(0, WMakeResource(IDC_CROSS));
    window_class.class_name = window_class_name;
    
    RegisterClassA(&window_class);
    
    
    WWindowHandle window = CreateWindowExA(
                                           0,
                                           window_class_name,
                                           mtp->window_title,
                                           WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                           mtp->window_position.x,
                                           mtp->window_position.y,
                                           mtp->window_dimensions.x,
                                           mtp->window_dimensions.y,
                                           0,
                                           0,
                                           window_class.instance,
                                           0);
    
    
    // Calibrate the window position offset.
    {            
        WRect window_rect;
        b32 get_window_rect_result = GetWindowRect(window, &window_rect);
        
        WRect client_rect;
        b32 get_client_rect_result = GetClientRect(window, &client_rect);
        
        s32 _window_width = window_rect.right - window_rect.left;
        s32 _window_height = window_rect.bottom - window_rect.top;
        
        s32 border_width = (_window_width - client_rect.right);
        s32 border_height = (_window_height - client_rect.bottom);
        
        s32 scale = 1;
        
        s32 new_width = _window_width * scale + border_width;
        s32 new_height = _window_height * scale + border_height;
        
        SetWindowPos(window, 0, window_rect.left, window_rect.top, new_width, new_height, 0);
    }
    
    s_platform_state.window = window;
    
    _ReadWriteBarrier();
    mtp->init_done = true;
    
    for(;;)
    {
        WMessage message;
        
        GetMessageA(&message, 0, 0, 0); 
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
    
    return 0;
}


static void Platform_Init(char* window_title, v2s window_position, v2s window_dimensions, bool vsync, bool create_thread_pool)
{
    Set_Zero_To_CW_DEFAULT(&window_position.x);
    Set_Zero_To_CW_DEFAULT(&window_position.y);
    Set_Zero_To_CW_DEFAULT(&window_dimensions.x);
    Set_Zero_To_CW_DEFAULT(&window_dimensions.y);
    
    GetSystemInfo(&s_platform_state.system_info);
    
    {
        WModuleHandle xinput = {};
        for(u64 i = 0; i < Array_Lenght(WModuleMamesXInput[i]) && !xinput; ++i)
        {
            xinput = LoadLibraryA(WModuleMamesXInput[i]);
            if(xinput)
            {
                _XInputGetState = (XInputGetState*)GetProcAddress(xinput, XInputGetStateProcName);
            }
        }
    }
    
    
    s_platform_state.main_thread_id = GetCurrentThreadId();
    
    u32 message_thread_id;
    Message_Thread_Paramater message_thread_parameter = {window_title, window_position, window_dimensions};
    CloseHandle(CreateThread(0, 0, Message_Thread, &message_thread_parameter, 0, &message_thread_id));
    
    while(!message_thread_parameter.init_done);
    
    s_platform_state.flags |= Platform_Flags::focused;
    
    _ReadWriteBarrier();
    
    {
        WRect client_rect;
        if(GetClientRect(s_platform_state.window, &client_rect))
        {
            s32 _window_width = client_rect.right - client_rect.left;
            s32 _window_height = client_rect.bottom - client_rect.top;
            s_platform_state.window_dimensions = {_window_width, _window_height};
        }
    }    
    
    s_platform_state.device_context = GetDC(s_platform_state.window);
    
    Init_Sound();
    
    #if COMPILE_OPENGL
    Init_OpenGL();
    #endif
    
    if(create_thread_pool)
    {
        Init_Thread_Pool();
    }
    
    Set_VSync(vsync);
    
    QueryPerformanceFrequency(&s_platform_state.system_clock_freqeuncy);
    QueryPerformanceCounter(&s_platform_state.last_time_stamp);
}


static void Flush_Message_Queue()
{
    s_platform_state.typing_information = {};
    s_platform_state.scroll_delta = 0;
    
    u64 clear_mask = Platform_Flags::resize | Platform_Flags::wants_to_exit;
    Inverse_Bit_Mask(&s_platform_state.flags, clear_mask);
    
    // This message loop resives messages we choose to send it. Using ThreadMessage and PostMessage.
    WMessage message;
    while(PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
    {
        switch(message.message)
        {
            case WM_SIZE:
            {
                WRect client_rect;
                GetClientRect(s_platform_state.window, &client_rect);
                
                s_platform_state.window_dimensions = 
                {
                    client_rect.right - client_rect.left,
                    client_rect.bottom - client_rect.top
                };
                
#if 0
                if(glRenderbufferStorage)
                {
                    s32 width = s_platform_state.window_dimensions.x;
                    s32 height = s_platform_state.window_dimensions.y;
                    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, width, height);
                }
#endif
                
                s_platform_state.flags |= Platform_Flags::resize;
            }break;
            
            
            case WM_DESTROY:
            case WM_CLOSE:
            {
                s_platform_state.flags |= Platform_Flags::wants_to_exit;
            }break;
            
            
            case WM_CHAR:
            {
                char c = (char)message.wparam;
                
                u32 lenght = Array_Lenght(s_platform_state.typing_information.buffer);
                if(s_platform_state.typing_information.count < lenght)
                {
                    s_platform_state.typing_information.buffer[s_platform_state.typing_information.count] = c;
                    s_platform_state.typing_information.count += 1;
                }
            }break;
            
            
            case WM_MOUSEWHEEL:
            {
                f32 wheel_delta = f32(WGetWheelDeltaWParam(message.wparam));
                s_platform_state.scroll_delta = wheel_delta / f32(WHEEL_DELTA);
            }break;
            
            
            case WM_ACTIVATEAPP:
            {
                Inverse_Bit_Mask(&s_platform_state.flags, Platform_Flags::focused);
                b32 active = (b32)message.wparam;
                if(active)
                {
                    s_platform_state.flags |= Platform_Flags::focused;
                }
            }break;
        }
    }
    
    
    // Update controller state.
    {
        for(s32 i = 0; i < Array_Lenght(s_platform_state.controller_state); ++i)
        {
            s_platform_state.controller_state[i].m_prev = s_platform_state.controller_state[i].m_curr;
            
            Controller_State::Data* controller = &(s_platform_state.controller_state + i)->m_curr;
            
            if(_XInputGetState)
            {
                XInputState xinput_state;
                f64 time_stamp = Platform_Get_Time_Stamp();
                if(time_stamp >= s_platform_state.next_controler_test_time)
                {
                    u32 result = _XInputGetState(i, &xinput_state);
                    if(result == ERROR_SUCCESS)
                    {
                        XInputGamepad pad = xinput_state.gamepad;
                        controller->button_states = 0;
                        
                        for(u16 button_idx = 0; button_idx < (u16)Button::BUTTON_COUNT; ++button_idx)
                        {
                            if(s_controller_map[button_idx] & pad.buttons)
                            {
                                controller->button_states |= (1 << button_idx);
                            }
                        }
                        
                        constexpr u32 negative_max_range = 32768;
                        constexpr u32 positive_max_range = 32767;
                        
                        if(pad.thumb_left_x < 0)
                        {
                            controller->l_thumb_x = (f32)pad.thumb_left_x / negative_max_range;
                        }
                        else
                        {
                            controller->l_thumb_x = (f32)pad.thumb_left_x / positive_max_range;
                        }
                        
                        if(pad.thumb_left_y < 0)
                        {
                            controller->l_thumb_y = (f32)pad.thumb_left_y / negative_max_range;
                        }
                        else
                        {
                            controller->l_thumb_y = (f32)pad.thumb_left_y / positive_max_range;
                        }
                        
                        
                        if(pad.thumb_right_x < 0)
                        {
                            controller->r_thumb_x = (f32)pad.thumb_right_x / negative_max_range;
                        }
                        else
                        {
                            controller->r_thumb_x = (f32)pad.thumb_right_x / positive_max_range;
                        }
                        
                        if(pad.thumb_right_y < 0)
                        {
                            controller->r_thumb_y = (f32)pad.thumb_right_y / negative_max_range;
                        }
                        else
                        {
                            controller->r_thumb_y = (f32)pad.thumb_right_y / positive_max_range;
                        }
                        
                        constexpr u32 trigger_max_range = 255;
                        
                        controller->l_trig = (f32)pad.left_trigger / trigger_max_range;
                        controller->r_trig = (f32)pad.right_trigger / trigger_max_range;
                    }
                    else if(result == ERROR_DEVICE_NOT_CONNECTED)
                    {
                        s_platform_state.controller_state[i] = {0};
                        s_platform_state.next_controler_test_time = time_stamp + 3;
                    }
                }
            }
        }
    }
}


static void* Platform_Allocate_Memory(u32 amount, u32* out_amount)
{
    u32 page = u32(s_platform_state.system_info.page_size);
    u32 p_count = amount / page;
    if(p_count * page < amount)
    {
        p_count += 1;
        u32 new_amount = p_count * page;
        Assert(new_amount >= amount);
        amount = new_amount;
        
        if(out_amount)
        {
            *out_amount = amount;
        }
    }
    
    void* memory = VirtualAlloc(0, amount, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    return memory;
}


static void Platform_Free_Memory(void* memory)
{
    b32 success = VirtualFree(memory, 0, MEM_RELEASE);
    Assert(success);
}


static void Platform_Queue_Threaded_Work(Platform_Work_Function work_function, Work_Data* work_data)
{
    u32 max_tasks = Array_Lenght(s_thread_queue.tasks);
    
    u32 write_head_pos = s_thread_queue.task_write_head;
    u32 next_write_head_pos = (write_head_pos + 1) % max_tasks;
    
    Assert(next_write_head_pos != s_thread_queue.task_read_head);
    
    _ReadWriteBarrier();
    
    // Check that the write head has not moved since we read it.
    if(s_thread_queue.task_write_head == write_head_pos)
    {
        // Write in the work!
        *(s_thread_queue.tasks + write_head_pos) = {work_function, *work_data};
        
        // Increase the write head if no thread still hasn't touched it.
        // If this exhance succeeds, a thread can take the work item and,
        // start executing it NOW!
        u32 assumed_write_head_pos = _InterlockedCompareExchange(
                                                                 (volatile s32*)&s_thread_queue.task_write_head,
                                                                 next_write_head_pos,
                                                                 write_head_pos);
        
        
        Assert(assumed_write_head_pos == write_head_pos);
        
        ReleaseSemaphore(s_thread_queue.semaphore_handle, 1, 0);   
    }
}


static v2s Platform_Get_Cursor_Position()
{
    // TODO: cashe the border values.
    
    WPoint cursor_p;
    b32 get_cursor_pos_result = GetCursorPos(&cursor_p);
    
    WRect window_rect;
    b32 get_window_rect_result = GetWindowRect(s_platform_state.window, &window_rect);
    
    WRect client_rect;
    b32 get_client_rect_result = GetClientRect(s_platform_state.window, &client_rect);
    
    s32 window_width = window_rect.right - window_rect.left;
    s32 window_height = window_rect.bottom - window_rect.top;
    
    s32 border_width = (window_width - client_rect.right) / 2;
    s32 border_height = (window_height - client_rect.bottom) - border_width;
    
    // x and y are now relative to the top left of the windows client area.
    s32 x = cursor_p.x - (window_rect.left + border_width);
    s32 y = cursor_p.y - (window_rect.top + border_height);
    
    // Next they are converted to be relative to the bottom left. Increasing up and rightwards.
    y = (y - client_rect.bottom) * -1;
    
    // Next convert the screenspace coordinates into pixel space coordinates.
    x = s32(x * ((f32)s_platform_state.window_dimensions.x / (f32)client_rect.right));
    y = s32(y * ((f32)s_platform_state.window_dimensions.y / (f32)client_rect.bottom));
    
    return v2s{x, y};
}


static bool Platform_Get_Keyboard_Key_Down(Key_Code key_code)
{
    bool result = false;
    
    if(key_code != Key_Code::NONE)
    {
        result = (GetKeyState(s_windows_keycode_map[(u32)key_code]) & (1 << 15)) > 0;
    }
    
    return result;
}


static Char_Array Platform_Get_Typing_Information()
{
    return s_platform_state.typing_information;
}


static f64 Platform_Get_Time_Stamp()
{
    // Memory is memory, so fuck your LARGE_INTEGERS!
    
    u64 time_stamp;
    QueryPerformanceCounter(&time_stamp);
    return f64(time_stamp) / s_platform_state.system_clock_freqeuncy;
}


static f64 Platform_Get_Frame_Time()
{
    return s_platform_state.frame_time;
}


static f32 Platform_Get_Scroll_Wheel_Delta()
{
    return s_platform_state.scroll_delta;
}


static u64 Platform_Get_Frame_Count()
{
    return s_platform_state.frame_count;
}


static s32 Platform_Get_OpenGL_Version()
{
    return s_platform_state.opengl_context_version;
}


static v2s Platform_Get_Window_Dimensions()
{
    return s_platform_state.window_dimensions;
}


static u32 Platform_Get_Sound_Samples_Per_Second()
{
    return s_sound.samples_per_second;
}


static Controller_State Platform_Get_Controller_State(u32 controller)
{
    Assert(controller < Array_Lenght(s_platform_state.controller_state));
    return s_platform_state.controller_state[controller]; 
}


static bool Open_File_Handle(char* path, u32 access, u32 creation_dispotion, WHandle* out_handle)
{
    Assert(path);
    Assert(out_handle);
    
    WHandle handle = CreateFileA(path, access, 0, 0, creation_dispotion, FILE_ATTRIBUTE_NORMAL, 0);
    bool result = handle != INVALID_HANDLE_VALUE;
    *out_handle = handle;
    
    return result;
}


static bool Platform_Get_File_Size(char* path, u32* out_size)
{
    Assert(out_size);
    
    bool result = false;
    
    WHandle file_handle;
    if(Open_File_Handle(path, GENERIC_READ, OPEN_EXISTING, &file_handle))
    {
        *out_size = GetFileSize(file_handle, 0);
        CloseHandle(file_handle);
        result = true;
    }
    
    return result;
}


static bool Platform_Read_File(char* path, u8* buffer, u32 buffer_size)
{
    bool file_read = 0;
    
    WHandle file_handle;
    if(Open_File_Handle(path, GENERIC_READ, OPEN_EXISTING, &file_handle))
    {
        u32 bytes_read;
        file_read = ReadFile(file_handle, buffer, buffer_size, &bytes_read, 0);
        
        Assert(bytes_read == buffer_size);
        CloseHandle(file_handle);
    }
    
    return file_read;
}


static bool Platform_Write_File(char* path, char* buffer, u32 buffer_size)
{
    b32 result = false;
    
    WHandle file_handle;
    if(Open_File_Handle(path, GENERIC_WRITE, CREATE_ALWAYS, &file_handle))
    {
        u32 bytes_written;
        result = WriteFile(file_handle, buffer, buffer_size, &bytes_written, 0);
        Assert(bytes_written == buffer_size);
        CloseHandle(file_handle);
    }
    
    
    return result;
    
}


static Color* Platform_Resize_Software_Render_Target_Pixel_Buffer(v2s dim)
{
    constexpr u32 SSE_padding = 3; // For SSE wide operations.
    
    Assert(dim.x >= 0);
    Assert(dim.y >= 0);
    
    s_software_render_target.width = dim.x;
    s_software_render_target.height = dim.y;
    
    u32 pixel_count = dim.x * dim.y;
    u32 mem_size = (pixel_count + SSE_padding) * sizeof(*s_software_render_target.buffer);
    if(s_software_render_target.allocated_capacity < mem_size)
    {
        if(s_software_render_target.buffer)
        {
            VirtualFree(s_software_render_target.buffer, 0, MEM_RELEASE);
        }
        
        s_software_render_target.buffer = (Color*)Platform_Allocate_Memory(mem_size, &mem_size);
        s_software_render_target.allocated_capacity = mem_size;
    }
    
    return s_software_render_target.buffer;
}


static void Software_Renderer_SwapBuffers()
{
    Assert(s_software_render_target.buffer);
    
    WBitMapInfoHeader header = {};
    header.size = sizeof(header);
    header.width  = s_software_render_target.width;
    header.height = s_software_render_target.height;
    header.planes = 1;
    header.bit_count = sizeof(Color) * 8;
    
    WBitmapInfo info = {header};
    
    StretchDIBits(
                  s_platform_state.device_context,
                  0, 
                  0,
                  s_platform_state.window_dimensions.x,
                  s_platform_state.window_dimensions.y,
                  0, 
                  0,
                  s_software_render_target.width,
                  s_software_render_target.height,
                  s_software_render_target.buffer,
                  &info,
                  0,
                  SRCCOPY
                  );
}


static void Platform_Set_Clipboard_Data_As_Text(char* buffer, u32 size)
{
    // TODO: Check documentation to see if the globaly allocated memory should be freed!
    
    if(OpenClipboard(0)) 
    {
        WGlobalHandle handle = GlobalAlloc(GMEM_MOVEABLE, size + 1);
        char* dest = (char*)GlobalLock(handle);
        dest[size] = 0;
        Mem_Copy(dest, buffer, size);
        
        GlobalUnlock(handle);
        
        EmptyClipboard();
        
        SetClipboardData(CF_TEXT, handle);
        
        CloseClipboard();
    }
}


static char* Platform_Get_Clipboard_Data_As_Text()
{
    // TODO: Check documentation to see if the handle needs to be closed!
    
    char* result = 0;
    
    if(OpenClipboard(0))
    {
        WHandle h = GetClipboardData(CF_TEXT);
        result = (char*)h;
        
        CloseClipboard();        
    }
    
    return result;
}


static void Platform_Set_VSync(bool enabled)
{
    if(Is_Bit_Set(s_platform_state.flags, Platform_Flags::vsync) != enabled)
    {
        Set_VSync(s32(enabled));
    }
}


static void Platform_Set_Fullscreen(bool enabled)
{
    if(((s_platform_state.flags & Platform_Flags::fullscreen) > 0) != enabled)
    {
        u32 dwStyle = GetWindowLongA(s_platform_state.window, GWL_STYLE);
        
        if(enabled)
        {
            if(GetWindowPlacement(s_platform_state.window, &s_platform_state.window_placement))
            {
                WMonitorInfo monitor_info = {sizeof(monitor_info)};
                
                if(GetMonitorInfoA(MonitorFromWindow(s_platform_state.window, MONITOR_DEFAULTTOPRIMARY), &monitor_info))
                {
                    s_platform_state.flags |= Platform_Flags::fullscreen;
                    
                    SetWindowLongA(s_platform_state.window, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
                    SetWindowPos
                    (
                     s_platform_state.window,
                     HWND_TOP,
                     monitor_info.monitor.left,
                     monitor_info.monitor.top,
                     monitor_info.monitor.right - monitor_info.monitor.left,
                     monitor_info.monitor.bottom - monitor_info.monitor.top,
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED
                     );
                }
            }
        }
        else
        {
            Inverse_Bit_Mask(&s_platform_state.flags, Platform_Flags::fullscreen);
            
            SetWindowLongA(s_platform_state.window, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
            SetWindowPlacement(s_platform_state.window, &s_platform_state.window_placement);
            SetWindowPos(s_platform_state.window, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
}


static void Platform_Output_Sound()
{
    s_sound.accumulated_frame_time += s_platform_state.frame_time;
    f64 sound_frame_time = 1.0 / 30.0;
    
    if(s_sound.com)
    {
        while(s_sound.accumulated_frame_time > sound_frame_time)
        {
            s_sound.accumulated_frame_time -= sound_frame_time;
            u32 play_cursor;
            u32 write_cursor;
            if(SUCCEEDED(s_sound.com->dsound_buffer->GetCurrentPosition(s_sound.com, &play_cursor, &write_cursor)))
            {
                // On startup we have to find out where in the sound ring buffer we have to start writing to.
                // This snaps our sample write counter to the write_cursor.
                if(!s_sound.sample_write_counter)
                {
                    s_sound.sample_write_counter = write_cursor / s_sound.bytes_per_sample;
                    return;
                }
                
                u32 bytes_per_frame = u32(sound_frame_time * f64(s_sound.samples_per_second)) * s_sound.bytes_per_sample;
                Assert(bytes_per_frame);
                
                u32 buffer_size = s_sound.client_buffer_size;
                
                u32 safety_bytes = 1024 * 2;
                
                // To not have skips in the audio, we always continue writing the buffer where we last left off.
                u32 byte_to_lock = (s_sound.sample_write_counter * s_sound.bytes_per_sample) % buffer_size;
                
                // Where the play cursor is expected to be on the next time through.
                u32 expected_frame_boundary_byte = play_cursor + bytes_per_frame;
                
                // Unwrap the ring-buffer. uw_write cursor will in 0 - ( 2 x buffer_size - 1).
                u32 unwrapped_write_cursor = write_cursor;
                if(unwrapped_write_cursor < play_cursor)
                {
                    unwrapped_write_cursor += buffer_size;
                }
                Assert(unwrapped_write_cursor >= play_cursor);
                unwrapped_write_cursor += safety_bytes;
                
                bool audio_is_low_latency = unwrapped_write_cursor < expected_frame_boundary_byte;
                
                u32 target_cursor;
                if(audio_is_low_latency)
                {
                    // Only write up to the next "audio" frame boundary.
                    target_cursor = expected_frame_boundary_byte;
                }
                else
                {
                    target_cursor = write_cursor + bytes_per_frame;
                }
                target_cursor = (target_cursor + safety_bytes) % buffer_size;
                
                
                u32 bytes_to_write = 0;
                if(byte_to_lock > target_cursor)
                {
                    bytes_to_write = buffer_size - byte_to_lock + target_cursor;
                }
                else if(byte_to_lock < target_cursor)
                {
                    bytes_to_write = target_cursor - byte_to_lock;
                }
                
                if(bytes_to_write)
                {
                    u32 samples_to_write = bytes_to_write / s_sound.bytes_per_sample;
                    Output_Sound(s_sound.client_buffer, samples_to_write, s_sound.samples_per_second);
                    
                    void* regions[2];
                    u32 regions_size[1];
                    
                    // [LEFT RIGHT] [LEFT RIGHT]
                    // [2Bytes 2Bytes]=4 [2Bytes 2Bytes]=4
                    
                    if(SUCCEEDED(s_sound.com->dsound_buffer->Lock(
                                                                  s_sound.com,
                                                                  byte_to_lock, 
                                                                  bytes_to_write, 
                                                                  &regions[0], 
                                                                  &regions_size[0], 
                                                                  &regions[1], 
                                                                  &regions_size[1], 
                                                                  0)))
                    {
                        u32 write_idx = 0;
                        for(u32 r = 0; r < Array_Lenght(regions); ++r)
                        {
                            s16* sample_out = (s16*)regions[r];
                            for(u32 i = 0; i < regions_size[r] / s_sound.bytes_per_sample; ++i)
                            {
                                Target_Buffer_Sound_Sample* sample = s_sound.client_buffer + write_idx++;
                                *sample_out++ = sample->left;
                                *sample_out++ = sample->right;
                                s_sound.sample_write_counter++;
                            }
                        }
                        
                        if(!SUCCEEDED(s_sound.com->dsound_buffer->Unlock(s_sound.com, regions[0], regions_size[0], regions[1], regions_size[1])))
                        {
                            Assert(0);
                        }
                    }
                }
            }
        }
    }
}


static void Update_Frame_Time()
{
    u64 time_stamp;
    QueryPerformanceCounter(&time_stamp);
    
    s64 counter_elapsed_time = time_stamp - s_platform_state.last_time_stamp;
    
    s_platform_state.last_time_stamp = time_stamp;
    
    f64 frame_time = f64(counter_elapsed_time) / s_platform_state.system_clock_freqeuncy;
    s_platform_state.frame_time = frame_time;
}


static void Platform_Update(Platform_Swap_Buffer_Type swap_type)
{
    Platform_Output_Sound();
    
    switch(swap_type)
    {
        case Platform_Swap_Buffer_Type::software:
        {
            Software_Renderer_SwapBuffers();
        }break;
        
        case Platform_Swap_Buffer_Type::device:
        {
            SwapBuffers(s_platform_state.device_context);
        }break;
    }
    
    Flush_Message_Queue();
    
    Update_Frame_Time();
    
    s_platform_state.frame_count += 1;
}