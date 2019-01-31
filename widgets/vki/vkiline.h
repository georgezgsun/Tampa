#ifndef VKILINE_H
#define VKILINE_H

#include <QLineEdit>
#include <QMouseEvent>

class vkILine : public QLineEdit
{
    Q_OBJECT
public:
    explicit vkILine(QWidget *parent = 0);

    void setIput(QString &s);

signals:
    void linePressed(vkILine *l);
    void lineFocused(vkILine *l);

public slots:


protected:
    void focusInEvent (QFocusEvent * event);
    void mousePressEvent(QMouseEvent *e);

private:
    bool m_mousePressSent;

};

#endif // VKILINE_H
