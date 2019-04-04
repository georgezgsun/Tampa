#include "base_menu.h"
#include <QDebug>
#include "state.h"
#include "debug.h"
#include "utils.h"
#include "ColdFireCommands.h"

baseMenu::baseMenu(QWidget *parent) :
  QWidget(parent)
{
    m_listIndex = 0;
    m_prevListIndex = 0;
    m_baseTimer = NULL;
    m_vkb = NULL;

    m_onOffList << "ON" << "OFF";

    setAttribute(Qt::WA_AcceptTouchEvents);

	m_baseTimer = new QTimer(this);
	connect(m_baseTimer, SIGNAL(timeout()), this, SLOT(timerHit()));
    m_baseTimer->start(1000);  // start time in derived class

}

void baseMenu::exeUpSelect()
{
	Utils& u = Utils::get();
	u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );

    if (this->isEnabled() == false)
        return;

	if ( m_list.size() < 2 ) {
	  return;
	}

repeat:
    if (m_listIndex <= 0)
        m_listIndex = m_list.size() - 1;
    else
        m_listIndex --;

    if (m_listIndex == m_prevListIndex)
        goto repeat;

    QWidget *p = m_list.at(m_listIndex);
    if (p->isHidden() || (p->isEnabled() == false)) {
	  goto repeat;
	}
    p->setFocus();
    p->update();

    //qDebug() << "UP: " << p->objectName() << " selected";

    m_prevListIndex = m_listIndex;
    this->updateSelect();
}

void baseMenu::exeDownSelect()
{
	Utils& u = Utils::get();
	u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );

    if (this->isEnabled() == false)
        return;

	if ( m_list.size() < 2 ) {
	  return;
	}
	
    if (m_listIndex < 0 || ( m_listIndex >= m_list.size() - 1)){
	  // because it will incr the m_listIndex next, need to set it to -1
	  m_listIndex = -1;
	}
repeat:
    m_listIndex = (m_listIndex + 1) % m_list.size();
    if (m_listIndex == m_prevListIndex)
        goto repeat;

    QWidget *p = m_list.at(m_listIndex);
    if(p->isHidden() || (p->isEnabled() == false)){
	  goto repeat;
	}
    p->setFocus();
    p->update();

    m_prevListIndex = m_listIndex;
    this->updateSelect();
}


void baseMenu::setListIndex()
{
    if (m_list.at(m_listIndex)->hasFocus())
        return;

    int i = 0;
    while (i < m_list.size()) {
        QWidget *p = m_list.at(i);
        if (p->hasFocus()) {
            m_listIndex = i;
            break;
        }
        i++;
    }

    if (m_prevListIndex != m_listIndex)
        m_prevListIndex = m_listIndex;
}


int baseMenu::updateState()
{
  state& v = state::get();
  
  this->setEnabled(true);
  this->setInitFocus();
  //emit selectChanged(m_command);
  return v.getState();
}

void baseMenu::setInitFocus()
{
    if (m_list.size() == 0)
        return;

    QWidget *w = m_list.at(m_listIndex);
    Q_ASSERT(w);
    w->setFocus();
}

/*
void baseMenu::setFocus()
{
    setInitFocus();
}
*/
/*
void baseMenu::setUser(int id, QString &name)
{
    setUserID(id);
    setUserName(name);
}
*/

void baseMenu::updateIndexAndSelect()
{
    setListIndex();
    updateSelect();

    //TODO: transmit signal to open new menu
}

void baseMenu::setCmd()
{
    QObject *obj = QObject::sender();
    QWidget *w = dynamic_cast <QWidget *> (obj);
    setCmd(w);
}

void baseMenu::setCmd(vkILine *l)
{
    QWidget *w = (QWidget *)l;
    setCmd(w);

    qDebug() << "setCmd (VKIL), " << l->objectName() << " pressed";
    emit requestVKB(l);
}

void baseMenu::setCmd(QWidget *w)
{
    /*
    if (w == m_list.at(m_listIndex))
        return;

    if (m_hashWidgetToIdx.empty())
        this->buildWIHashTable();

    int idx = m_hashWidgetToIdx.value(w);

    Q_ASSERT(idx >= 0 && idx < m_list.size());

    m_listIndex = idx;
    m_command = m_cmdList.at(idx);

    m_prevListIndex = m_listIndex;
*/
    this->setIndexAndCmd(w);
	//	DEBUG();
    emit selectChanged();
}

void baseMenu::setIndexAndCmd(QWidget *w)
{
    if (w == m_list.at(m_listIndex))
        return;

    if (m_hashWidgetToIdx.empty())
        this->buildWIHashTable();

    int idx = m_hashWidgetToIdx.value(w);

    Q_ASSERT(idx >= 0 && idx < m_list.size());

    m_listIndex = idx;
    m_command = m_cmdList.at(idx);

    m_prevListIndex = m_listIndex;
}

void baseMenu::updateSelect()
{
    int cmd = m_cmdList.at(m_listIndex);

    if (cmd == m_command)
        return;

    if (m_list.at(m_listIndex)->hasFocus() == false)
        m_command = CMD_NONE;
    else
        m_command = cmd;

    //emit selectChanged(m_command, m_listIndex);
}

void baseMenu::connectWidgetSigs()
{
    QWidget *w = NULL;
    foreach (w, m_list) {
        vkILine *le;
        QPushButton *pb;

        le = qobject_cast<vkILine *> (w);
        pb = qobject_cast<QPushButton *> (w);

		//        qDebug() << w->metaObject()->className();
		//		DEBUG() << "Connect " <<  pb;
        if (pb)
            connect(pb, SIGNAL(clicked()), this, SLOT(setCmd()));
        else if (le) { //TODO: customize a signal
            connect(le, SIGNAL(linePressed(vkILine*)), this, SLOT(setCmd(vkILine*)));
        }
    }
}
void baseMenu::disconnectWidgetSigs()
{
    // 12/6/2017, Steven Cao
    QWidget *w = NULL;
    foreach (w, m_list) {
        vkILine *le;
        QPushButton *pb;

        le = qobject_cast<vkILine *> (w);
        pb = qobject_cast<QPushButton *> (w);

		//        qDebug() << w->metaObject()->className();
		//		DEBUG() << "Disconnect " << pb;
        if (pb)
            disconnect(pb, SIGNAL(clicked()), this, SLOT(setCmd()));
        else if (le) { //TODO: customize a signal
            disconnect(le, SIGNAL(linePressed(vkILine*)), this, SLOT(setCmd(vkILine*)));
        }
    }
}

void baseMenu::toggleValue(int cmd, int index, int flag)
{
    QLineEdit *le = NULL;
    QLabel *lb = NULL;
    QPushButton *pb =NULL;
    if (flag == 0)
        le = qobject_cast<QLineEdit *>(m_list.at(index));
    else if (flag == 1)
        lb = qobject_cast<QLabel *>(m_list.at(index));
    else
        pb= qobject_cast<QPushButton *>(m_list.at(index));

    if (!le && !lb && !pb) {
	  QString cmdHex= QString("%1").arg(cmd , 0, 16);
	  DEBUG() << "toggle for NULL object cmd = " <<  cmdHex << "index = " << index;
	  return;
    }

    QStringList *l = m_hashValueList.value(cmd);
    int *i = m_hashValueIndex.value(cmd);
    int j = (*i + 1) % l->size();
    *i = j;

    QString s = l->at(j);
    if (flag == 0)
        le->setText(s);
    else if (flag == 1)
        lb->setText(s);
    else
        pb->setText(s);

    if(cmd == CMD_MODE || cmd == CMD_SHUTTER || cmd ==CMD_GAIN || cmd == CMD_EV )
        Utils::get().sendCmdToCamera(cmd, j);

}

void baseMenu::buildWIHashTable()
{
    if (m_list.empty()) {
        Q_ASSERT_X(0, "buildWIHashTable", "m_list is empty");
    }

    int i;
    QWidget *w = NULL;
    for (i=0; i < m_list.size(); i++) {
        w = m_list.at(i);
        m_hashWidgetToIdx[w] = i;
    }
}
