#ifndef ICONWIFI_H
#define ICONWIFI_H

#include <QWidget>

class iconWifi : public QWidget
{
    Q_OBJECT

public:
    iconWifi(QWidget *parent = 0);
    ~iconWifi();

    QSize sizeHint() const;

public slots:

signals:
    void penColorChanged (QColor c);
    void penWidthChanged (uint w);
    void bruchColorChanged (QColor c);
    void defaultColorChanged (QColor c);

private:
    QColor          m_defaultColor;
    QColor          m_penColor;
    QColor          m_brushColor;
    Qt::PenStyle    m_penStyle;
    Qt::BrushStyle  m_brushStyle;
    uint            m_penWidth;

    int m_totalBars;
    int m_workBars;
    int m_width;
    int m_height;
    int m_barWidth;
    int m_barHeight[6];

};

#endif // ICONWIFI_H
