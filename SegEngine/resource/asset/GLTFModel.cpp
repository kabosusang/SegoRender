#include "GLTFModel.hpp"
#include "core/Vulkan/include/context.hpp"
using namespace Sego;

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "core/Vulkan/Vulkan_rhi.hpp"

namespace GltfModel{
// Bounding box
	BoundingBox::BoundingBox() {
	}
    BoundingBox::BoundingBox(const glm::vec3 &min, const glm::vec3 &max): min(min), max(max) {};

	BoundingBox BoundingBox::getAABB(glm::mat4 m) {
		glm::vec3 min = glm::vec3(m[3]);
		glm::vec3 max = min;
		glm::vec3 v0, v1;
			
		glm::vec3 right = glm::vec3(m[0]);
		v0 = right * this->min.x;
		v1 = right * this->max.x;
		min += glm::min(v0, v1);
		max += glm::max(v0, v1);

		glm::vec3 up = glm::vec3(m[1]);
		v0 = up * this->min.y;
		v1 = up * this->max.y;
		min += glm::min(v0, v1);
		max += glm::max(v0, v1);

		glm::vec3 back = glm::vec3(m[2]);
		v0 = back * this->min.z;
		v1 = back * this->max.z;
		min += glm::min(v0, v1);
		max += glm::max(v0, v1);

		return BoundingBox(min, max);
	}

    Primitive::Primitive(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount, PBRMaterial &material)
    : firstIndex(firstIndex), indexCount(indexCount), vertexCount(vertexCount), material(material)
	{
		hasIndices = indexCount > 0;
    }

    void Primitive::setBoundingBox(const glm::vec3 &min, const glm::vec3 &max)
    {
		bb.min = min;
		bb.max = max;
		bb.valid = true;
    }

void GLTFTexture::fromglTfImage(tinygltf::Image & gltfimage, TextureSampler textureSampler)
{	
	unsigned char* buffer = nullptr;
	vk::DeviceSize bufferSize = 0;
	bool deleteBuffer = false;
	if (gltfimage.component == 3) {
			// Most devices don't support RGB only on Vulkan so convert if necessary
			// TODO: Check actual format support and transform only if required
			bufferSize = gltfimage.width * gltfimage.height * 4;
			buffer = new unsigned char[bufferSize];
			unsigned char* rgba = buffer;
			unsigned char* rgb = &gltfimage.image[0];
			for (int32_t i = 0; i< gltfimage.width * gltfimage.height; ++i) {
				for (int32_t j = 0; j < 3; ++j) {
					rgba[j] = rgb[j];
				}
				rgba += 4;
				rgb += 3;
			}
			deleteBuffer = true;
		}
		else {
			buffer = &gltfimage.image[0];
			bufferSize = gltfimage.image.size();
		}

	format_ = vk::Format::eR8G8B8A8Unorm;
	width_ = gltfimage.width;
	height_ = gltfimage.height;
	mipLevels_ = static_cast<uint32_t>(floor(log2(std::max(width_, height_))) + 1.0);
	//Create image
	vk::ImageUsageFlags image_usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc;
	if(buffer){
		image_usage |= vk::ImageUsageFlagBits::eTransferDst;
	}
	Vulkantool::createImage(width_, height_, mipLevels_, layerCount_, 
	vk::SampleCountFlagBits::e1, format_, vk::ImageTiling::eOptimal, 
	image_usage, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, image_view_sampler_.vma_image);

	vk::Image image = image_view_sampler_.image();
	image_view_sampler_.image_view = Vulkantool::createImageView(
	image, format_, Vulkantool::calcImageAspectFlags(format_), mipLevels_, layerCount_);

	//create sampler
	image_view_sampler_.sampler = Vulkantool::createSample(textureSampler.minFilter, textureSampler.magFilter, mipLevels_, 
	textureSampler.addressModeU, textureSampler.addressModeU, textureSampler.addressModeU);
	//set image layout add descriptor type
	image_view_sampler_.image_layout = vk::ImageLayout::eShaderReadOnlyOptimal;
	image_view_sampler_.descriptor_type = vk::DescriptorType::eCombinedImageSampler;

	//Load texture image
	VmaBuffer stagin_buffer;
	Vulkantool::createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_AUTO_PREFER_HOST, stagin_buffer);
	//Copy image pixel data to staging buffer
    Vulkantool::updateBuffer(stagin_buffer, buffer, bufferSize);
	//Create image
	Vulkantool::transitionImageLayout(image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, format_, mipLevels_, layerCount_);
	//copy staging buffer to image
	Vulkantool::copyBufferToImage(stagin_buffer.buffer, image, width_,height_);
	//destroy staging buffer
	vmaDestroyBuffer(Context::Instance().getAllocator(), stagin_buffer.buffer, stagin_buffer.allocation);
	// generate image mipmaps, and transition image to READ_ONLY_OPT state for shader reading
    Vulkantool::createImageMipmaps(image, width_, height_, mipLevels_);
}

//Mesh
Mesh::Mesh(glm::mat4 matrix){
	this->uniformBlock.matrix = matrix;
	Vulkantool::createBuffer(sizeof(UniformBlock), vk::BufferUsageFlagBits::eUniformBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU, uniformBuffer.buffer);
}

Mesh::~Mesh(){
	for (auto p : primitives)
		delete p;
	uniformBuffer.buffer.destroy();
}	

void Mesh::setBoundingBox(const glm::vec3& min, const glm::vec3& max){
	bb.min = min;
	bb.max = max;
	bb.valid = true;
}
//Node
glm::mat4 Node::localMatrix(){
	if (!useCachedMatrix){
		cachedLocalMatrix = glm::translate(glm::mat4(1.0f),translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * matrix;
	};
	return cachedLocalMatrix;
}

glm::mat4 Node::getMatrix(){
	// Use a simple caching algorithm to avoid having to recalculate matrices to often while traversing the node hierarchy
		if (!useCachedMatrix) {
			glm::mat4 m = localMatrix();
			Node* p = parent;
			while (p) {
				m = p->localMatrix() * m;
				p = p->parent;
			}
			cachedMatrix = m;
			useCachedMatrix = true;
			return m;
		} else {
			return cachedMatrix;
		}
}

void Node::update(){
	useCachedMatrix = false;
	if (mesh){
		glm::mat4 m = getMatrix();
		if (skin){
			mesh->uniformBlock.matrix = m;
			// Update join matrices
			glm::mat4 inverseTransform = glm::inverse(m);
			size_t numJoints = std::min((uint32_t)skin->joints.size(),MAX_NUM_JOINTS);
			for (size_t i = 0;i < numJoints; i++){
				Node* joint = skin->joints[i];
				glm::mat4 jointMat = joint->getMatrix() * skin->inverseBindMatrices[i];
				jointMat = inverseTransform * jointMat;
				mesh->uniformBlock.jointMatrix[i] = jointMat;
			}
			mesh->uniformBlock.jointcount = (float)numJoints;
			Vulkantool::updateBuffer(mesh->uniformBuffer.buffer, &mesh->uniformBlock, sizeof(mesh->uniformBlock));
		}else{
			Vulkantool::updateBuffer(mesh->uniformBuffer.buffer, &m, sizeof(glm::mat4));
		}
	}

	for (auto& child : children){
		child->update();
	}
}

Node::~Node(){
	if (mesh){
		delete mesh;
	}
	for (auto& child : children){
		delete child;
	}

}

//Model
void Model::destory(){
	vertexBuffer_.destroy();
	indexBuffer_.destroy();
	
	for (auto texture : textures){
		texture.destory();
	}

	textures.resize(0);
	for (auto node : nodes){
		delete node;
	}
	materials.resize(0);
	animations.resize(0);
	nodes.resize(0);
	linearNodes.resize(0);
	extensions.resize(0);
	for (auto skin : skins)
		delete skin;
	skins.resize(0);
}

void Model::loadNode(Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model, LoaderInfo& loaderInfo, float globalscale){
	Node* newNode = new Node{};
	newNode->index = nodeIndex;
	newNode->parent = parent;
	newNode->name = node.name;
	newNode->matrix = glm::mat4(1.0f);
	newNode->skinIndex = node.skin;
	
	// Generate local node matrix
	glm::vec3 translation = glm::vec3(0.0f);
	if (node.translation.size() == 3) {
		translation = glm::make_vec3(node.translation.data());
		newNode->translation = translation;
	}
	glm::mat4 rotation = glm::mat4(1.0f);
		if (node.rotation.size() == 4) {
			glm::quat q = glm::make_quat(node.rotation.data());
			newNode->rotation = glm::mat4(q);
		}
		glm::vec3 scale = glm::vec3(1.0f);
		if (node.scale.size() == 3) {
			scale = glm::make_vec3(node.scale.data());
			newNode->scale = scale;
		}
		if (node.matrix.size() == 16) {
			newNode->matrix = glm::make_mat4x4(node.matrix.data());
		};

		// Node with children
		if (node.children.size() > 0) {
			for (size_t i = 0; i < node.children.size(); i++) {
				loadNode(newNode, model.nodes[node.children[i]], node.children[i], model, loaderInfo, globalscale);
			}
		}

		// Node contains mesh data
		if (node.mesh > -1) {
			const tinygltf::Mesh mesh = model.meshes[node.mesh];
			Mesh *newMesh = new Mesh(newNode->matrix);
			for (size_t j = 0; j < mesh.primitives.size(); j++) {
				const tinygltf::Primitive &primitive = mesh.primitives[j];
				uint32_t vertexStart = static_cast<uint32_t>(loaderInfo.vertexPos);
				uint32_t indexStart = static_cast<uint32_t>(loaderInfo.indexPos);
				uint32_t indexCount = 0;
				uint32_t vertexCount = 0;
				glm::vec3 posMin{};
				glm::vec3 posMax{};
				bool hasSkin = false;
				bool hasIndices = primitive.indices > -1;
				// Vertices
				{
					const float *bufferPos = nullptr;
					const float *bufferNormals = nullptr;
					const float *bufferTexCoordSet0 = nullptr;
					const float *bufferTexCoordSet1 = nullptr;
					const float* bufferColorSet0 = nullptr;
					const void *bufferJoints = nullptr;
					const float *bufferWeights = nullptr;

					int posByteStride;
					int normByteStride;
					int uv0ByteStride;
					int uv1ByteStride;
					int color0ByteStride;
					int jointByteStride;
					int weightByteStride;

					int jointComponentType;

					// Position attribute is required
					assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

					const tinygltf::Accessor &posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
					const tinygltf::BufferView &posView = model.bufferViews[posAccessor.bufferView];
					bufferPos = reinterpret_cast<const float *>(&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
					posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
					posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);
					vertexCount = static_cast<uint32_t>(posAccessor.count);
					posByteStride = posAccessor.ByteStride(posView) ? (posAccessor.ByteStride(posView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);

					if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
						const tinygltf::Accessor &normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
						const tinygltf::BufferView &normView = model.bufferViews[normAccessor.bufferView];
						bufferNormals = reinterpret_cast<const float *>(&(model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
						normByteStride = normAccessor.ByteStride(normView) ? (normAccessor.ByteStride(normView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
					}

					// UVs
					if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
						const tinygltf::Accessor &uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
						const tinygltf::BufferView &uvView = model.bufferViews[uvAccessor.bufferView];
						bufferTexCoordSet0 = reinterpret_cast<const float *>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
						uv0ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
					}
					if (primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end()) {
						const tinygltf::Accessor &uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_1")->second];
						const tinygltf::BufferView &uvView = model.bufferViews[uvAccessor.bufferView];
						bufferTexCoordSet1 = reinterpret_cast<const float *>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
						uv1ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
					}

					// Vertex colors
					if (primitive.attributes.find("COLOR_0") != primitive.attributes.end()) {
						const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("COLOR_0")->second];
						const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
						bufferColorSet0 = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
						color0ByteStride = accessor.ByteStride(view) ? (accessor.ByteStride(view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
					}

					// Skinning
					// Joints
					if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end()) {
						const tinygltf::Accessor &jointAccessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];
						const tinygltf::BufferView &jointView = model.bufferViews[jointAccessor.bufferView];
						bufferJoints = &(model.buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]);
						jointComponentType = jointAccessor.componentType;
						jointByteStride = jointAccessor.ByteStride(jointView) ? (jointAccessor.ByteStride(jointView) / tinygltf::GetComponentSizeInBytes(jointComponentType)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
					}

					if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end()) {
						const tinygltf::Accessor &weightAccessor = model.accessors[primitive.attributes.find("WEIGHTS_0")->second];
						const tinygltf::BufferView &weightView = model.bufferViews[weightAccessor.bufferView];
						bufferWeights = reinterpret_cast<const float *>(&(model.buffers[weightView.buffer].data[weightAccessor.byteOffset + weightView.byteOffset]));
						weightByteStride = weightAccessor.ByteStride(weightView) ? (weightAccessor.ByteStride(weightView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
					}

					hasSkin = (bufferJoints && bufferWeights);

					for (size_t v = 0; v < posAccessor.count; v++) {
						MeshAndSkeletonVertex& vert = loaderInfo.vertexBuffer[loaderInfo.vertexPos];
						vert.pos = glm::vec4(glm::make_vec3(&bufferPos[v * posByteStride]), 1.0f);
						vert.normal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * normByteStride]) : glm::vec3(0.0f)));
						vert.uv0 = bufferTexCoordSet0 ? glm::make_vec2(&bufferTexCoordSet0[v * uv0ByteStride]) : glm::vec3(0.0f);
						vert.uv0.y = 1.0f - vert.uv0.y; //Vulkan 
						vert.uv1 = bufferTexCoordSet1 ? glm::make_vec2(&bufferTexCoordSet1[v * uv1ByteStride]) : glm::vec3(0.0f);
						vert.uv1.y = 1.0f - vert.uv1.y; //Vulkan
						vert.color = bufferColorSet0 ? glm::make_vec4(&bufferColorSet0[v * color0ByteStride]) : glm::vec4(1.0f);

						if (hasSkin)
						{
							switch (jointComponentType) {
							case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
								const uint16_t *buf = static_cast<const uint16_t*>(bufferJoints);
								vert.joint0 = glm::vec4(glm::make_vec4(&buf[v * jointByteStride]));
								break;
							}
							case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
								const uint8_t *buf = static_cast<const uint8_t*>(bufferJoints);
								vert.joint0 = glm::vec4(glm::make_vec4(&buf[v * jointByteStride]));
								break;
							}
							default:
								// Not supported by spec
								std::cerr << "Joint component type " << jointComponentType << " not supported!" << std::endl;
								break;
							}
						}
						else {
							vert.joint0 = glm::vec4(0.0f);
						}
						vert.weight0 = hasSkin ? glm::make_vec4(&bufferWeights[v * weightByteStride]) : glm::vec4(0.0f);
						// Fix for all zero weights
						if (glm::length(vert.weight0) == 0.0f) {
							vert.weight0 = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
						}
						loaderInfo.vertexPos++;
					}
				}
				// Indices
				if (hasIndices)
				{
					const tinygltf::Accessor &accessor = model.accessors[primitive.indices > -1 ? primitive.indices : 0];
					const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

					indexCount = static_cast<uint32_t>(accessor.count);
					const void *dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

					switch (accessor.componentType) {
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
						const uint32_t *buf = static_cast<const uint32_t*>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++) {
							loaderInfo.indexBuffer[loaderInfo.indexPos] = buf[index] + vertexStart;
							loaderInfo.indexPos++;
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
						const uint16_t *buf = static_cast<const uint16_t*>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++) {
							loaderInfo.indexBuffer[loaderInfo.indexPos] = buf[index] + vertexStart;
							loaderInfo.indexPos++;
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
						const uint8_t *buf = static_cast<const uint8_t*>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++) {
							loaderInfo.indexBuffer[loaderInfo.indexPos] = buf[index] + vertexStart;
							loaderInfo.indexPos++;
						}
						break;
					}
					default:
						std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
						return;
					}
				}					
				Primitive *newPrimitive = new Primitive(indexStart, indexCount, vertexCount, primitive.material > -1 ? materials[primitive.material] : materials.back());
				newPrimitive->setBoundingBox(posMin, posMax);
				newMesh->primitives.push_back(newPrimitive);
			}
			// Mesh BB from BBs of primitives
			for (auto p : newMesh->primitives) {
				if (p->bb.valid && !newMesh->bb.valid) {
					newMesh->bb = p->bb;
					newMesh->bb.valid = true;
				}
				newMesh->bb.min = glm::min(newMesh->bb.min, p->bb.min);
				newMesh->bb.max = glm::max(newMesh->bb.max, p->bb.max);
			}
			newNode->mesh = newMesh;
		}
		if (parent) {
			parent->children.push_back(newNode);
		} else {
			nodes.push_back(newNode);
		}
		linearNodes.push_back(newNode);
}

void Model::getNodeProps(const tinygltf::Node &node, const tinygltf::Model &model, size_t &vertexCount, size_t &indexCount)
{
	if (node.children.size() > 0){
		for (size_t i = 0; i < node.children.size(); i++){
			getNodeProps(model.nodes[node.children[i]], model, vertexCount, indexCount);
		}
	}
	if (node.mesh > -1) {
			const tinygltf::Mesh mesh = model.meshes[node.mesh];
			for (size_t i = 0; i < mesh.primitives.size(); i++) {
				auto primitive = mesh.primitives[i];
				vertexCount += model.accessors[primitive.attributes.find("POSITION")->second].count;
				if (primitive.indices > -1) {
					indexCount += model.accessors[primitive.indices].count;
				}
			}
		}
}

void Model::loadSkins(tinygltf::Model &gltfModel)
{
	for (tinygltf::Skin &source : gltfModel.skins) {
			Skin *newSkin = new Skin{};
			newSkin->name = source.name;
				
			// Find skeleton root node
			if (source.skeleton > -1) {
				newSkin->skeletonRoot = nodeFromIndex(source.skeleton);
			}

			// Find joint nodes
			for (int jointIndex : source.joints) {
				Node* node = nodeFromIndex(jointIndex);
				if (node) {
					newSkin->joints.push_back(nodeFromIndex(jointIndex));
				}
			}

			// Get inverse bind matrices from buffer
			if (source.inverseBindMatrices > -1) {
				const tinygltf::Accessor &accessor = gltfModel.accessors[source.inverseBindMatrices];
				const tinygltf::BufferView &bufferView = gltfModel.bufferViews[accessor.bufferView];
				const tinygltf::Buffer &buffer = gltfModel.buffers[bufferView.buffer];
				newSkin->inverseBindMatrices.resize(accessor.count);
				memcpy(newSkin->inverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::mat4));
			}

			skins.push_back(newSkin);
		}
}

void Model::loadTextures(tinygltf::Model &gltfModel)
{
	for (tinygltf::Texture &tex : gltfModel.textures) {
		tinygltf::Image image = gltfModel.images[tex.source];
		TextureSampler textureSampler;
		if (tex.sampler == -1){
			//No sampler specified, use a default one
			textureSampler.magFilter = vk::Filter::eLinear;
			textureSampler.minFilter = vk::Filter::eLinear;
			textureSampler.addressModeU = vk::SamplerAddressMode::eRepeat;
			textureSampler.addressModeV = vk::SamplerAddressMode::eRepeat;
			textureSampler.addressModeW = vk::SamplerAddressMode::eRepeat;
		}else{
			textureSampler = textureSamplers[tex.sampler];
		}
		GLTFTexture texture;
		texture.fromglTfImage(image, textureSampler);
		textures.push_back(texture);
	}

}

vk::SamplerAddressMode Model::getVkWrapMode(int32_t wrapMode)
{
	switch (wrapMode)
	{
	case -1:
	case 10497:
		return vk::SamplerAddressMode::eRepeat;
	case 33071:
		return vk::SamplerAddressMode::eClampToEdge;
	case 33648:
		return vk::SamplerAddressMode::eMirroredRepeat;
	}
	SG_CORE_WARN("Invalid wrap mode: {0}", wrapMode);
    return vk::SamplerAddressMode::eRepeat;
}

vk::Filter Model::getVkFilterMode(int32_t filterMode)
{
	switch (filterMode)
	{
	case -1:
	case 9728:
		return vk::Filter::eNearest;
	case 9729:
		return vk::Filter::eLinear;
	case 9984:
		return vk::Filter::eNearest;
	case 9985:
		return vk::Filter::eNearest;
	case 9986:
		return vk::Filter::eLinear;
	case 9987:
		return vk::Filter::eLinear;
	}
	SG_CORE_WARN("Invalid filter mode for :{}",filterMode);

    return vk::Filter::eLinear;
}

void Model::loadTextureSamplers(tinygltf::Model &gltfModel)
{
	for (auto smpl : gltfModel.samplers) {
		TextureSampler sampler;
		sampler.minFilter = getVkFilterMode(smpl.minFilter);
		sampler.magFilter = getVkFilterMode(smpl.magFilter);
		sampler.addressModeU = getVkWrapMode(smpl.wrapS);
		sampler.addressModeV = getVkWrapMode(smpl.wrapT);
		sampler.addressModeW = sampler.addressModeV;
		textureSamplers.push_back(sampler);
	}
}

void Model::loadMaterials(tinygltf::Model &gltfModel)
{
	for (tinygltf::Material &mat : gltfModel.materials) {
		PBRMaterial material{};
		material.doubleSided = mat.doubleSided;
		//BaseColor
		if (mat.values.find("baseColorTexture") != mat.values.end()) {
			material.baseColorTexture = &textures[mat.values["baseColorTexture"].TextureIndex()];
			material.texCoordSets.baseColor = mat.values["baseColorTexture"].TextureTexCoord();
		}

		//MetallicRoughnessTexture
		if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
			material.metallicRoughnessTexture = &textures[mat.values["metallicRoughnessTexture"].TextureIndex()];
			material.texCoordSets.metallicRoughness = mat.values["metallicRoughnessTexture"].TextureTexCoord();
		}

		if (mat.values.find("roughnessFactor") != mat.values.end()) {
			material.roughnessFactor = static_cast<float>(mat.values["roughnessFactor"].Factor());
		}
		if (mat.values.find("metallicFactor") != mat.values.end()) {
			material.metallicFactor = static_cast<float>(mat.values["metallicFactor"].Factor());
		}
		if (mat.values.find("baseColorFactor") != mat.values.end()) {
			material.baseColorFactor = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
		}

		//Normal Texture		
		if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) {
			material.normalTexture = &textures[mat.additionalValues["normalTexture"].TextureIndex()];
			material.texCoordSets.normal = mat.additionalValues["normalTexture"].TextureTexCoord();
		}


		if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
			material.emissiveTexture = &textures[mat.additionalValues["emissiveTexture"].TextureIndex()];
			material.texCoordSets.emissive = mat.additionalValues["emissiveTexture"].TextureTexCoord();
		}

		if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
			material.occlusionTexture = &textures[mat.additionalValues["occlusionTexture"].TextureIndex()];
			material.texCoordSets.occlusion = mat.additionalValues["occlusionTexture"].TextureTexCoord();
		}

		if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end()) {
			tinygltf::Parameter param = mat.additionalValues["alphaMode"];
			if (param.string_value == "BLEND") {
				material.alphaMode = PBRMaterial::ALPHAMODE_BLEND;
			}
			if (param.string_value == "MASK") {
				material.alphaCutoff = 0.5f;
				material.alphaMode = PBRMaterial::ALPHAMODE_MASK;
			}
		}
		if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end()) {
			material.alphaCutoff = static_cast<float>(mat.additionalValues["alphaCutoff"].Factor());
		}
		if (mat.additionalValues.find("emissiveFactor") != mat.additionalValues.end()) {
			material.emissiveFactor = glm::vec4(glm::make_vec3(mat.additionalValues["emissiveFactor"].ColorFactor().data()), 1.0);
		}

		// Extensions
		// @TODO: Find out if there is a nicer way of reading these properties with recent tinygltf headers
		if (mat.extensions.find("KHR_materials_pbrSpecularGlossiness") != mat.extensions.end()) {
			auto ext = mat.extensions.find("KHR_materials_pbrSpecularGlossiness");
			if (ext->second.Has("specularGlossinessTexture")) {
				auto index = ext->second.Get("specularGlossinessTexture").Get("index");
				material.extension.specularGlossinessTexture = &textures[index.Get<int>()];
				auto texCoordSet = ext->second.Get("specularGlossinessTexture").Get("texCoord");
				material.texCoordSets.specularGlossiness = texCoordSet.Get<int>();
				material.pbrWorkflows.specularGlossiness = true;
			}
			if (ext->second.Has("diffuseTexture")) {
				auto index = ext->second.Get("diffuseTexture").Get("index");
				material.extension.diffuseTexture = &textures[index.Get<int>()];
			}
			if (ext->second.Has("diffuseFactor")) {
				auto factor = ext->second.Get("diffuseFactor");
				for (uint32_t i = 0; i < factor.ArrayLen(); i++) {
					auto val = factor.Get(i);
					material.extension.diffuseFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
				}
			}
			if (ext->second.Has("specularFactor")) {
				auto factor = ext->second.Get("specularFactor");
				for (uint32_t i = 0; i < factor.ArrayLen(); i++) {
					auto val = factor.Get(i);
					material.extension.specularFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
				}
			}
		}

		if (mat.extensions.find("KHR_materials_unlit") != mat.extensions.end()) {
			material.unlit = true;
		}

		if (mat.extensions.find("KHR_materials_emissive_strength") != mat.extensions.end()) {
			auto ext = mat.extensions.find("KHR_materials_emissive_strength");
			if (ext->second.Has("emissiveStrength")) {
				auto value = ext->second.Get("emissiveStrength");
				material.emissiveStrength = (float)value.Get<double>();
			}
		}

		material.index = static_cast<uint32_t>(materials.size());
		materials.push_back(material);
	}
	// Push a default material at the end of the list for meshes with no material assigned
	materials.push_back(PBRMaterial());
}

void Model::loadAnimations(tinygltf::Model &gltfModel)
{
	for (tinygltf::Animation &anim : gltfModel.animations) {
		Animation animation{};
		animation.name = anim.name;
		if (anim.name.empty()) {
			animation.name = std::to_string(animations.size());
		}

		// Samplers
		for (auto &samp : anim.samplers) {
			AnimationSampler sampler{};

			if (samp.interpolation == "LINEAR") {
				sampler.interpolation = AnimationSampler::InterpolationType::LINEAR;
			}
			if (samp.interpolation == "STEP") {
				sampler.interpolation = AnimationSampler::InterpolationType::STEP;
			}
			if (samp.interpolation == "CUBICSPLINE") {
				sampler.interpolation = AnimationSampler::InterpolationType::CUBICSPLINE;
			}

			// Read sampler input time values
			{
				const tinygltf::Accessor &accessor = gltfModel.accessors[samp.input];
				const tinygltf::BufferView &bufferView = gltfModel.bufferViews[accessor.bufferView];
				const tinygltf::Buffer &buffer = gltfModel.buffers[bufferView.buffer];

				assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

				const void *dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
				const float *buf = static_cast<const float*>(dataPtr);
				for (size_t index = 0; index < accessor.count; index++) {
					sampler.inputs.push_back(buf[index]);
				}

				for (auto input : sampler.inputs) {
					if (input < animation.start) {
						animation.start = input;	//Start Time
					};
					if (input > animation.end) {
						animation.end = input;		//End Time
					}
				}
			}

			// Read sampler output T/R/S values 
			{
				const tinygltf::Accessor &accessor = gltfModel.accessors[samp.output];
				const tinygltf::BufferView &bufferView = gltfModel.bufferViews[accessor.bufferView];
				const tinygltf::Buffer &buffer = gltfModel.buffers[bufferView.buffer];

				assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

				const void *dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];

				switch (accessor.type) {
				case TINYGLTF_TYPE_VEC3: {
					const glm::vec3 *buf = static_cast<const glm::vec3*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++) {
						sampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));
					}
					break;
				}
				case TINYGLTF_TYPE_VEC4: {
					const glm::vec4 *buf = static_cast<const glm::vec4*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++) {
						sampler.outputsVec4.push_back(buf[index]);
					}
					break;
				}
				default: {
					std::cout << "unknown type" << std::endl;
					break;
				}
				}
			}

			animation.samplers.push_back(sampler);
		}

		// Channels
		for (auto &source: anim.channels) {
			AnimationChannel channel{};

			if (source.target_path == "rotation") {
				channel.path = AnimationChannel::PathType::ROTATION;
			}
			if (source.target_path == "translation") {
				channel.path = AnimationChannel::PathType::TRANSLATION;
			}
			if (source.target_path == "scale") {
				channel.path = AnimationChannel::PathType::SCALE;
			}
			if (source.target_path == "weights") {
				std::cout << "weights not yet supported, skipping channel" << std::endl;
				continue;
			}
			channel.samplerIndex = source.sampler;
			channel.node = nodeFromIndex(source.target_node);
			if (!channel.node) {
				continue;
			}

			animation.channels.push_back(channel);
		}

		animations.push_back(animation);
	}
}

void Model::loadFromFile(std::string filename, float scale)
{
	tinygltf::Model gltfModel;
	tinygltf::TinyGLTF gltfContext;

	std::string error;
	std::string warning;

	bool binary = false;
	size_t extpos = filename.rfind('.', filename.length());
	if (extpos != std::string::npos) {
		binary = (filename.substr(extpos + 1, filename.length() - extpos) == "glb");
	}

	bool fileLoaded = binary ? gltfContext.LoadBinaryFromFile(&gltfModel, &error, &warning, filename.c_str()) : gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, filename.c_str());

	LoaderInfo loaderInfo{};
	size_t vertexCount = 0;
	size_t indexCount = 0;

	if (fileLoaded) {
		loadTextureSamplers(gltfModel);
		loadTextures(gltfModel);
		loadMaterials(gltfModel);

		const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];

		// Get vertex and index buffer sizes up-front
		for (size_t i = 0; i < scene.nodes.size(); i++) {
			getNodeProps(gltfModel.nodes[scene.nodes[i]], gltfModel, vertexCount, indexCount);
		}
		loaderInfo.vertexBuffer = new MeshAndSkeletonVertex[vertexCount];
		loaderInfo.indexBuffer = new uint32_t[indexCount];

		// TODO: scene handling with no default scene
		for (size_t i = 0; i < scene.nodes.size(); i++) {
			const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
			loadNode(nullptr, node, scene.nodes[i], gltfModel, loaderInfo, scale);
		}
		if (gltfModel.animations.size() > 0) {
			loadAnimations(gltfModel);
		}
		loadSkins(gltfModel);

		for (auto node : linearNodes) {
			// Assign skins
			if (node->skinIndex > -1) {
				node->skin = skins[node->skinIndex];
			}
			// Initial pose
			if (node->mesh) {
				node->update();
			}
		}
	}
	else {
		// TODO: throw
		std::cerr << "Could not load gltf file: " << error << std::endl;
		return;
	}

	extensions = gltfModel.extensionsUsed;

	size_t vertexBufferSize = vertexCount * sizeof(MeshAndSkeletonVertex);
	size_t indexBufferSize = indexCount * sizeof(uint32_t);

	assert(vertexBufferSize > 0);
	//Create buffers
	//5. Create buffers
    Vulkantool::createVertexBuffer(vertexBufferSize,
    (void*)loaderInfo.vertexBuffer, vertexBuffer_);
   
    Vulkantool::createIndexBuffer(indexBufferSize,
    (void*)loaderInfo.indexBuffer, indexBuffer_);


	delete[] loaderInfo.vertexBuffer;
	delete[] loaderInfo.indexBuffer;

	getSceneDimensions();
}

void Model::drawNode(Node *node, vk::CommandBuffer commandBuffer)
{
	if (node->mesh){
		for (Primitive *primitive : node->mesh->primitives){
			commandBuffer.drawIndexed(primitive->indexCount, 1, primitive->firstIndex, 0, 0);
		}
	}
	for (auto& child : node->children){
		drawNode(child, commandBuffer);
	}
}

void Model::draw(vk::CommandBuffer commandBuffer)
{
	const vk::DeviceSize offsets[1] = { 0 };
	commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer_.buffer, offsets);
	commandBuffer.bindIndexBuffer(indexBuffer_.buffer, 0, vk::IndexType::eUint32);
	for (Node *node : nodes){
		drawNode(node, commandBuffer);
	}
}

void Model::calculateBoundingBox(Node *node, Node *parent)
{
	BoundingBox parentBvh = parent ? parent->bvh : BoundingBox(dimensions.min, dimensions.max);

	if (node->mesh) {
			if (node->mesh->bb.valid) {
				node->aabb = node->mesh->bb.getAABB(node->getMatrix());
				if (node->children.size() == 0) {
					node->bvh.min = node->aabb.min;
					node->bvh.max = node->aabb.max;
					node->bvh.valid = true;
				}
			}
		}

		parentBvh.min = glm::min(parentBvh.min, node->bvh.min);
		parentBvh.max = glm::min(parentBvh.max, node->bvh.max);

		for (auto &child : node->children) {
			calculateBoundingBox(child, node);
		}
}

void Model::getSceneDimensions()
{
	// Calculate binary volume hierarchy for all nodes in the scene
		for (auto node : linearNodes) {
			calculateBoundingBox(node, nullptr);
		}

		dimensions.min = glm::vec3(FLT_MAX);
		dimensions.max = glm::vec3(-FLT_MAX);

		for (auto node : linearNodes) {
			if (node->bvh.valid) {
				dimensions.min = glm::min(dimensions.min, node->bvh.min);
				dimensions.max = glm::max(dimensions.max, node->bvh.max);
			}
		}

		// Calculate scene aabb
		aabb = glm::scale(glm::mat4(1.0f), glm::vec3(dimensions.max[0] - dimensions.min[0], dimensions.max[1] - dimensions.min[1], dimensions.max[2] - dimensions.min[2]));
		aabb[3][0] = dimensions.min[0];
		aabb[3][1] = dimensions.min[1];
		aabb[3][2] = dimensions.min[2];
}

void Model::updateAnimation(uint32_t index, float time)
{
	if (animations.empty()) {
			std::cout << ".glTF does not contain animation." << std::endl;
			return;
		}
		if (index > static_cast<uint32_t>(animations.size()) - 1) {
			std::cout << "No animation with index " << index << std::endl;
			return;
		}
		Animation &animation = animations[index];

		bool updated = false;
		for (auto& channel : animation.channels) {
			AnimationSampler &sampler = animation.samplers[channel.samplerIndex];
			if (sampler.inputs.size() > sampler.outputsVec4.size()) {
				continue;
			}

			for (size_t i = 0; i < sampler.inputs.size() - 1; i++) {
				if ((time >= sampler.inputs[i]) && (time <= sampler.inputs[i + 1])) {
					float u = std::max(0.0f, time - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
					if (u <= 1.0f) {
						switch (channel.path) {
						case AnimationChannel::PathType::TRANSLATION: {
							glm::vec4 trans = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
							channel.node->translation = glm::vec3(trans);
							break;
						}
						case AnimationChannel::PathType::SCALE: {
							glm::vec4 trans = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
							channel.node->scale = glm::vec3(trans);
							break;
						}
						case AnimationChannel::PathType::ROTATION: {
							glm::quat q1;
							q1.x = sampler.outputsVec4[i].x;
							q1.y = sampler.outputsVec4[i].y;
							q1.z = sampler.outputsVec4[i].z;
							q1.w = sampler.outputsVec4[i].w;
							glm::quat q2;
							q2.x = sampler.outputsVec4[i + 1].x;
							q2.y = sampler.outputsVec4[i + 1].y;
							q2.z = sampler.outputsVec4[i + 1].z;
							q2.w = sampler.outputsVec4[i + 1].w;
							channel.node->rotation = glm::normalize(glm::slerp(q1, q2, u));
							break;
						}
						}
						updated = true;
					}
				}
			}
		}
		if (updated) {
			for (auto &node : nodes) {
				node->update();
			}
		}
}

Node *Model::findNode(Node *parent, uint32_t index)
{
    Node* nodeFound = nullptr;
	if (parent->index == index) {
		return parent;
	}

	for (auto& child : parent->children) {
		nodeFound = findNode(child, index);
		if (nodeFound) {
			break;
		}
	}

	return nodeFound;
}

Node *Model::nodeFromIndex(uint32_t index)
{
    Node* nodeFound = nullptr;
		for (auto &node : nodes) {
			nodeFound = findNode(node, index);
			if (nodeFound) {
				break;
			}
		}
		return nodeFound;
	}

    Sego::VmaImageViewSampler &PBRMaterial::GetColorImageViewSampler()
    {
        if (baseColorTexture == nullptr){
			return Sego::VulkanRhi::Instance().defaultTexture->image_view_sampler_;
		}else{
			return baseColorTexture->GetImageViewSampler();
		}
    }

    Sego::VmaImageViewSampler &PBRMaterial::GetMetallicRoughnessImageViewSampler()
    {
        if (metallicRoughnessTexture == nullptr){
			return Sego::VulkanRhi::Instance().defaultTexture->image_view_sampler_;
		}else{
			return metallicRoughnessTexture->GetImageViewSampler();
		}
    }
    Sego::VmaImageViewSampler &PBRMaterial::GetNormalImageViewSampler()
    {
         if (normalTexture == nullptr){
			return Sego::VulkanRhi::Instance().defaultTexture->image_view_sampler_;
		}else{
			return normalTexture->GetImageViewSampler();
		}
    }
    Sego::VmaImageViewSampler &PBRMaterial::GetOcclusionImageViewSampler()
    {
         if (occlusionTexture == nullptr){
			return Sego::VulkanRhi::Instance().defaultTexture->image_view_sampler_;
		}else{
			return occlusionTexture->GetImageViewSampler();
		}
    }
    Sego::VmaImageViewSampler &PBRMaterial::GetEmissiveImageViewSampler()
    {
         if (emissiveTexture == nullptr){
			return Sego::VulkanRhi::Instance().defaultTexture->image_view_sampler_;
		}else{
			return emissiveTexture->GetImageViewSampler();
		}
    }
}
