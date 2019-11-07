#ifndef PERSISTENT_TAB_WIDGET_H
#define PERSISTENT_TAB_WIDGET_H

#include <QtPlugin>

class PersistentTabWidget {
protected:
	PersistentTabWidget();
	~PersistentTabWidget();

public:
	PersistentTabWidget(const PersistentTabWidget& other) = delete;
	PersistentTabWidget &operator =(const PersistentTabWidget& other) = delete;

	virtual void saveToStream(QDataStream& stream) const = 0;
	virtual void restoreFromStream(QDataStream& stream) = 0;

};

Q_DECLARE_INTERFACE(PersistentTabWidget, "org.unwind-project.ESOBrowser.PersistentTabWidget/1.0");

#endif
