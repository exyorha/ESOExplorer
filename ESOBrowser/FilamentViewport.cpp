#include "FilamentViewport.h"
#include "FilamentEngineInstance.h"

#include <filament/Engine.h>
#include <filament/View.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/TransformManager.h>
#include <filament/LightManager.h>

#include <QTimer>

FilamentViewport::FilamentViewport(QScreen *screen) : QWindow(screen), m_engine(nullptr) {

}

FilamentViewport::~FilamentViewport() {
	m_view->setCamera(nullptr);
}

void FilamentViewport::initialize(FilamentEngineInstance* engine) {
	m_engine = engine;

	auto f = engine->engine();

	m_swapchain = FilamentSwapChain(f->createSwapChain(reinterpret_cast<void *>(winId())), f);
	if (!m_swapchain)
		throw std::bad_alloc();

	m_renderer = FilamentRenderer(f->createRenderer(), f);
	if (!m_renderer)
		throw std::bad_alloc();

	m_scene = FilamentScene(f->createScene(), f);
	if (!m_scene)
		throw std::bad_alloc();
	
	m_view = FilamentView(f->createView(), f);
	if (!m_view)
		throw std::bad_alloc();
	
	m_camera = FilamentCamera(f->createCamera(), f);
	if (!m_camera)
		throw std::bad_alloc();

	m_view->setScene(m_scene.get());

	m_lightEntity = utils::EntityManager::get().create();
	filament::LightManager::Builder lightBuilder(filament::LightManager::Type::DIRECTIONAL);
	lightBuilder.intensity(50000.0f);
	lightBuilder.direction(filament::math::float3(0.5, -0.5f, 0.0f));
	lightBuilder.build(*f, m_lightEntity);

	m_scene->addEntity(m_lightEntity);

	m_view->setCamera(m_camera.get());
}

void FilamentViewport::exposeEvent(QExposeEvent* event) {
	if (isExposed()) {
		drawFrame();
	}
}

void FilamentViewport::drawFrame() {
	m_view->setViewport(filament::Viewport(0, 0, width(), height()));

	auto f = m_engine->engine();
	auto& transformManager = f->getTransformManager();

	m_camera->setProjection(
		45.0f,
		width() * 1.0f / height(),
		0.01f,
		100.0f
	);

	m_camera->setExposure(
		16.0f,
		1 / 125.0f,
		100.0f
	);

	transformManager.openLocalTransformTransaction();

	m_camera->lookAt(
		filament::math::float3(-55.0f, 0.0f, 0.0f), filament::math::float3(0.0f, 0.0f, 0.0f)
	);

	transformManager.commitLocalTransformTransaction();

	if (m_renderer->beginFrame(m_swapchain.get())) {
		m_renderer->render(m_view.get());
		m_renderer->endFrame();
	}

	if (isExposed())
		requestUpdate();
}

bool FilamentViewport::event(QEvent* event) {
	if (event->type() == QEvent::Type::UpdateRequest) {
		drawFrame();
	}

	return QWindow::event(event);
}
