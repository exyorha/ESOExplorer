#ifndef ESO_BROWSER_SELECT_DEPOT_DIALOG_H
#define ESO_BROWSER_SELECT_DEPOT_DIALOG_H

#include <QDialog>
#include <filesystem>

#include "ui_ESOBrowserSelectDepotDialog.h"

class ESOBrowserSelectDepotDialog final : public QDialog {
	Q_OBJECT

public:
	explicit ESOBrowserSelectDepotDialog(QWidget *parent = nullptr);
	~ESOBrowserSelectDepotDialog() override;

	std::filesystem::path path() const;

private slots:
	void on_pathBrowse_clicked();

private:
	Ui::ESOBrowserSelectDepotDialog ui;
};

#endif

