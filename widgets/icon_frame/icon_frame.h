#ifndef ICON_FRAME_H
#define ICON_FRAME_H

#include <QFrame>
#include <QEvent>
#include <QTimer>
#include <QList>
#include <QLabel>

enum LayoutDirection {
    DIR_HORIZONAL,
    DIR_VERTICAL,
    DIR_GRID
};

enum HoverType {
    HOVER_NONE,
    HOVER_MOVE,
    HOVER_LEAVE,
    HOVER_ENTER
};

namespace Ui {
class iconFrame;
}

class iconFrame : public QFrame
{
    Q_OBJECT

public:
    explicit iconFrame(int width, int height, int numIcons=5, LayoutDirection direction=DIR_VERTICAL, QWidget *parent = 0);
    ~iconFrame();

    void showIcons();
    void hideIcons();

public slots:
    void handleHover(HoverType);
    void timerHit();

protected:
    bool event(QEvent *e);

signals:
    void hoverEvent(HoverType);

private:
    Ui::iconFrame *ui;

    QTimer *m_timer;
    QList<QLabel *> m_iconList;

    int m_hover;
    int m_width;
    int m_height;
    int m_numIcons;
    LayoutDirection m_layoutDirect;

};

#endif // ICON_FRAME_H
