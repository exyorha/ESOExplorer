#include <QLabel>
#include <QToolButton>
#include <QSettings>
#include <QCloseEvent>

#include "ESOBrowserMainWindow.h"
#include "DataStorage.h"
#include "DummyTabWidget.h"
#include "FilesystemBrowserWidget.h"
#include "BinaryFileViewWidget.h"
#include "DDSFileViewWidget.h"
#include "TextFileViewWidget.h"
#include "FilesystemBrowserWidget.h"
#include "Granny2FileViewWidget.h"
#include "NameHarvestingEngine.h"
#include "NameHarvestingWidget.h"
#include "DatabaseBrowserWidget.h"

ESOBrowserMainWindow::ESOBrowserMainWindow(DataStorage* storage, QWidget* parent) :
	QMainWindow(parent),
	m_storage(storage),
	m_command(MainWindowCommand::None), m_dummyTab(nullptr), m_populated(false), m_filament(m_storage->filesystem()) {

	ui.setupUi(this);

	m_depotVersion = new QLabel(this);
	ui.statusbar->addWidget(m_depotVersion);

	m_depotVersion->setText(
		tr("Depot %1, built at %2, for client %3")
		.arg(QString::fromStdString(storage->depotBuild()))
		.arg(QString::fromStdString(storage->depotBuildDate()))
		.arg(QString::fromStdString(storage->depotClientVersion()))
	);

	m_addTabMenu = new QMenu(this);
	m_addTabMenu->addAction(tr("Filesystem Browser"), this, &ESOBrowserMainWindow::addFilesystemBrowser, QKeySequence(QStringLiteral("Ctrl+T")));
	m_addTabMenu->addAction(tr("Database Browser"), this, &ESOBrowserMainWindow::addDatabaseBrowser, QKeySequence(QStringLiteral("Ctrl+Shift+T")));
	m_addTabMenu->addAction(tr("Name Harvesting"), this, &ESOBrowserMainWindow::addNameHarvesting);

	m_addTab = new QToolButton(this);
	m_addTab->setText(tr("Add Tab"));
	m_addTab->setMenu(m_addTabMenu);
	m_addTab->setPopupMode(QToolButton::InstantPopup);
	ui.tabs->setCornerWidget(m_addTab);

	m_nameHarvesting = new NameHarvestingEngine(m_storage, this);

	addDummyTabIfNecessary();
}

ESOBrowserMainWindow::~ESOBrowserMainWindow() {
	while (!m_dummyTab) {
		on_tabs_tabCloseRequested(0);
	}
}

void ESOBrowserMainWindow::on_actionReloadDirectives_triggered() {
	m_command = MainWindowCommand::ReloadDirectives;
	close();
}

void ESOBrowserMainWindow::on_actionChangeDepot_triggered() {
	m_command = MainWindowCommand::ChangeDepot;
	close();
}

void ESOBrowserMainWindow::on_actionQuit_triggered() {
	QCoreApplication::quit();
}

void ESOBrowserMainWindow::addFilesystemBrowser() {
	addTab(new FilesystemBrowserWidget(this, this));
}

void ESOBrowserMainWindow::addNameHarvesting() {
	addTab(new NameHarvestingWidget(this, this));
}

void ESOBrowserMainWindow::addDatabaseBrowser() {
	addTab(new DatabaseBrowserWidget(this, this));
}

void ESOBrowserMainWindow::tabWindowTitleChanged(const QString& title) {
	auto widget = static_cast<QWidget*>(sender());
	auto index = ui.tabs->indexOf(widget);
	if (index >= 0) {
		ui.tabs->setTabText(index, widget->windowTitle());
	}
}

void ESOBrowserMainWindow::addTab(QWidget* widget) {
	auto index = ui.tabs->addTab(widget, widget->windowTitle());
	ui.tabs->setCurrentIndex(index);
	connect(widget, &QWidget::windowTitleChanged, this, &ESOBrowserMainWindow::tabWindowTitleChanged);

	if (m_dummyTab && widget != m_dummyTab) {
		auto dummyIndex = ui.tabs->indexOf(m_dummyTab);
		Q_ASSERT(dummyIndex >= 0);
		ui.tabs->removeTab(dummyIndex);
		delete m_dummyTab;
		m_dummyTab = nullptr;		
	}

	ui.tabs->setTabsClosable(!m_dummyTab);
}

void ESOBrowserMainWindow::on_tabs_tabCloseRequested(int index) {
	auto widget = ui.tabs->widget(index);

	if (widget == m_dummyTab)
		return;

	ui.tabs->removeTab(index);
	delete widget;

	addDummyTabIfNecessary();
}

void ESOBrowserMainWindow::addDummyTabIfNecessary() {
	if (ui.tabs->count() == 0) {
		m_dummyTab = new DummyTabWidget(this);
		addTab(m_dummyTab);
	}
}

void ESOBrowserMainWindow::showEvent(QShowEvent* ev) {
	QMainWindow::showEvent(ev);

	if (!ev->spontaneous() && !m_populated) {
		restoreAllState();
		m_populated = true;
	}
}

void ESOBrowserMainWindow::hideEvent(QHideEvent* ev) {
	QMainWindow::hideEvent(ev);

	if (!ev->spontaneous()) {
		saveAllState();
	}
}

void ESOBrowserMainWindow::saveAllState() {
	QSettings settings;
	settings.setValue(QStringLiteral("mainWindow/geometry"), saveGeometry());
	settings.setValue(QStringLiteral("mainWindow/state"), saveState());

	QByteArray tabData;
	QDataStream tabs(&tabData, QIODevice::WriteOnly);
	tabs.setVersion(QDataStream::Qt_5_13);

	for (int index = 0, count = ui.tabs->count(); index < count; index++) {
		auto widget = ui.tabs->widget(index);

		auto persistent = qobject_cast<PersistentTabWidget*>(widget);
		if (persistent) {
			tabs << QString::fromUtf8(widget->metaObject()->className());
			persistent->saveToStream(tabs);
		}
	}

	settings.setValue(QStringLiteral("mainWindow/tabs"), tabData);
	settings.setValue(QStringLiteral("mainWindow/currentTab"), ui.tabs->currentIndex());
}

void ESOBrowserMainWindow::restoreAllState() {
	QSettings settings;
	restoreGeometry(settings.value(QStringLiteral("mainWindow/geometry")).toByteArray());
	restoreState(settings.value(QStringLiteral("mainWindow/state")).toByteArray());

	QDataStream tabs(settings.value(QStringLiteral("mainWindow/tabs")).toByteArray());
	tabs.setVersion(QDataStream::Qt_5_13);

	while (!tabs.atEnd()) {
		QString className;
		tabs >> className;

		auto it = m_tabConstructors.find(className.toStdString());
		if (it == m_tabConstructors.end()) {
			break;
		}

		auto instance = (this->*it->second)();

		auto persistent = qobject_cast<PersistentTabWidget*>(instance);
		if (!persistent)
			break;

		persistent->restoreFromStream(tabs);

		addTab(instance);
	}

	bool ok;
	int idx = settings.value(QStringLiteral("mainWindow/currentTab")).toInt(&ok);
	if (ok)
		ui.tabs->setCurrentIndex(idx);

}

template<typename T>
QWidget* ESOBrowserMainWindow::createTab() {
	return new T(this, this);
}

const std::unordered_map<std::string, QWidget* (ESOBrowserMainWindow::*)()> ESOBrowserMainWindow::m_tabConstructors{
	{ "BinaryFileViewWidget", &ESOBrowserMainWindow::createTab<BinaryFileViewWidget> },
	{ "DDSFileViewWidget", &ESOBrowserMainWindow::createTab<DDSFileViewWidget> },
	{ "TextFileViewWidget", &ESOBrowserMainWindow::createTab<TextFileViewWidget> },
	{ "FilesystemBrowserWidget", &ESOBrowserMainWindow::createTab<FilesystemBrowserWidget> },
	{ "Granny2FileViewWidget", &ESOBrowserMainWindow::createTab<Granny2FileViewWidget> },
	{ "NameHarvestingWidget", &ESOBrowserMainWindow::createTab<NameHarvestingWidget> },
	{ "DatabaseBrowserWidget", &ESOBrowserMainWindow::createTab<DatabaseBrowserWidget> }
};
