#include "gLTFLoader.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <algorithm>

#include "../math/transform.h"
#include "../animations/skeleton.h"

// takes a path and returns a cgltf_data pointer
cgltf_data* load_gltf_file(const char* path)
{
	cgltf_options options;
	memset(&options, 0, sizeof(cgltf_options));
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, path, &data);
	if (result != cgltf_result_success) {
		std::cout << "Could not load: " << path << "\n";
		return 0;
	}
	result = cgltf_load_buffers(&options, data, path);
	if (result != cgltf_result_success) {
		cgltf_free(data);
		std::cout << "Could not load: " << path << "\n";
		return 0;
	}
	result = cgltf_validate(data);
	if (result != cgltf_result_success) {
		cgltf_free(data);
		std::cout << "Invalid file: " << path << "\n";
		return 0;
	}
	return data;
}

void free_gltf_file(cgltf_data* data)
{
	if (data == 0) {
		std::cout << "WARNING: Can't free null data\n";
	}
	else {
		cgltf_free(data);
	}
}

Pose load_rest_pose(const cgltf_data* data)
{
	unsigned int num_skins = data->skins_count;
	if (num_skins > 1) {
		std::cout << "[Warning] Multiple skins not supported." << std::endl;
	}

	cgltf_skin skin = data->skins[0];
	unsigned int num_joints = skin.joints_count;
	Pose rest_pose(num_joints);
	
	for (unsigned int i = 0; i < num_joints; i++)
	{
		cgltf_node* node = skin.joints[i];
		// TODO: TASK3 Set the information of the joint (local transform and parent id) to the rest_pose. Use get_local_transform and get_joint_index to retrieve the information.
		
		//...
	}
	return rest_pose;
}

Pose load_bind_pose(const cgltf_data* data)
{
	Pose rest_pose = load_rest_pose(data);

	unsigned int num_bones = rest_pose.size();
	std::vector<mat4> world_bind_pose(num_bones);
	// initialize the array of matrices with the global transforms of the rest pose bones
	for (unsigned int i = 0; i < num_bones; i++) {
		world_bind_pose[i] = transform_to_mat4(rest_pose.get_global_transform(i));
	}

	// get number of skins (skinned meshes)
	unsigned int num_skins = data->skins_count;
	for (unsigned int i = 0; i < num_skins; i++) {
		cgltf_skin* skin = &(data->skins[i]);
		// read the inverse bind pose matrix into a large vector of float values
		std::vector<float> inv_bind_accessor;
		GLTFHelpers::get_scalar_values(inv_bind_accessor, 16, *skin->inverse_bind_matrices);

		// for each joint in the skin, get the bind pose world matrix
		unsigned int num_joints = skin->joints_count;
		for (unsigned int j = 0; j < num_joints; j++) {
			// get the 16 values of the inverse world bind matrix and put them into a mat4
			float* matrix = &(inv_bind_accessor[j * 16]);
			mat4 inv_bind_matrix = mat4(matrix);
			
			//compute the world matrix of the bind pose
			mat4 bind_matrix = inverse(inv_bind_matrix);
					 
			// update the world_bind_pose array with the global matrix of the joint in bind pose.
			int joint_index = GLTFHelpers::get_joint_index(skin->joints[j], skin->joints, num_joints);
			world_bind_pose[joint_index] = bind_matrix;
		}
	}

	Pose bind_pose = rest_pose;
	for (unsigned int i = 0; i < num_bones; ++i) {
		// TODO: TASK3 Configure the bind pose so the transforms of each joint are relative to its parent. Use the world_bind_pose. Be careful with the root bone, it has no parent!
		
		//...
	}
	return bind_pose;
}

// loads the names of every joint in the same order that the joints for the rest pose were loaded
std::vector<std::string> load_joint_names(const cgltf_data* data)
{
	cgltf_skin skin = data->skins[0];
	unsigned int num_joints = skin.joints_count;

	std::vector<std::string> result(num_joints, "Not Set");
	for (unsigned int i = 0; i < num_joints; ++i) {

		cgltf_node* node = skin.joints[i];
		if (node->name == 0) {
			result[i] = "EMPTY NODE";
		}
		else {
			result[i] = node->name;
		}
	}
	return result;
}

mat4 load_armature_transform(const cgltf_data* data)
{
	for (unsigned int i = 0; i < data->nodes_count; i++)
	{
		if (*(data->nodes[i].name) == *(data->skins[0].name)) {
			Transform t = GLTFHelpers::get_local_transform(data->nodes[i]);
			return transform_to_mat4(t);
		}
	}

	return mat4(); // no armature was found
}

Skeleton load_skeleton(const cgltf_data* data)
{	
	return Skeleton(
		load_rest_pose(data),
		load_bind_pose(data),
		load_joint_names(data)
	);
}

SkinnedEntity* load_meshes(const cgltf_data* data)
{
	SkinnedEntity* result = new SkinnedEntity();
	cgltf_node* nodes = data->nodes;
	unsigned int node_count = data->nodes_count;

	for (unsigned int i = 0; i < node_count; ++i) {
		cgltf_node* node = &nodes[i];
		// Only process nodes that have both a mesh and a skin
		if (node->mesh == 0 || node->skin == 0) {
			continue;
		}
		result->name = node->parent->name;
		int num_prims = node->mesh->primitives_count;
		for (int j = 0; j < num_prims; ++j) {

			// create a mesh for each primitive
			SkinnedEntity* entity = new SkinnedEntity(node->name);
			entity->parent = result;
			entity->mesh = new Mesh();
			Mesh* mesh = entity->mesh;
			mesh->name = node->name;
			result->children.push_back(entity);

			// Loop through all the attributes in the primitive and populate the mesh data
			cgltf_primitive* primitive = &node->mesh->primitives[j];
			unsigned int atributes_count = primitive->attributes_count;
			for (unsigned int k = 0; k < atributes_count; ++k) {
				cgltf_attribute* attribute = &primitive->attributes[k];
				GLTFHelpers::mesh_from_attribute(*mesh, *attribute, node->skin, nodes, node_count);

				// if the primitive has indices, the index buffer of the mesh needs to be filled out
				if (primitive->indices != 0) {
					int ic = primitive->indices->count;
					std::vector<unsigned int>& indices = mesh->indices; //mesh.get_indices();
					indices.resize(ic);

					for (unsigned int k = 0; k < ic; ++k) {
						indices[k] = cgltf_accessor_read_index(primitive->indices, k);
					}
				}
			}

			// for render the mesh
			mesh->upload_to_vram();

			// Assign the material information to the mesh
			if (primitive->material) {
				GLTFHelpers::material_from_primitive(*entity, *primitive);
			}
		}
	}

	// Set the armature transform as the model of the parent node
	mat4 model = load_armature_transform(data);
	result->set_model(model);

	// return the resulting vector of meshes
	return result;
}

void GLTFHelpers::material_from_primitive(Entity& entity, cgltf_primitive& primitive)
{
	PBRMaterial* material = new PBRMaterial();

	if (primitive.material->has_pbr_metallic_roughness)
	{
		cgltf_pbr_metallic_roughness pbr_mat = primitive.material->pbr_metallic_roughness;
		cgltf_texture* albedo_tex = pbr_mat.base_color_texture.texture;
		if (albedo_tex)
		{
			cgltf_buffer_view* buffer_view = albedo_tex->image->buffer_view;
			int width, height, num_components;
			const unsigned char* data = buffer_view->offset + (unsigned char*)buffer_view->buffer->data;
			int byte_count = buffer_view->size;
			if (!stbi_info_from_memory(data, byte_count, &width, &height, &num_components)) {
				assert(false);
			}
			unsigned char* image_data = stbi_load_from_memory(data, byte_count, &width, &height, &num_components, 4);

			// create texture and link it to the material
			material->albedo_tex = new Texture(width, height, num_components == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, true, image_data, 0);
			stbi_image_free(image_data);

		}
		// store properties of the texture
		float* color_factor = pbr_mat.base_color_factor;
		material->color = vec4(color_factor, 1.f);

		cgltf_texture* met_rou_tex = pbr_mat.metallic_roughness_texture.texture;
		if (met_rou_tex)
		{
			cgltf_buffer_view* buffer_view = met_rou_tex->image->buffer_view;
			int width, height, num_components;
			const unsigned char* data = buffer_view->offset + (unsigned char*)buffer_view->buffer->data;
			int byte_count = buffer_view->size;
			if (!stbi_info_from_memory(data, byte_count, &width, &height, &num_components)) {
				assert(false);
			}
			unsigned char* image_data = stbi_load_from_memory(data, byte_count, &width, &height, &num_components, 4);

			// create texture and link it to the material
			material->met_rou_tex = new Texture(width, height, num_components == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, true, image_data, 0);
			stbi_image_free(image_data);

			// store properties of the texture
			material->metallic = pbr_mat.metallic_factor;
			material->roughness = pbr_mat.roughness_factor;
		}

		if (!albedo_tex && !met_rou_tex) {
			entity.material = new FlatMaterial();
			entity.material->color = material->color;
			return;
		}
	}
	if (primitive.material->normal_texture.texture)
	{
		cgltf_texture* normal_tex = primitive.material->normal_texture.texture;

		cgltf_buffer_view* buffer_view = normal_tex->image->buffer_view;
		int width, height, num_components;
		const unsigned char* data = buffer_view->offset + (unsigned char*)buffer_view->buffer->data;
		int byte_count = buffer_view->size;
		if (!stbi_info_from_memory(data, byte_count, &width, &height, &num_components)) {
			assert(false);
		}
		unsigned char* image_data = stbi_load_from_memory(data, byte_count, &width, &height, &num_components, 4);

		// create texture and link it to the material
		material->normal_tex = new Texture(width, height, num_components == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, true, image_data, 0);
		stbi_image_free(image_data);
	}
	// @TODO: support other textures or material properties
	

	// @TODO: set opengl flags to the material
	//primitive.material->double_sided;
	//primitive.material->alpha_cutoff;
	//primitive.material->alpha_mode;

	// Set the material created to the entity

	entity.material = material;
}

// Gets the local transform of cgltf_node
Transform GLTFHelpers::get_local_transform(cgltf_node& n)
{
	Transform result;

	// if the node stores its transformation as a matrix, use the mat4ToTransform decomposition function;
	if (n.has_matrix) {
		mat4 mat(&n.matrix[0]);
		result = mat4_to_transform(mat);
	}
	if (n.has_translation) {
		result.position = vec3(n.translation[0], n.translation[1], n.translation[2]);
	}
	if (n.has_rotation) {
		result.rotation = quat(n.rotation[0], n.rotation[1], n.rotation[2], n.rotation[3]);
	}

	if (n.has_scale) {
		result.scale = vec3(n.scale[0], n.scale[1], n.scale[2]);
	}
	return result;
}

// Gets the node index given all the nodes
int GLTFHelpers::get_node_index(cgltf_node* node, cgltf_node* all_nodes, unsigned int num_nodes)
{
	if (node == 0) {
		return -1;
	}
	for (unsigned int i = 0; i < num_nodes; ++i) {
		if (node == &all_nodes[i]) {
			return (int)i;
		}
	}
	return -1;
}

// Gets the node index given all the nodes
int GLTFHelpers::get_joint_index(cgltf_node* joint, cgltf_node** all_joints, unsigned int num_joints)
{
	if (joint == 0) {
		return -1;
	}
	for (unsigned int i = 0; i < num_joints; ++i) {
		if (joint == all_joints[i]) {
			return (int)i;
		}
	}
	return -1;
}

// Reads the floating-point values of a gltf accessor and put them into a vector of floats
void GLTFHelpers::get_scalar_values(std::vector<float>& out, unsigned int comp_count, const cgltf_accessor& in_accessor)
{
	out.resize(in_accessor.count * comp_count);
	for (cgltf_size i = 0; i < in_accessor.count; ++i) {
		cgltf_accessor_read_float(&in_accessor, i, &out[i * comp_count], comp_count);
	}
}

/*
It takes a mesh and a cgltf_attribute function, along with some additional data required for parsing.
The attribute contains one of our mesh components, such as the position, normal, UV coordinate, weights, or influences.
This attribute provides the appropriate mesh data
*/
void GLTFHelpers::mesh_from_attribute(Mesh& out_mesh, cgltf_attribute& attribute, cgltf_skin* skin, cgltf_node* nodes, unsigned int node_count)
{
	cgltf_attribute_type attrib_type = attribute.type;
	cgltf_accessor& accessor = *attribute.data;

	// get how many attributes the current component has
	unsigned int component_count = 0;
	if (accessor.type == cgltf_type_vec2) {
		component_count = 2;
	}
	else if (accessor.type == cgltf_type_vec3) {
		component_count = 3;
	}
	else if (accessor.type == cgltf_type_vec4) {
		component_count = 4;
	}

	// Parse the data
	std::vector<float> values;
	get_scalar_values(values, component_count, accessor);
	unsigned int acessor_count = accessor.count;

	// Create references to the position, normal, texture coordinate, influences, and weights vectors of the mesh
	std::vector<vec3>& positions = out_mesh.vertices; //out_mesh.get_positions();
	std::vector<vec3>& normals = out_mesh.normals; //out_mesh.get_normals();
	std::vector<vec2>& texCoords = out_mesh.uvs; //out_mesh.get_tex_coords();
	std::vector<ivec4>& influences = out_mesh.bones; //out_mesh.get_influences();
	std::vector<vec4>& weights = out_mesh.weights; //out_mesh.get_weights();

	// loop through all the values in the current accessor and assign them to the appropriate vector based on the accessor type
	for (unsigned int i = 0; i < acessor_count; ++i) {
		int index = i * component_count;
		switch (attrib_type)
		{
		case cgltf_attribute_type_position:
			positions.push_back(vec3(values[index + 0], values[index + 1], values[index + 2]));
			break;

		case cgltf_attribute_type_normal:
		{
			vec3 normal = vec3(values[index + 0], values[index + 1], values[index + 2]);
			// if the normal is not normalized, normalize it
			if (len_sq(normal) < 0.000001f) {
				normal = vec3(0, 1, 0);
			}
			normals.push_back(normalized(normal));
		}
		break;

		case cgltf_attribute_type_texcoord:
			texCoords.push_back(vec2(values[index + 0], values[index + 1]));
			break;

		case cgltf_attribute_type_weights:
			weights.push_back(vec4(values[index + 0], values[index + 1], values[index + 2], values[index + 3]));
			break;

		case cgltf_attribute_type_joints:
		{
			// Joints are stored as floating-point numbers.Convert them into integers: 
			// These indices are skin relative. This function has no information about the skin that is being parsed.
			// Add +0.5f to round, since we can't read integers
			ivec4 joints(
				(int)(values[index + 0] + 0.5f),
				(int)(values[index + 1] + 0.5f),
				(int)(values[index + 2] + 0.5f),
				(int)(values[index + 3] + 0.5f)
			);

			// Make sure that even the invalid nodes have a value of 0 (any negative joint indices will break the skinning implementation)
			if (joints.x < 0 || joints.y < 0 || joints.z < 0 || joints.w < 0) {

				printf("%i", joints.x);
				printf("%i", joints.y);
				printf("%i", joints.z);
				printf("%i", joints.w);
			}
			joints.x = std::max(0, joints.x);
			joints.y = std::max(0, joints.y);
			joints.z = std::max(0, joints.z);
			joints.w = std::max(0, joints.w);

			influences.push_back(joints);
		}
		break;
		}
	}
}