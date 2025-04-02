#pragma once

#include "cgltf.h"

#include "../animations/pose.h"
#include "../graphics/mesh.h"
#include "../graphics/material.h"
#include "../entity.h"

cgltf_data* load_gltf_file(const char* path);
void free_gltf_file(cgltf_data* data);
Pose load_rest_pose(const cgltf_data* data);
Pose load_bind_pose(const cgltf_data* data);
std::vector<std::string> load_joint_names(const cgltf_data* data); 
mat4 load_armature_transform(const cgltf_data* data);
Skeleton load_skeleton(const cgltf_data* data);
SkinnedEntity* load_meshes(const cgltf_data* data);

namespace GLTFHelpers
{
	Transform get_local_transform(cgltf_node& node);
	int get_node_index(cgltf_node* target, cgltf_node* all_nodes, unsigned int num_nodes);
	int get_joint_index(cgltf_node* joint, cgltf_node** all_joints, unsigned int num_joints);
	void get_scalar_values(std::vector<float>& out, unsigned int comp_count, const cgltf_accessor& in_accessor);
	void material_from_primitive(Entity& entity, cgltf_primitive& primitive);
	void mesh_from_attribute(Mesh& out_mesh, cgltf_attribute& attribute, cgltf_skin* skin, cgltf_node* nodes, unsigned int node_count);
};