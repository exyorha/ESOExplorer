#ifndef FILAMENT_VIEWPORT_H
#define FILAMENT_VIEWPORT_H

#include <QWidget>
#include "FilamentTypeHelpers.h"

#include <utils/Entity.h>

class FilamentEngineInstance;

QT_FORWARD_DECLARE_CLASS(QTimer)

class FilamentViewport final : public QWidget {
	Q_OBJECT

public:
	explicit FilamentViewport(QWidget* parent = nullptr);
	~FilamentViewport() override;

	void initialize(FilamentEngineInstance* engine);

	inline filament::Scene* scene() {
		return m_scene.get();
	}

private:
	void paintEvent(QPaintEvent* event) override;
	void showEvent(QShowEvent* event) override;
	void hideEvent(QHideEvent* event) override;

private slots:
	void drawFrame();

private:
	FilamentEngineInstance* m_engine;
	FilamentSwapChain m_swapchain;
	FilamentRenderer m_renderer;
	FilamentScene m_scene;
	FilamentView m_view;
	FilamentCamera m_camera;
	utils::Entity m_lightEntity;
	QTimer* m_frameTimer;
};

#endif
