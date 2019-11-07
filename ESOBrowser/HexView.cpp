#include "HexView.h"
#include <QScrollBar>
#include <QPainter>
#include <QSize>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QClipboard>
#include <QApplication>

#include <QDebug>
#include <QFontDatabase>

#include <stdexcept>

static constexpr int HEXCHARS_IN_LINE = 47;
static constexpr int GAP_ADR_HEX = 10;
static constexpr int GAP_HEX_ASCII = 16;
static constexpr int BYTES_PER_LINE = 16;


HexView::HexView(QWidget* parent) :
	QAbstractScrollArea(parent)
{
	setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

	m_charWidth = fontMetrics().horizontalAdvance(QLatin1Char('9'));
	m_charHeight = fontMetrics().height();

	m_posAddr = 0;
	m_posHex = 10 * m_charWidth + GAP_ADR_HEX;
	m_posAscii = m_posHex + HEXCHARS_IN_LINE * m_charWidth + GAP_HEX_ASCII;

	setMinimumWidth(m_posAscii + (BYTES_PER_LINE * m_charWidth) + verticalScrollBar()->sizeHint().width());

	setFocusPolicy(Qt::StrongFocus);
}


HexView::~HexView() = default;

void HexView::setData(std::unique_ptr<HexView::DataStorage> &&pData)
{
	verticalScrollBar()->setValue(0);
	m_pdata = std::move(pData);
	m_cursorPos = 0;
	resetSelection(0);
}


void HexView::showFromOffset(std::size_t offset)
{
	if (m_pdata && offset < m_pdata->size())
	{
		setCursorPos(offset * 2);

		int cursorY = m_cursorPos / (2 * BYTES_PER_LINE);

		verticalScrollBar()->setValue(cursorY);
	}
}

void HexView::clear()
{
	verticalScrollBar()->setValue(0);
}


QSize HexView::fullSize() const
{
	if (!m_pdata)
		return QSize(0, 0);

	std::size_t width = m_posAscii + (BYTES_PER_LINE * m_charWidth);
	std::size_t height = m_pdata->size() / BYTES_PER_LINE;
	if (m_pdata->size() % BYTES_PER_LINE)
		height++;

	height *= m_charHeight;

	return QSize(width, height);
}

void HexView::paintEvent(QPaintEvent* event)
{
	if (!m_pdata)
		return;
	QPainter painter(viewport());

	QSize areaSize = viewport()->size();
	QSize  widgetSize = fullSize();
	verticalScrollBar()->setPageStep(areaSize.height() / m_charHeight);
	verticalScrollBar()->setRange(0, (widgetSize.height() - areaSize.height()) / m_charHeight + 1);

	int firstLineIdx = verticalScrollBar()->value();

	int lastLineIdx = firstLineIdx + areaSize.height() / m_charHeight;
	if (lastLineIdx > m_pdata->size() / BYTES_PER_LINE)
	{
		lastLineIdx = m_pdata->size() / BYTES_PER_LINE;
		if (m_pdata->size() % BYTES_PER_LINE)
			lastLineIdx++;
	}

	painter.fillRect(event->rect(), this->palette().color(QPalette::Base));

	QColor addressAreaColor = QColor(0xd4, 0xd4, 0xd4, 0xff);
	painter.fillRect(QRect(m_posAddr, event->rect().top(), m_posHex - GAP_ADR_HEX + 2, height()), addressAreaColor);

	int linePos = m_posAscii - (GAP_HEX_ASCII / 2);
	painter.setPen(Qt::gray);

	painter.drawLine(linePos, event->rect().top(), linePos, height());

	painter.setPen(Qt::black);

	int yPosStart = m_charHeight;

	QBrush def = painter.brush();
	QBrush selected = QBrush(QColor(0x6d, 0x9e, 0xff, 0xff));
	QByteArray data = m_pdata->getData(firstLineIdx * BYTES_PER_LINE, (lastLineIdx - firstLineIdx) * BYTES_PER_LINE);

	for (int lineIdx = firstLineIdx, yPos = yPosStart; lineIdx < lastLineIdx; lineIdx += 1, yPos += m_charHeight)
	{
		QString address = QString("%1").arg(lineIdx * 16, 10, 16, QChar('0'));
		painter.drawText(m_posAddr, yPos, address);

		for (int xPos = m_posHex, i = 0; i < BYTES_PER_LINE && ((lineIdx - firstLineIdx) * BYTES_PER_LINE + i) < data.size(); i++, xPos += 3 * m_charWidth)
		{
			std::size_t pos = (lineIdx * BYTES_PER_LINE + i) * 2;
			if (pos >= m_selectBegin && pos < m_selectEnd)
			{
				painter.setBackground(selected);
				painter.setBackgroundMode(Qt::OpaqueMode);
			}

			QString val = QString::number((data.at((lineIdx - firstLineIdx) * BYTES_PER_LINE + i) & 0xF0) >> 4, 16);
			painter.drawText(xPos, yPos, val);


			if ((pos + 1) >= m_selectBegin && (pos + 1) < m_selectEnd)
			{
				painter.setBackground(selected);
				painter.setBackgroundMode(Qt::OpaqueMode);
			}
			else
			{
				painter.setBackground(def);
				painter.setBackgroundMode(Qt::OpaqueMode);
			}

			val = QString::number((data.at((lineIdx - firstLineIdx) * BYTES_PER_LINE + i) & 0xF), 16);
			painter.drawText(xPos + m_charWidth, yPos, val);

			painter.setBackground(def);
			painter.setBackgroundMode(Qt::OpaqueMode);

		}

		for (int xPosAscii = m_posAscii, i = 0; ((lineIdx - firstLineIdx) * BYTES_PER_LINE + i) < data.size() && (i < BYTES_PER_LINE); i++, xPosAscii += m_charWidth)
		{
			char ch = data[(lineIdx - firstLineIdx) * BYTES_PER_LINE + i];
			if ((ch < 0x20) || (ch > 0x7e))
				ch = '.';

			painter.drawText(xPosAscii, yPos, QString(ch));
		}

	}

	if (hasFocus())
	{
		int x = (m_cursorPos % (2 * BYTES_PER_LINE));
		int y = m_cursorPos / (2 * BYTES_PER_LINE);
		y -= firstLineIdx;
		int cursorX = (((x / 2) * 3) + (x % 2)) * m_charWidth + m_posHex;
		int cursorY = y * m_charHeight + 4;
		painter.fillRect(cursorX, cursorY, 2, m_charHeight, this->palette().color(QPalette::WindowText));
	}
}


void HexView::keyPressEvent(QKeyEvent* event)
{
	bool setVisible = false;

	/*****************************************************************************/
	/* Cursor movements */
	/*****************************************************************************/
	if (event->matches(QKeySequence::MoveToNextChar))
	{
		setCursorPos(m_cursorPos + 1);
		resetSelection(m_cursorPos);
		setVisible = true;
	}
	if (event->matches(QKeySequence::MoveToPreviousChar))
	{
		setCursorPos(m_cursorPos - 1);
		resetSelection(m_cursorPos);
		setVisible = true;
	}
	if (event->matches(QKeySequence::MoveToEndOfLine))
	{
		setCursorPos(m_cursorPos | ((BYTES_PER_LINE * 2) - 1));
		resetSelection(m_cursorPos);
		setVisible = true;
	}
	if (event->matches(QKeySequence::MoveToStartOfLine))
	{
		setCursorPos(m_cursorPos | (m_cursorPos % (BYTES_PER_LINE * 2)));
		resetSelection(m_cursorPos);
		setVisible = true;
	}
	if (event->matches(QKeySequence::MoveToPreviousLine))
	{
		setCursorPos(m_cursorPos - BYTES_PER_LINE * 2);
		resetSelection(m_cursorPos);
		setVisible = true;
	}
	if (event->matches(QKeySequence::MoveToNextLine))
	{
		setCursorPos(m_cursorPos + BYTES_PER_LINE * 2);
		resetSelection(m_cursorPos);
		setVisible = true;
	}

	if (event->matches(QKeySequence::MoveToNextPage))
	{
		setCursorPos(m_cursorPos + (viewport()->height() / m_charHeight - 1) * 2 * BYTES_PER_LINE);
		resetSelection(m_cursorPos);
		setVisible = true;
	}
	if (event->matches(QKeySequence::MoveToPreviousPage))
	{
		setCursorPos(m_cursorPos - (viewport()->height() / m_charHeight - 1) * 2 * BYTES_PER_LINE);
		resetSelection(m_cursorPos);
		setVisible = true;
	}
	if (event->matches(QKeySequence::MoveToEndOfDocument))
	{
		if (m_pdata)
			setCursorPos(m_pdata->size() * 2);
		resetSelection(m_cursorPos);
		setVisible = true;
	}
	if (event->matches(QKeySequence::MoveToStartOfDocument))
	{
		setCursorPos(0);
		resetSelection(m_cursorPos);
		setVisible = true;
	}

	/*****************************************************************************/
	/* Select commands */
	/*****************************************************************************/
	if (event->matches(QKeySequence::SelectAll))
	{
		resetSelection(0);
		if (m_pdata)
			setSelection(2 * m_pdata->size() + 1);
		setVisible = true;
	}
	if (event->matches(QKeySequence::SelectNextChar))
	{
		int pos = m_cursorPos + 1;
		setCursorPos(pos);
		setSelection(pos);
		setVisible = true;
	}
	if (event->matches(QKeySequence::SelectPreviousChar))
	{
		int pos = m_cursorPos - 1;
		setSelection(pos);
		setCursorPos(pos);
		setVisible = true;
	}
	if (event->matches(QKeySequence::SelectEndOfLine))
	{
		int pos = m_cursorPos - (m_cursorPos % (2 * BYTES_PER_LINE)) + (2 * BYTES_PER_LINE);
		setCursorPos(pos);
		setSelection(pos);
		setVisible = true;
	}
	if (event->matches(QKeySequence::SelectStartOfLine))
	{
		int pos = m_cursorPos - (m_cursorPos % (2 * BYTES_PER_LINE));
		setCursorPos(pos);
		setSelection(pos);
		setVisible = true;
	}
	if (event->matches(QKeySequence::SelectPreviousLine))
	{
		int pos = m_cursorPos - (2 * BYTES_PER_LINE);
		setCursorPos(pos);
		setSelection(pos);
		setVisible = true;
	}
	if (event->matches(QKeySequence::SelectNextLine))
	{
		int pos = m_cursorPos + (2 * BYTES_PER_LINE);
		setCursorPos(pos);
		setSelection(pos);
		setVisible = true;
	}

	if (event->matches(QKeySequence::SelectNextPage))
	{
		int pos = m_cursorPos + (((viewport()->height() / m_charHeight) - 1) * 2 * BYTES_PER_LINE);
		setCursorPos(pos);
		setSelection(pos);
		setVisible = true;
	}
	if (event->matches(QKeySequence::SelectPreviousPage))
	{
		int pos = m_cursorPos - (((viewport()->height() / m_charHeight) - 1) * 2 * BYTES_PER_LINE);
		setCursorPos(pos);
		setSelection(pos);
		setVisible = true;
	}
	if (event->matches(QKeySequence::SelectEndOfDocument))
	{
		int pos = 0;
		if (m_pdata)
			pos = m_pdata->size() * 2;
		setCursorPos(pos);
		setSelection(pos);
		setVisible = true;
	}
	if (event->matches(QKeySequence::SelectStartOfDocument))
	{
		int pos = 0;
		setCursorPos(pos);
		setSelection(pos);
		setVisible = true;
	}

	if (event->matches(QKeySequence::Copy))
	{
		if (m_pdata)
		{
			QString res;
			int idx = 0;
			int copyOffset = 0;

			QByteArray data = m_pdata->getData(m_selectBegin / 2, (m_selectEnd - m_selectBegin) / 2 + 1);
			if (m_selectBegin % 2)
			{
				res += QString::number((data.at((idx + 1) / 2) & 0xF), 16);
				res += " ";
				idx++;
				copyOffset = 1;
			}

			int selectedSize = m_selectEnd - m_selectBegin;
			for (; idx < selectedSize; idx += 2)
			{
				QString val = QString::number((data.at((copyOffset + idx) / 2) & 0xF0) >> 4, 16);
				if (idx + 1 < selectedSize)
				{
					val += QString::number((data.at((copyOffset + idx) / 2) & 0xF), 16);
					val += " ";
				}
				res += val;

				if ((idx / 2) % BYTES_PER_LINE == (BYTES_PER_LINE - 1))
					res += "\n";
			}
			QClipboard* clipboard = QApplication::clipboard();
			clipboard->setText(res);
		}
	}

	if (setVisible)
		ensureVisible();
	viewport()->update();
}

void HexView::mouseMoveEvent(QMouseEvent* event)
{
	int actPos = cursorPos(event->pos());
	setCursorPos(actPos);
	setSelection(actPos);

	viewport()->update();
}

void HexView::mousePressEvent(QMouseEvent* event)
{
	int cPos = cursorPos(event->pos());

	if ((QApplication::keyboardModifiers() & Qt::ShiftModifier) && event->button() == Qt::LeftButton)
		setSelection(cPos);
	else
		resetSelection(cPos);

	setCursorPos(cPos);

	viewport()->update();
}


std::size_t HexView::cursorPos(const QPoint& position)
{
	int pos = -1;

	if ((position.x() >= m_posHex) && (position.x() < (m_posHex + HEXCHARS_IN_LINE * m_charWidth)))
	{
		int x = (position.x() - m_posHex) / m_charWidth;
		if ((x % 3) == 0)
			x = (x / 3) * 2;
		else
			x = ((x / 3) * 2) + 1;

		int firstLineIdx = verticalScrollBar()->value();
		int y = (position.y() / m_charHeight) * 2 * BYTES_PER_LINE;
		pos = x + y + firstLineIdx * BYTES_PER_LINE * 2;
	}
	return pos;
}


void HexView::resetSelection()
{
	m_selectBegin = m_selectInit;
	m_selectEnd = m_selectInit;
}

void HexView::resetSelection(int pos)
{
	if (pos < 0)
		pos = 0;

	m_selectInit = pos;
	m_selectBegin = pos;
	m_selectEnd = pos;
}

void HexView::setSelection(int pos)
{
	if (pos < 0)
		pos = 0;

	if (pos >= m_selectInit)
	{
		m_selectEnd = pos;
		m_selectBegin = m_selectInit;
	}
	else
	{
		m_selectBegin = pos;
		m_selectEnd = m_selectInit;
	}
}


void HexView::setCursorPos(int position)
{
	if (position < 0)
		position = 0;

	int maxPos = 0;
	if (m_pdata)
	{
		maxPos = m_pdata->size() * 2;
		if (m_pdata->size() % BYTES_PER_LINE)
			maxPos++;
	}

	if (position > maxPos)
		position = maxPos;

	m_cursorPos = position;
}

void HexView::ensureVisible()
{
	QSize areaSize = viewport()->size();

	int firstLineIdx = verticalScrollBar()->value();
	int lastLineIdx = firstLineIdx + areaSize.height() / m_charHeight;

	int cursorY = m_cursorPos / (2 * BYTES_PER_LINE);

	if (cursorY < firstLineIdx)
		verticalScrollBar()->setValue(cursorY);
	else if (cursorY >= lastLineIdx)
		verticalScrollBar()->setValue(cursorY - areaSize.height() / m_charHeight + 1);
}
