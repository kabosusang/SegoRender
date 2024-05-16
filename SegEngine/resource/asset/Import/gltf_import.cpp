#include "gltf_import.hpp"

#include "core/Vulkan/include/context.hpp"


#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#ifdef VK_USE_PLATFORM_ANDROID_KHR
#define TINYGLTF_ANDROID_LOAD_FROM_ASSETS
#endif

#define INVALID_INDEX -1

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Sego{


void GlTFImporter::LoadNodes(const tinygltf::Node& inputNode, 
const tinygltf::Model& input, Node* parent, 
std::vector<uint32_t>& indexBuffer, std::vector<StaticVertex>& vertexBuffer,
std::shared_ptr<StaticMeshRenderData>& meshRenderData)
{
		Node* node = new Node{};
		node->matrix = glm::mat4(1.0f);
		node->parent = parent;

		// Get the local node matrix
		// It's either made up from translation, rotation, scale or a 4x4 matrix
		if (inputNode.translation.size() == 3) {
			node->matrix = glm::translate(node->matrix, glm::vec3(glm::make_vec3(inputNode.translation.data())));
		}
		if (inputNode.rotation.size() == 4) {
			glm::quat q = glm::make_quat(inputNode.rotation.data());
			node->matrix *= glm::mat4(q);
		}
		if (inputNode.scale.size() == 3) {
			node->matrix = glm::scale(node->matrix, glm::vec3(glm::make_vec3(inputNode.scale.data())));
		}
		if (inputNode.matrix.size() == 16) {
			node->matrix = glm::make_mat4x4(inputNode.matrix.data());
		};

		// Load node's children
		if (inputNode.children.size() > 0) {
			for (size_t i = 0; i < inputNode.children.size(); i++) {
				LoadNodes(input.nodes[inputNode.children[i]], input , node, indexBuffer, vertexBuffer,meshRenderData);
			}
		}

		// If the node contains mesh data, we load vertices and indices from the buffers
		// In glTF this is done via accessors and buffer views
		if (inputNode.mesh > -1) {
			const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];
			// Iterate through all primitives of this node's mesh
			for (size_t i = 0; i < mesh.primitives.size(); i++) {
				const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
				uint32_t firstIndex = static_cast<uint32_t>(indexBuffer.size());
				uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
				uint32_t indexCount = 0;
				// Vertices
				{
					const float* positionBuffer = nullptr;
					const float* normalsBuffer = nullptr;
					const float* texCoordsBuffer = nullptr;
					size_t vertexCount = 0;

					// Get buffer data for vertex positions
					if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end()) {
						const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("POSITION")->second];
						const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
						positionBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
						vertexCount = accessor.count;
					}
					// Get buffer data for vertex normals
					if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end()) {
						const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
						const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
						normalsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					}
					// Get buffer data for vertex texture coordinates
					// glTF supports multiple sets, we only load the first one
					if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end()) {
						const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
						const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
						texCoordsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					}

					// Append data to model's vertex buffer
					for (size_t v = 0; v < vertexCount; v++) {
						StaticVertex vert{};
						vert.pos = glm::make_vec3(&positionBuffer[v * 3]);
						vert.normal = glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f));
						vert.uv = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
						vert.uv.y = 1.0f - vert.uv.y; // Vulkan has a Y axis starting at the bottom of the image for textures
						vert.color = glm::vec3(1.0f);
						vertexBuffer.push_back(vert);
					}
				}
				// Indices
				{
					const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.indices];
					const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];

					indexCount += static_cast<uint32_t>(accessor.count);

					// glTF supports different component types of indices
					switch (accessor.componentType) {
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
						const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
						for (size_t index = 0; index < accessor.count; index++) {
							indexBuffer.push_back(buf[index] + vertexStart);
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
						const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
						for (size_t index = 0; index < accessor.count; index++) {
							indexBuffer.push_back(buf[index] + vertexStart);
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
						const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
						for (size_t index = 0; index < accessor.count; index++) {
							indexBuffer.push_back(buf[index] + vertexStart);
						}
						break;
					}
					default:
						std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
						SG_CORE_ERROR("Index component type not supported!");
						return;
					}
				}
				Primitive primitive{};
				primitive.firstIndex = firstIndex;
				primitive.indexCount = indexCount;
				primitive.materialIndex = glTFPrimitive.material;
				node->mesh.primitives.push_back(primitive);
			}
		}

		if (parent) {
			parent->children.push_back(node);
		}
		else {
			meshRenderData->nodes_.push_back(node);
		}
}

std::shared_ptr<StaticMeshRenderData> GlTFImporter::LoadglTFFile(const std::string& filename){
    tinygltf::Model gltf_model;
    tinygltf::TinyGLTF loader;
    std::string error,warning;
    std::string extension = std::filesystem::path(filename).extension().string();
	bool is_gltf = extension == ".gltf";
	bool success = false;

	if(is_gltf){
		success = loader.LoadASCIIFromFile(&gltf_model, &error, &warning, filename);
	}else{
		success = loader.LoadBinaryFromFile(&gltf_model, &error, &warning, filename);
	}

	if(!success){
		SG_CORE_ERROR("Failed to load glTF file: {0}!,error:{1},warning: {2}", filename,error,warning);
	
	}
    std::shared_ptr<StaticMeshRenderData> mesh_data = std::make_shared<StaticMeshRenderData>();
    
    //1. Load Images
    mesh_data->textures_.resize(gltf_model.images.size());
    for(size_t i = 0; i < gltf_model.images.size(); i++){
        tinygltf::Image& glTFImage = gltf_model.images[i];
        
        mesh_data->textures_[i].image_data_ = glTFImage.image;
        mesh_data->textures_[i].width_ = glTFImage.width;
        mesh_data->textures_[i].height_ = glTFImage.height;
        mesh_data->textures_[i].format_ = vk::Format::eR8G8B8A8Unorm;
        mesh_data->textures_[i].loadFromMemory();
    }

    //2. Load Materials
    mesh_data->materials_.resize(gltf_model.materials.size());
    for(size_t i =0; i < gltf_model.materials.size(); i++){
        tinygltf::Material& glTFMaterial = gltf_model.materials[i];
      
        //Get the base color texture
        if(glTFMaterial.values.find("baseColorFactor") != glTFMaterial.values.end()){
        	mesh_data->materials_[i].baseColorFactor =  glm::make_vec4(glTFMaterial.values["baseColorFactor"].ColorFactor().data());
        }
        //Get the color texture index
        if(glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end()){
           	mesh_data->materials_[i].baseColorTextureIndex = glTFMaterial.values["baseColorTexture"].TextureIndex();
			mesh_data->materials_[i].has_baseColorTexture = 1;
		}

    }

    //3. Load Textures
    mesh_data->textureindex_.resize(gltf_model.textures.size());
    for(size_t i = 0; i < gltf_model.textures.size(); i++){
        mesh_data->textureindex_[i].imageIndex = gltf_model.textures[i].source;
    }

    std::vector<uint32_t> indexBuffer;
    std::vector<StaticVertex> vertexBuffer;

    const tinygltf::Scene& scene = gltf_model.scenes[0];
    //4. Load Meshes
    for (size_t i = 0; i < scene.nodes.size(); i++) {
        const tinygltf::Node node = gltf_model.nodes[scene.nodes[i]];
        LoadNodes(node, gltf_model, nullptr, indexBuffer, vertexBuffer,mesh_data);
    }
    //5. Create buffers
    Vulkantool::createVertexBuffer(sizeof(vertexBuffer[0])* vertexBuffer.size(),
    (void*)vertexBuffer.data(), mesh_data->vertexBuffer_);
   
    Vulkantool::createIndexBuffer(sizeof(indexBuffer[0]) * indexBuffer.size(),
    (void*)indexBuffer.data(), mesh_data->indexBuffer_);

    return mesh_data;
}

}

