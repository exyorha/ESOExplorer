#ifndef FILAMENT_VIEWPORT_H
#define FILAMENT_VIEWPORT_H

#include <QWindow>
#include "FilamentTypeHelpers.h"

#include <utils/Entity.h>

class FilamentEngineInstance;

QT_FORWARD_DECLARE_CLASS(QTimer)

class FilamentViewport final : public QWindow {
	Q_OBJECT

public:
	explicit FilamentViewport(QScreen *screen = nullptr);
	~FilamentViewport() override;

	void initialize(FilamentEngineInstance* engine);

	inline filament::Scene* scene() {
		return m_scene.get();
	}

private:
	void exposeEvent(QExposeEvent* event) override;
	bool event(QEvent* event) override;
	
private:
	void drawFrame();

	FilamentEngineInstance* m_engine;
	FilamentSwapChain m_swapchain;
	FilamentRenderer m_renderer;
	FilamentScene m_scene;
	FilamentView m_view;
	FilamentCamera m_camera;
	utils::Entity m_lightEntity;
};

#endif
