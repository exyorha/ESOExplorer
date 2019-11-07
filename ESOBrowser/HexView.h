#ifndef HEX_VIEW_H
#define HEX_VIEW_H

#include <QAbstractScrollArea>
#include <QByteArray>
#include <QFile>

class HexView : public QAbstractScrollArea {
public:
	class DataStorage
	{
	public:
		virtual ~DataStorage() {};
		virtual QByteArray getData(size_t position, size_t length) = 0;
		virtual size_t size() = 0;
	};

	HexView(QWidget* parent = 0);
	~HexView();

public slots:
	void setData(std::unique_ptr<DataStorage> &&pData);
	void clear();
	void showFromOffset(size_t offset);

protected:
	void paintEvent(QPaintEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;

private:
	std::unique_ptr<DataStorage> m_pdata;
	size_t           m_posAddr;
	size_t           m_posHex;
	size_t           m_posAscii;
	size_t           m_charWidth;
	size_t           m_charHeight;


	size_t           m_selectBegin;
	size_t           m_selectEnd;
	size_t           m_selectInit;
	size_t           m_cursorPos;


	QSize fullSize() const;
	void resetSelection();
	void resetSelection(int pos);
	void setSelection(int pos);
	void ensureVisible();
	void setCursorPos(int pos);
	size_t cursorPos(const QPoint& position);
};

#endif
