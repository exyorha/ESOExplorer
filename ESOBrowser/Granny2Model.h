#ifndef GRANNY2_MODEL_H
#define GRANNY2_MODEL_H

#include <stdint.h>

#include <vector>
#include <unordered_set>

#include "Granny2TypeHelpers.h"
#include "FilamentTypeHelpers.h"

class FilamentEngineInstance;
class Granny2Renderable;
class DDSTexture;

struct granny_file_info;
struct granny_vertex_data;
struct granny_tri_topology;
struct granny_material;

class Granny2Model final : public std::enable_shared_from_this<Granny2Model> {
public:
	Granny2Model(FilamentEngineInstance* engine, uint64_t key);
	~Granny2Model();

	Granny2Model(const Granny2Model& other) = delete;
	Granny2Model &operator =(const Granny2Model& other) = delete;

	void load();

	std::unique_ptr<Granny2Renderable> createInstance();

	inline FilamentEngineInstance *engine() {
		return m_engine;
	}

private:
	template<typename T>
	static void bufferDescriptorReleaseCallback(void* buffer, size_t size, void* user);

	static void releasePointerToSelf(void* buffer, size_t size, void* user);

	filament::VertexBuffer* findVertexBuffer(granny_file_info* info, granny_vertex_data* vertexData);
	filament::IndexBuffer* findIndexBuffer(granny_file_info* info, granny_tri_topology* triangleTopology);
	filament::MaterialInstance* findMaterial(granny_file_info* info, granny_material* material);

	bool findMap(granny_material* material, const char* map, uint64_t &key);
	
	FilamentEngineInstance* m_engine;
	uint64_t m_key;
	GrannyFile m_file;
	std::unordered_set<std::shared_ptr<DDSTexture>> m_requiredTextures;
	std::vector<FilamentVertexBuffer> m_vertexBuffers;
	std::vector<FilamentIndexBuffer> m_indexBuffers;
	std::vector<FilamentMaterialInstance> m_materials;
};

#endif
