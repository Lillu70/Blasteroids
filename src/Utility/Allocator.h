
#pragma once

#include "Primitives.h"
#include "Assert.h"

#define Exhaustive_Memory_Verification
#undef Exhaustive_Memory_Verification

#ifdef Exhaustive_Memory_Verification

#define EMV_Call(X) X

#else

#define EMV_Call(X)

#endif


struct Free_Block
{
	Free_Block* prew_block = 0;
	Free_Block* next_block = 0;
	u32 size = 0;
};

struct General_Allocator
{
	static u32 constexpr min_alloc_size = sizeof(Free_Block);
	
	Free_Block* first_block;
	Free_Block* last_block;
	u8* memory = 0;
	u32 full_capacity = 0;
	u32 push_call_count = 0;
	u32 free_call_count = 0;


	void init(void* _memory, u32 _capacity)
	{
		memory = (u8*)_memory;
		Free_Block* block = (Free_Block*)memory;
		*block = { 0,0, _capacity };
		first_block = block;
		last_block = block;
		
		full_capacity = _capacity;
	}

    
	void clear()
	{
		Free_Block* block = (Free_Block*)memory;
		*block = { 0,0, full_capacity };
		first_block = block;
		last_block = block;
		push_call_count = 0;
		free_call_count = 0;
	}
	
	
	u32 count_free_blocks()
	{
		u32 result = 0;
		
		Free_Block* block = first_block;
		while(block)
		{
			result += 1;
			block = block->next_block;
		}
		
		return result;
	}
    
	
	bool address_inside_region(u8* ptr)
	{
		return ptr >= memory && ptr < memory + full_capacity;
	}
	
	void test_free_block_start_to_end_ordering()
	{
		
		Free_Block* block = first_block;
		
		while(block)
		{
			Assert(!block->next_block || block->next_block > block);
			Assert((block == first_block && block == last_block) || block->prew_block != block->next_block);
			Assert(block != block->prew_block && block != block->next_block);
			
			Assert(!block->next_block || (u8*)(block->next_block) >= (u8*)block + block->size);
			
			block = block->next_block;
		}
	
	}
	
	void test_free_block_end_to_start_ordering()
	{
		Free_Block* block = last_block;
		
		while(block)
		{
			Assert(!block->prew_block || block->prew_block < block);
			Assert((block == first_block && block == last_block) ||block->prew_block != block->next_block);
			Assert(block != block->prew_block && block != block->next_block);
			block = block->prew_block;
		}
	
	}
	
	void test_free_block_allignement()
	{
		test_free_block_start_to_end_ordering();
		test_free_block_end_to_start_ordering();
		
		Free_Block* a_block = first_block;
		u32 a_count = 0;
		
		while(a_block)
		{
			Free_Block* b_block = first_block;
			
			while(b_block)
			{
				if(a_block != b_block)
				{
					if((u8*)b_block == (u8*)a_block + a_block->size)
					{
						Terminate;
					}
					
					if((u8*)b_block + b_block->size == (u8*)a_block)
					{
						Terminate;
					}
				}
				
				b_block = b_block->next_block;
			}
			
			a_count += 1;
			a_block = a_block->next_block;
		}
	}
	
	
	void* push(u32 size)
	{
		EMV_Call(test_free_block_allignement();)
		
		push_call_count += 1;
		
		size += sizeof(size);
		if (size < min_alloc_size)
			size = min_alloc_size;
		
		u32 best_diff = 0;
		Free_Block* block = 0;
		
		Free_Block* fb = first_block;
		
		while (fb)
		{

			//TODO: If with only asserts inside? Change this whole block into a DBCALL.
			if(fb != first_block)
			{
				Assert(fb->prew_block);
				Assert(address_inside_region((u8*)fb->prew_block));
			}
			
			
			// Exact match. Push in here!
			if (fb->size == size)
			{
				remove_fb_from_chain(fb);
				
				u32* result = (u32*)fb;
				*result = size;
				
				EMV_Call(test_free_block_allignement();)
				
				return (void*)(result + 1);
			}
			
			// Can fit the alloc in here, but it's not an exact match.
			else if (fb->size > size)
			{
				u32 diff = fb->size - size;
				if (diff > best_diff)
				{
					best_diff = diff;
					block = fb;
				}
			}
			
			fb = fb->next_block;
		}
		
		if (block)
		{
			// This is the biggest free block and it can't fit a free header in it.
			if (block->size < size + sizeof(Free_Block))
			{
				EMV_Call(test_free_block_allignement();)
				
				// So we allocate all of this, sorry; you get more than what you asked for.
				// but, now we have to get rid of this block.
				remove_fb_from_chain(block);
				size = block->size;
				
				EMV_Call(test_free_block_allignement();)
			}
		
			// Shave the fb, and then move the fb definition.    
			else
			{
				Assert(block != block->prew_block && block != block->next_block);
				EMV_Call(test_free_block_allignement();)
				
				Free_Block block_copy = *block;
				
				// This has similar concerns to killing a block;
				// next and previous have to be informed about where the fb now resides.
				Free_Block* new_pos = (Free_Block*)((u8*)block + size);
				Assert(new_pos != block->next_block);
				Assert(new_pos != block->prew_block);
				Assert(new_pos != block);
				
				Assert(block != block->prew_block && block != block->next_block);
				
				*new_pos = { block->prew_block, block->next_block, block->size - size };
				
				Assert(new_pos != new_pos->prew_block && new_pos != new_pos->next_block);
				
				relink_free_block(new_pos);
				Assert(new_pos != new_pos->prew_block && new_pos != new_pos->next_block);
				
				EMV_Call
				(
					test_free_block_allignement();
					int a = 0;
				)
			}
			
			u32* result = (u32*)block;
			*result = size;
			
			EMV_Call(test_free_block_allignement();)
			
			return (void*)(result + 1);
		
		}
		
		
		Terminate;
		return 0;
	}

	template<typename T>
	T* push()
	{
		return (T*)push(sizeof(T));
	}

	void free(void* ptr)
	{
		EMV_Call(test_free_block_allignement();)
		
		free_call_count += 1;
		
		u8* block_start = (u8*)ptr - sizeof(Free_Block::size);
		u32 block_size = *((u32*)block_start);
		
		Assert(block_start >= memory && block_start < memory + full_capacity);
		
		// We need to find out where on the memory line this block is.
		
		if (first_block)
		{
		
			// Seek for the last free block that is behind this address. 
			Free_Block* fb = 0;
			Assert(block_start != (u8*)first_block);
			
			
			if (block_start > (u8*)first_block)
			{
				fb = first_block;
				while (fb->next_block && (u8*)fb->next_block < block_start)
					fb = fb->next_block;
				
				if(!fb)
					fb = last_block;
				
				
				Assert(fb >= first_block);
				Assert(block_start >= (u8*)(first_block + 1));
				Assert(!fb->next_block || (u8*)block_start + block_size <= (u8*)fb->next_block);
				
				// some block was found to be before this point on the memory line.
				
				// that block ends where this one starts.
				if ((u8*)fb + fb->size == block_start)
				{
					// expand fp to include this memory area.
					fb->size += block_size;
					
					// This block is in between 2 free blocks.
					if ((u8*)fb->next_block == block_start + block_size)
					{
						// Again grow left block, now by next free block size.
						fb->size += fb->next_block->size;
						
						// this kills a free block.
						// TODO: this func is overkill. In this case we know we're not the first block.
						remove_fb_from_chain(fb->next_block);
						
						EMV_Call
						(
							test_free_block_allignement();
							int a = 0;
						)
					}
					
					EMV_Call
					(
						test_free_block_allignement();
						int a = 0;
					)
				}
				
				// Some block is right after this one. Merge that backwards.
				else if ((u8*)fb->next_block == block_start + block_size)
				{
					// ~ Moving the next block backwards.
					
					EMV_Call(test_free_block_allignement();)
					
					u8* fb_copy = (u8*)(first_block);
					u8* lb_copy = (u8*)(last_block);
					
					Free_Block* new_pos = (Free_Block*)block_start;
					*new_pos = { fb->next_block->prew_block, fb->next_block->next_block, fb->next_block->size + block_size };
					relink_free_block(new_pos);
					
					EMV_Call
					(
						test_free_block_allignement();
						int a = 0;
					)
				}
				
				// We have a previous block, but we don't allign with it or the it's next block.
				// We have to create a new free block!
				else
				{
					EMV_Call(test_free_block_allignement();)
					Free_Block* new_block = (Free_Block*)block_start;
					
					Assert(fb < new_block);
					Assert(!fb->next_block || (u8*)new_block < (u8*)fb->next_block);
					Assert(!fb->next_block || (u8*)new_block + block_size < (u8*)fb->next_block);
					
					*new_block = { fb, fb->next_block, block_size };
					new_block->prew_block->next_block = new_block;
					
					if (new_block->next_block)
						new_block->next_block->prew_block = new_block;
					else
						last_block = new_block;
					
					EMV_Call
					(
						test_free_block_allignement();
						int a = 0;
					)
				}
			
			}
			else
			{
				Assert(block_start < (u8*)first_block);
				
				// First block is right after this one. Merge that backwards.
				if ((u8*)first_block == block_start + block_size)
				{
					Free_Block* new_pos = (Free_Block*)block_start;
					*new_pos = { 0, first_block->next_block, first_block->size + block_size };
					first_block = new_pos;
					
					if (first_block->next_block)
						first_block->next_block->prew_block = first_block;
					else
						last_block = first_block;
					
					EMV_Call(test_free_block_allignement();)
				}
				
				// We are before the first block and we don't allgin with it.
				// Create a new block here and link with prev first.
				else
				{
					EMV_Call(test_free_block_allignement();)
					
					Free_Block* new_block = (Free_Block*)block_start;
					
					*new_block = { 0, first_block, block_size };
					first_block->prew_block = new_block;
					first_block = new_block;
					
					EMV_Call
					(
						test_free_block_allignement();
						int a = 0;
					)
				}
			}
		}
		
		// We don't have a first block.
		// Make this the first and the last block.
		else
		{
			Free_Block* new_block = (Free_Block*)block_start;
			*new_block = { 0, 0, block_size };
			first_block = new_block;
			last_block = new_block;
			
			EMV_Call(test_free_block_allignement();)
			
		}
		
		
		EMV_Call
		(
			test_free_block_allignement();
			int a = 0;
		)
	}

private:
	inline void remove_fb_from_chain(Free_Block* fb)
	{
		if (fb->prew_block && fb->next_block)
		{
		fb->prew_block->next_block = fb->next_block;
		fb->next_block->prew_block = fb->prew_block;
		}
		
		else
		{
			// this is a case where we can be;
			// - the only block. Clear first and last block to 0.
			// When freeing create a free block at the free site,
			// and link up the first and last in there.
			
			// - the first block. (but not the last one)
			// Shift up the first block into the next block.
			
			// - the the last block. (but not the first one)
			// shift last block down into the previous block.
			
			bool is_first = (fb == first_block);
			bool is_last = (fb == last_block);
			
			if (is_first && is_last)
			{
				first_block = 0;
				last_block = 0;
			}
			
			else if (is_first)
			{
				first_block = fb->next_block;
				first_block->prew_block = 0;
			}
			
			else // is last assumed.
			{
				last_block = fb->prew_block;
				last_block->next_block = 0;
			}
		}
	
	}
	
	inline void relink_free_block(Free_Block* fb)
	{
		if (fb->prew_block)
			fb->prew_block->next_block = fb;
		else
			first_block = fb;
		
		if (fb->next_block)
			fb->next_block->prew_block = fb;
		else
			last_block = fb;
	}
};


struct Linear_Allocator
{
	u8* memory = 0;
	u8* next_free = 0;
	u32 capacity = 0;
	
	void init(void* _memory, u32 _capacity)
	{
		memory = (u8*)_memory;
		next_free = memory;
		capacity = _capacity;
	}
	
	void clear()
	{
		next_free = memory;
	}
	
	void* push(u32 size)
	{
		Assert(capacity - (next_free - memory) >= size);
		u8* result = next_free;
		next_free += size;
		return result;
	}
	
	template<typename T>
	T* push()
	{
		return (T*)push(sizeof(T));
	}
	
	u32 inline get_free_capacity()
	{
		u32 result = u32(capacity - (next_free - memory));
		return result;
	}
};
