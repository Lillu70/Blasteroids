
#pragma once

enum class Entity_Type : u32
{
	none = 0,
	player_ship,
	enemy_ship,
	asteroid,
	bullet,
	pickup,
};


enum class Entity_Flags : u8
{
	alive = 0,
	immune_to_time_stop,
};


enum class Size : u8
{
	small = 0,
	medium,
	large,
};


struct Entity
{
	Entity_Type type = Entity_Type::none;
	u32 id = 0;
	u32 flags = 0;
	u32 color = color_to_u32(WHITE);
	
	v2f position = { 0, 0 };
	v2f velocity = { 0, 0 };
	
	static constexpr u32 max_data = 32;
	u8 data[max_data] = {0};
	
	template<typename T>
	T* alloc_external(General_Allocator* mem)
	{
		u64 size = sizeof(T);
		Assert(size > max_data);
		
		void* ptr = &data[0]; 
		*((T**)ptr) = mem->push<T>();
		T* external = *((T**)ptr);
		*external = T();
		
		return external;
	}
	
	
	template<typename T>
	T* retrive_external()
	{
		T* result = *((T**)&data[0]);
		return result;
	}
	
	
	template<typename T>
	T* alloc_internal()
	{
		u64 size = sizeof(T);
		Assert(size <= max_data);
		
		T* internal = (T*)(&data[0]);
		*internal = T();
		
		return internal;
	}
	
	
	template<typename T>
	T* retrive_internal()
	{
		T* result = (T*)&data[0];
		return result;
	}

};


static void set_entity_flag(Entity* entity, Entity_Flags flag, bool value)
{
	Assert(entity);
	
	//https://stackoverflow.com/questions/47981/how-to-set-clear-and-toggle-a-single-bit
	
	u8 n = (u8)flag;
	entity->flags = (entity->flags & ~(1 << n)) | (value << n);
}


static bool get_entity_flag(Entity* entity, Entity_Flags flag)
{
	Assert(entity);
	
	u8 n = (u8)flag;
	return (entity->flags >> n) & 1;
}


static void set_inheritet_entity_flags(Entity* entity, Entity* parent)
{
	Entity_Flags flag;
	flag = Entity_Flags::immune_to_time_stop;
	set_entity_flag(entity, flag, get_entity_flag(parent, flag));
}
