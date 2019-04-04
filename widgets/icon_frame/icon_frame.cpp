#include <QList>
#include "icon_frame.h"
#include "ui_icon_frame.h"
#include <QDebug>

iconFrame::iconFrame(int width, int height, int numIcons, LayoutDirection direction, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::iconFrame)
{
    ui->setupUi(this);

    m_width = width;
    m_height = height;
    m_numIcons = numIcons;
    m_layoutDirect = direction;


    QString sty (QStringLiteral("background-color: rgba(0,0,0,255);"));
    m_iconList = this->findChildren<QLabel *>();

#ifdef JUNK
    for (int i = 0; i < (m_iconList.size()); i++) {
        QLabel *lb = m_iconList.at(i);
        lb->setStyleSheet(sty);
        lb->setFixedWidth(m_width);
        lb->setFixedHeight(40);
    }
#endif
    this->setFixedWidth(m_width);
    this->setFixedHeight(m_height);

    setAttribute(Qt::WA_Hover);

    this->setStyleSheet(QStringLiteral("background-color: rgba(0, 0, 0, 255);"));

    //connect(this, SIGNAL(hoverEvent(HoverType)), this, SLOT(handleHover(HoverType)));

#if 0
    m_timer = new QTimer(this);
    connect(this->m_timer, SIGNAL(timeout()), this, SLOT(timerHit()));
    m_timer->start(1000);

    hideIcons();
#endif
}

iconFrame::~iconFrame()
{
    delete ui;
}

#ifdef JUNK
bool iconFrame::event(QEvent *e)
{
    if (e->type() == QEvent::HoverEnter)
        emit hoverEvent(HOVER_ENTER);
    else if (e->type() == QEvent::HoverMove)
        emit hoverEvent(HOVER_MOVE);
    else if (e->type() == QEvent::HoverLeave)
        emit hoverEvent(HOVER_LEAVE);

    return QFrame::event(e);
}

void iconFrame::handleHover(HoverType t)
{
    m_hover = t;

    switch (t) {
    case HOVER_ENTER:
        qDebug() << "hover enter";
        this->showIcons();
        break;

    case HOVER_MOVE:
        m_hover = HOVER_ENTER;
        break;

    case HOVER_LEAVE:
        qDebug() << "hove leave";
        break;

    default:
        break;
    }
}

void iconFrame::showIcons()
{
    for (int i = 0; i < (m_iconList.size()); i++) {
        QLabel *lb = m_iconList.at(i);
        lb->show();
    }
}

void iconFrame::hideIcons()
{
    for (int i = 0; i < (m_iconList.size()); i++) {
        QLabel *lb = m_iconList.at(i);
        lb->hide();
    }
}

void iconFrame::timerHit()
{
    static int count;

    if (m_hover == HOVER_ENTER)
        return;

    if (m_hover != HOVER_NONE) {
        if (--m_hover == HOVER_NONE) {
            hideIcons();
        }
        return;
    }

    if ((count++ % 10) > 5)
        return;
    else if (count % 2) {
        //show warning icons
        ui->lb_battery->show();
        //ui->lb_bt->show();
    }
    else
        //hide all icons
        hideIcons();
}
#endif
void iconFrame::batterysetPixmap ( QString pix )
{
  ui->lb_battery->setPixmap(QPixmap( pix ));
}
void iconFrame::GPSsetPixmap ( QString pix )
{
  ui->lb_GPS->setPixmap(QPixmap( pix ));
}
void iconFrame::storagesetPixmap ( QString pix )
{
  ui->lb_storage->setPixmap(QPixmap( pix ));
}
