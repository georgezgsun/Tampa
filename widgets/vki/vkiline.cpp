#include "vkiline.h"
#include <QDebug>

vkILine::vkILine(QWidget *parent) :
    QLineEdit(parent)
{
    m_mousePressSent = false;
}

void vkILine::mousePressEvent(QMouseEvent *e)
{
    if (this->rect().contains(e->pos())) {
        if (e->button() == Qt::LeftButton) {
            e->accept();
            emit linePressed(this);
        }
        else
            e->ignore();
    }
    else
        e->ignore();

    return;
}

void vkILine::focusInEvent(QFocusEvent *e)
{
    e->accept();
    emit lineFocused(this);
    return;
}

void vkILine::setIput(QString &s)
{
    this->setText(s);
    m_mousePressSent = false;
	//    this->clearFocus();
}

