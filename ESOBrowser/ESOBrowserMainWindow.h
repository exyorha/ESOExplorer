#ifndef ESO_BROWSER_MAIN_WINDOW_H
#define ESO_BROWSER_MAIN_WINDOW_H

#include <QMainWindow>

#include "ui_ESOBrowserMainWindow.h"
#include "FilamentEngineInstance.h"

class DataStorage;
class DummyTabWidget;
class NameHarvestingEngine;

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QToolButton)

enum class MainWindowCommand {
	None,
	ReloadDirectives,
	ChangeDepot
};

class ESOBrowserMainWindow final : public QMainWindow {
	Q_OBJECT

public:
	explicit ESOBrowserMainWindow(DataStorage *storage, QWidget* parent = nullptr);
	~ESOBrowserMainWindow() override;

	inline MainWindowCommand command() const {
		return m_command;
	}

	inline DataStorage* storage() const {
		return m_storage;
	}

	void addTab(QWidget* widget);

	inline FilamentEngineInstance *filament() {
		return &m_filament;
	}

	inline NameHarvestingEngine* nameHarvesting() {
		return m_nameHarvesting;
	}

private slots:
	void on_actionReloadDirectives_triggered();
	void on_actionChangeDepot_triggered();
	void on_actionQuit_triggered();
	void on_tabs_tabCloseRequested(int index);

	void tabWindowTitleChanged(const QString& title);

private:
	void addDummyTabIfNecessary();
	void saveAllState();
	void restoreAllState();

protected:
	void showEvent(QShowEvent* ev) override;
	void hideEvent(QHideEvent* ev) override;

private:
	template<typename T>
	QWidget* createTab();

	template<typename T>
	void addTab();

	DataStorage* m_storage;
	Ui::ESOBrowserMainWindow ui;
	QLabel* m_depotVersion;
	QMenu* m_addTabMenu;
	QToolButton* m_addTab;
	MainWindowCommand m_command;
	DummyTabWidget* m_dummyTab;
	bool m_populated;
	FilamentEngineInstance m_filament;
	NameHarvestingEngine* m_nameHarvesting;

	static const std::unordered_map<std::string, QWidget *(ESOBrowserMainWindow::*)()> m_tabConstructors;
};

#endif
