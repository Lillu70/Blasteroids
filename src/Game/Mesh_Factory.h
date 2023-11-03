
#pragma once

static void generate_mesh_data(Linear_Allocator* alloc, Transient_Data* transient)
{
	// Pickup mesh
	{
		
	    transient->pickup_mesh.p_count = 4;
	    transient->pickup_mesh.data = (v2f*)alloc->push(sizeof(v2f) * transient->pickup_mesh.p_count);
	}
	
	// Ship mesh
	{
	    transient->ship_mesh.p_count = 4;
	    transient->ship_mesh.data = (v2f*)alloc->push(sizeof(v2f) * transient->ship_mesh.p_count);
	    transient->ship_mesh.data[0] = {-10, -10};
	    transient->ship_mesh.data[1] = {0, 15};
	    transient->ship_mesh.data[2] = {10, -10};
	    transient->ship_mesh.data[3] = {0, -5};
		transient->ship_mesh_width = find_mesh_p_furthest_distance_from_origin(transient->ship_mesh);
	}

	// enemy mesh
	{
	    u32 p;
	    v2f* mesh = 0;
	    u32 mesh_idx;

	    mesh_idx = 0;
	    transient->enemy_meshes[mesh_idx].p_count = 7;
	    transient->enemy_meshes[mesh_idx].data = (v2f*)alloc->push(sizeof(v2f) * transient->enemy_meshes[mesh_idx].p_count); 
	    mesh = transient->enemy_meshes[mesh_idx].data;

	    p = 0;
	    mesh[p++] = {0, 7};
	    mesh[p++] = {20, 0};
	    mesh[p++] = {3, -5};
	    mesh[p++] = {6, -10};
	    mesh[p++] = {-6, -10};  
	    mesh[p++] = {-3, -5};
	    mesh[p++] = {-20, 0};
		
		transient->enemy_mesh_widths[mesh_idx] = find_mesh_p_furthest_distance_from_origin(transient->enemy_meshes[mesh_idx]);


	    Assert(p == transient->enemy_meshes[mesh_idx].p_count);
	    mesh_idx += 1;
		
	    transient->enemy_meshes[mesh_idx].p_count = 12;
	    transient->enemy_meshes[mesh_idx].data = (v2f*)alloc->push(sizeof(v2f) * transient->enemy_meshes[mesh_idx].p_count); 
	    mesh = transient->enemy_meshes[mesh_idx].data;

	    p = 0;
	    mesh[p++] = v2f{5, 5} * 1.5f;
	    mesh[p++] = v2f{7, 7} * 1.5f;
	    mesh[p++] = v2f{10, 5} * 1.5f;
	    mesh[p++] = v2f{10, -10} * 1.5f;
	    mesh[p++] = v2f{5, -10} * 1.5f;
	    mesh[p++] = v2f{5, -5} * 1.5f;
	    mesh[p++] = v2f{-5, -5} * 1.5f;
	    mesh[p++] = v2f{-5, -10} * 1.5f;
	    mesh[p++] = v2f{-10, -10} * 1.5f;
	    mesh[p++] = v2f{-10, 5} * 1.5f;
	    mesh[p++] = v2f{-7, 7} * 1.5f;
	    mesh[p++] = v2f{-5, 5} * 1.5f;
		
		transient->enemy_mesh_widths[mesh_idx] = find_mesh_p_furthest_distance_from_origin(transient->enemy_meshes[mesh_idx]);
		
		
	    Assert(p == transient->enemy_meshes[mesh_idx].p_count);
	    mesh_idx += 1;

	    transient->enemy_meshes[mesh_idx].p_count = 9;
	    transient->enemy_meshes[mesh_idx].data = (v2f*)alloc->push(sizeof(v2f) * transient->enemy_meshes[mesh_idx].p_count); 
	    mesh = transient->enemy_meshes[mesh_idx].data;
		
	    p = 0;
	    mesh[p++] = {0, 25};
	    mesh[p++] = {5, 12};
	    mesh[p++] = {5, 2};
	    mesh[p++] = {12, -7};
	    mesh[p++] = {12, -13};
	    mesh[p++] = {-12, -13};
	    mesh[p++] = {-12, -7};
	    mesh[p++] = {-5, 2};
	    mesh[p++] = {-5, 12};
		
		transient->enemy_mesh_widths[mesh_idx] = find_mesh_p_furthest_distance_from_origin(transient->enemy_meshes[mesh_idx]);
	    
		
		Assert(p == transient->enemy_meshes[mesh_idx].p_count);
	    mesh_idx += 1;
	}	
}