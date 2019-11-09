#ifndef FILAMENT_TYPE_HELPERS_H
#define FILAMENT_TYPE_HELPERS_H

#include <memory>

namespace filament {
	class Engine;
	class SwapChain;
	class Renderer;
	class Scene;
	class View;
	class VertexBuffer;
	class IndexBuffer;
	class MaterialInstance;
	class Camera;
	class Material;
	class Texture;
}

struct FilamentEngineDeleter {
	void operator()(filament::Engine* engine) const;
};

template<typename T>
struct FilamentEngineDerivedDeleter {
	inline FilamentEngineDerivedDeleter(filament::Engine* engine = nullptr) : m_engine(engine) {

	}

	inline void operator()(T* ptr) {
		m_engine->destroy(ptr);
	}

private:
	filament::Engine* m_engine;
};

using FilamentEngine = std::unique_ptr<filament::Engine, FilamentEngineDeleter>;
using FilamentSwapChain = std::unique_ptr<filament::SwapChain, FilamentEngineDerivedDeleter<filament::SwapChain>>;
using FilamentRenderer = std::unique_ptr<filament::Renderer, FilamentEngineDerivedDeleter<filament::Renderer>>;
using FilamentScene = std::unique_ptr<filament::Scene, FilamentEngineDerivedDeleter<filament::Scene>>;
using FilamentView = std::unique_ptr<filament::View, FilamentEngineDerivedDeleter<filament::View>>;
using FilamentVertexBuffer = std::unique_ptr<filament::VertexBuffer, FilamentEngineDerivedDeleter<filament::VertexBuffer>>;
using FilamentIndexBuffer = std::unique_ptr<filament::IndexBuffer, FilamentEngineDerivedDeleter<filament::IndexBuffer>>;
using FilamentMaterialInstance = std::unique_ptr<filament::MaterialInstance, FilamentEngineDerivedDeleter<filament::MaterialInstance>>;
using FilamentCamera = std::unique_ptr<filament::Camera, FilamentEngineDerivedDeleter<filament::Camera>>;
using FilamentMaterial = std::unique_ptr<filament::Material, FilamentEngineDerivedDeleter<filament::Material>>;
using FilamentTexture = std::unique_ptr<filament::Texture, FilamentEngineDerivedDeleter<filament::Texture>>;

#endif
